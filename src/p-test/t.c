#include<stdio.h>

__attribute__((noinline))
int mac_asm(int a,int b,int c) {
	asm __volatile__ (".word 0x40c58577\n");
	asm __volatile__ ("mv  %0,a0\n"
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
