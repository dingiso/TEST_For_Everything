#include<stdio.h>
#include<stdint.h>

__attribute__((noinline))
int mac_asm(int a,int b,int c) {
	asm __volatile__ (".word 0x10c58577\n");
	asm __volatile__ ("mv  %0,a0\n"
		: "=r"(a)
		: 
		:);
	// printf("a=0x%x\n",a);
	return a;
}

int main() {
	int a=5,b=0x7FFF7FFF,c=0x7FFF7FFF;
	printf("kadd16:=0x%x\n",mac_asm(a,b,c)); // Output : 0x7fff7fff
    b=-0x7FFF7FFF,c=-0x7FFF7FFF;
	printf("kadd16:=0x%x\n",mac_asm(a,b,c)); // Output : 0x80008000
	b= 0x40004000,c=0x20002000;
	printf("kadd16:=0x%x\n",mac_asm(a,b,c)); // Output : 0x60006000
	b= 0xc000c000,c=0xe000e000; // 0xc000 = -0x4000 , 0xe000 = -0x2000
	printf("kadd16:=0x%x\n",mac_asm(a,b,c)); // Output : 0xa000a000
	return 0;
}
