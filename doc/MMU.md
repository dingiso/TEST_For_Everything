# MMU


1. bare mode

using satp and csr to check if current mode is Machine mode 

```scala
val mode = if (q.useDmode) csr.priv.dmode else csr.priv.imode
val vmEnable = (satp.mode === 8.U && (mode < ModeM))
val portTranslateEnable = (0 until Width).map(i => vmEnable && !req(i).bits.no_translate)
```

2. pmp check 

for timing optimization, pmp check is divided into dynamic and static after `readTLB` 

1. **dynamic:** superpage (or full-connected reg entries) -> check pmp when translation done

2. **static:** 4K pages (or sram entries) -> check pmp with pre-checked results

```scala
(0 until Width).foreach{i =>
  pmp_check(pmp_addr(i), req_out(i).size, req_out(i).cmd, i)
  for (d <- 0 until nRespDups) {
    perm_check(perm(i)(d), req_out(i).cmd, static_pm(i), static_pm_v(i), i, d)
  }
}
```

**perm_check**

```scala
val modeCheck = !(mode === ModeU && !perm.u || mode === ModeS && perm.u && (!io.csr.priv.sum || ifecth))
val ldPermFail = !(modeCheck && (perm.r || io.csr.priv.mxr && perm.x))
val stPermFail = !(modeCheck && perm.w)
val instrPermFail = !(modeCheck && perm.x)
```


## PMP in QEMU

### 数据结构
在原有的基础上维护 `pmp_addr_t` 确定每一个内存区域的 起始地址和结束地址
```c
typedef struct {
    target_ulong addr_reg;
    uint8_t  cfg_reg;
} pmp_entry_t;

typedef struct {
    target_ulong sa; // start address
    target_ulong ea; // end address 
} pmp_addr_t;

typedef struct {
    pmp_entry_t pmp[MAX_RISCV_PMPS];
    pmp_addr_t  addr[MAX_RISCV_PMPS];
    uint32_t num_rules;
} pmp_table_t;
```

权限控制，主要有读写，以及锁的权限 对应到上面的 `uint8_t cfg_reg`

```c
typedef enum {
    PMP_READ  = 1 << 0,
    PMP_WRITE = 1 << 1,
    PMP_EXEC  = 1 << 2,
    PMP_LOCK  = 1 << 7
} pmp_priv_t;
```

对齐控制，主要识别内存区域的地址的对齐操作

```c
typedef enum {
    PMP_AMATCH_OFF,  /* Null (off)                            */
    PMP_AMATCH_TOR,  /* Top of Range                          */
    PMP_AMATCH_NA4,  /* Naturally aligned four-byte region    */
    PMP_AMATCH_NAPOT /* Naturally aligned power-of-two region */
} pmp_am_t;
```

### 算法

根据原有的 addr_reg 更新 pmp_addr_t 

```c
void pmp_update_rule_addr(CPURISCVState *env, uint32_t pmp_index)
{
    uint8_t this_cfg = env->pmp_state.pmp[pmp_index].cfg_reg;
    target_ulong this_addr = env->pmp_state.pmp[pmp_index].addr_reg;
    target_ulong prev_addr = 0u;
    target_ulong sa = 0u;
    target_ulong ea = 0u;

    if (pmp_index >= 1u) {
        prev_addr = env->pmp_state.pmp[pmp_index - 1].addr_reg;
    }

    switch (pmp_get_a_field(this_cfg)) {
    case PMP_AMATCH_OFF:
        sa = 0u;
        ea = -1;
        break;

    case PMP_AMATCH_TOR:
        sa = prev_addr << 2; /* shift up from [xx:0] to [xx+2:2] */
        ea = (this_addr << 2) - 1u;
        if (sa > ea) {
            sa = ea = 0u;
        }
        break;

    case PMP_AMATCH_NA4:
        sa = this_addr << 2; /* shift up from [xx:0] to [xx+2:2] */
        ea = (sa + 4u) - 1u;
        break;

    case PMP_AMATCH_NAPOT:
        pmp_decode_napot(this_addr, &sa, &ea);
        break;

    default:
        sa = 0u;
        ea = 0u;
        break;
    }

    env->pmp_state.addr[pmp_index].sa = sa;
    env->pmp_state.addr[pmp_index].ea = ea;
}
```

权限检查模块

