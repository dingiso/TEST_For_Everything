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

