## 测试代码

### 指令的测试

在我们了解了指令的添加流程后，我们要对添加后的指令进行测试以确保指令的正确性。

为了成功编译，你需要事先安装`riscv64-unknown-elf-gcc`来编译测试程序，你可以通过  [riscv-gnu-toolchain](https://github.com/riscv/riscv-gnu-toolchain) 进行编译安装

下面我将介绍一下QEMU的编译过程，鉴于我们只需要QEMU的用户态测试程序，我们执行以下命令在你保存qemu的文件夹下：

```bash
mkdir build
cd build
../configure --target-list=riscv64-linux-user
make
```

`build` 文件夹用于保存你的编译结果，你也可以自由选择想要保存的文件夹。

接着我们创建一个 `p_test` 文件夹保存测试程序并进行测试, 因为原有的工具链并不含有新添加的指令，没办法编译成合适的二进制编码，所以我们使用内联汇编为其提供二进制编码，省去更改工具链的麻烦。

```c
#! ./build/p_test/add16.c

#include<stdio.h>

__attribute__((noinline))
int mac_asm(int a,int b,int c) {
        asm __volatile__ (".word 0x40c58577\n");//使用二进制编码指令，过程见下面
        asm __volatile__ ("mv  %0,a0\n" //强制赋给 a0 即 a ，防止编译器优化
                : "=r"(a)
                :
                :);
        printf("a=%d\n",a);
        return a;
}

int main(){
        int a=5,b=0xFFFEFFFF,c=0xFFFEFFFF;
        printf("add16:=0x%x\n  add:=0x%x\n",mac_asm(a,b,c),b+c);
        return 0;
}

```

`RISC-V` 会将函数的参数放入 `a0-a7` 寄存器中，并将 `a0` 寄存器中的值返回

<img src="https://raw.githubusercontent.com/dingiso/dingiso.github.io/main/img/reg_table.png" />

`a0-a7`  对应 `x10-x17` 和二进制编码 `10-17` ， 因此最后的指令编码如下

```ruby
# Encoding used for "add16 a0, a1, a2"
0x40c58577 [base 16]
==
# Group by related bit chunks:
0100000 01100 01011 000 01010 1110111 [base 2]
^       ^     ^     ^   ^     ^
|       |     |     |   |     |
|       |     |     |   |     opcode (6..2=0x1D 1..0=3)
|       |     |     |   dest (10 : a0)
|       |     |     funct3 (14..12=0)
|       |     src1 (11 : a1)
|       src2 (12 : a2)
funct7 (31..25=0x40)
```

为了防止在编译过程中，编译器会对寄存器进行优化，将返回值存入其他寄存器，我们使用 `mv` 指令强制将变量 `a` 的值赋给 `a0` 寄存器,这样就能成功将值返回。

```bash
riscv64-unknown-elf-gcc  -o x x.c
../qemu-riscv64 xx
```

我们利用以上指令执行，返回正确的结果

```bash
add16:=0xfffcfffe
  add:=0xfffdfffe
```

`add16` 由于每16位进行计算，所以无后**16**位的进位，中间位是`c`，而`add`由于有进位，所以是`d`





### 进度

```
reg       .... ...0 1100    0101 1... 0101 0... ....
add16     0100 0000 ....    .... 1000 .... 0111 0111 - 0x40C58577  -  
radd16    0000 0000 ....    .... 1000 .... 0111 0111 - 0x00C58577  -  3/3 (signed shift)
uradd16   0010 0000 ....    .... 1000 .... 0111 0111 - 0x20C58577  -  3/3 (unsigned shift)
kadd16    0001 0000 ....    .... 1000 .... 0111 0111 - 0x10C58577  -  2/2 (with set OV)
ukadd16   0011 0000 ....    .... 1000 .... 0111 0111 - 0x30C58577  -  4/4 (with set OV)
sub16     0100 0010 ....    .... 1000 .... 0111 0111 - 0x42C58577  
rsub16    0000 0010 ....    .... 1000 .... 0111 0111 - 0x02C58577  -  4/4 (with set OV)
ursub16   0010 0010 ....    .... 1000 .... 0111 0111 - 0x22C58577  -  4/4 (with set OV)
ksub16    0001 0010 ....    .... 1000 .... 0111 0111 - 0x12C58577
uksub16   0011 0010 ....    .... 1000 .... 0111 0111 - 0x32C58577
```



### 编译方法

riscv64-unknown-elf-gcc  -o x x.c
../qemu-riscv64 xx