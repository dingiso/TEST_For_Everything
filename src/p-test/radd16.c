#include<stdio.h>
#include<stdint.h>

__attribute__((noinline))
int mac_asm(int a,int b,int c) {
	asm __volatile__ (".word 0x00c58577\n");
	asm __volatile__ ("mv  %0,a0\n"
		: "=r"(a)
		: 
		:);
	return a;
}

int main(){
	int a=5,b=0x7FFF7FFF,c=0x7FFF7FFF;
	printf("radd16:=0x%x\n",mac_asm(a,b,c));
    b=0x80008000,c=0x80008000;
	printf("radd16:=0x%x\n",mac_asm(a,b,c));
    b=0x40004000,c=0x80008000;
    printf("radd16:=0x%x\n",mac_asm(a,b,c));
	return 0;
}
