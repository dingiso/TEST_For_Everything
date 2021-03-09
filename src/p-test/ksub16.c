#include<stdio.h>
#include<stdint.h>

__attribute__((noinline))
int mac_asm(int a,int b,int c) {
	asm __volatile__ (".word 0x12c58577\n");
	asm __volatile__ ("mv  %0,a0\n"
		: "=r"(a)
		: 
		:);
	// printf("a=0x%x\n",a);
	return a;
}

int main(){
	int a=5,b=0x0,c=0x8FFF8FFF;
	printf("kadd16:=0x%x\n",mac_asm(a,b,c));
    b=0x8FFF8FFF,c=0x00010001;
	printf("radd16:=0x%x\n",mac_asm(a,b,c));
    // b=0x40004000,c=0x80008000;
    // printf("radd16:=0x%x\n",mac_asm(a,b,c));
	return 0;
}