```c
int pmp_hart_has_privs(CPURISCVState *env, target_ulong addr,
                       target_ulong size, pmp_priv_t privs,
                       pmp_priv_t *allowed_privs, target_ulong mode)
{
    int i = 0;
    int ret = -1;
    int pmp_size = 0;
    target_ulong s = 0;
    target_ulong e = 0;

    if (size == 0) {
        if (riscv_cpu_cfg(env)->mmu) {
            /*
             * If size is unknown (0), assume that all bytes
             * from addr to the end of the page will be accessed.
             */
            pmp_size = -(addr | TARGET_PAGE_MASK);
        } else {
            pmp_size = sizeof(target_ulong);
        }
    } else {
        pmp_size = size;
    }

    /*
     * 1.10 draft priv spec states there is an implicit order
     * from low to high
     */
    for (i = 0; i < MAX_RISCV_PMPS; i++) {
        s = pmp_is_in_range(env, i, addr);
        e = pmp_is_in_range(env, i, addr + pmp_size - 1);

        /* partially inside */
        if ((s + e) == 1) {
            qemu_log_mask(LOG_GUEST_ERROR,
                          "pmp violation - access is partially inside\n");
            ret = -1;
            break;
        }

        /* fully inside */
        const uint8_t a_field =
            pmp_get_a_field(env->pmp_state.pmp[i].cfg_reg);

        /*
         * Convert the PMP permissions to match the truth table in the
         * ePMP spec.
         */
        const uint8_t epmp_operation =
            ((env->pmp_state.pmp[i].cfg_reg & PMP_LOCK) >> 4) |
            ((env->pmp_state.pmp[i].cfg_reg & PMP_READ) << 2) |
            (env->pmp_state.pmp[i].cfg_reg & PMP_WRITE) |
            ((env->pmp_state.pmp[i].cfg_reg & PMP_EXEC) >> 2);

        if (((s + e) == 2) && (PMP_AMATCH_OFF != a_field)) {
            /*
             * If the PMP entry is not off and the address is in range,
             * do the priv check
             */
            if (!MSECCFG_MML_ISSET(env)) {
                /*
                 * If mseccfg.MML Bit is not set, do pmp priv check
                 * This will always apply to regular PMP.
                 */
                *allowed_privs = PMP_READ | PMP_WRITE | PMP_EXEC;
                if ((mode != PRV_M) || pmp_is_locked(env, i)) {
                    *allowed_privs &= env->pmp_state.pmp[i].cfg_reg;
                }
            } else {
                /*
                 * If mseccfg.MML Bit set, do the enhanced pmp priv check
                 */
                if (mode == PRV_M) {
                    switch (epmp_operation) {
                    case 0:
                    case 1:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        *allowed_privs = 0;
                        break;
                    case 2:
                    case 3:
                    case 14:
                        *allowed_privs = PMP_READ | PMP_WRITE;
                        break;
                    case 9:
                    case 10:
                        *allowed_privs = PMP_EXEC;
                        break;
                    case 11:
                    case 13:
                        *allowed_privs = PMP_READ | PMP_EXEC;
                        break;
                    case 12:
                    case 15:
                        *allowed_privs = PMP_READ;
                        break;
                    default:
                        g_assert_not_reached();
                    }
                } else {
                    switch (epmp_operation) {
                    case 0:
                    case 8:
                    case 9:
                    case 12:
                    case 13:
                    case 14:
                        *allowed_privs = 0;
                        break;
                    case 1:
                    case 10:
                    case 11:
                        *allowed_privs = PMP_EXEC;
                        break;
                    case 2:
                    case 4:
                    case 15:
                        *allowed_privs = PMP_READ;
                        break;
                    case 3:
                    case 6:
                        *allowed_privs = PMP_READ | PMP_WRITE;
                        break;
                    case 5:
                        *allowed_privs = PMP_READ | PMP_EXEC;
                        break;
                    case 7:
                        *allowed_privs = PMP_READ | PMP_WRITE | PMP_EXEC;
                        break;
                    default:
                        g_assert_not_reached();
                    }
                }
            }

            /*
             * If matching address range was found, the protection bits
             * defined with PMP must be used. We shouldn't fallback on
             * finding default privileges.
             */
            ret = i;
            break;
        }
    }

    /* No rule matched */
    if (ret == -1) {
        if (pmp_hart_has_privs_default(env, addr, size, privs,
                                       allowed_privs, mode)) {
            ret = MAX_RISCV_PMPS;
        }
    }

    return ret;
}
```

修复 map 的一些小问题

https://github.com/sparcians/map/pull/434

