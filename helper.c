#include<stdio.h>
#include<stdlib.h>
struct xy{
	int a;
	char ds;
};

int main(void){
	struct xy **ll;
	struct xy abc={2,'d'}, sds={4,'t'};
	ll = (struct xy **)malloc(sizeof(unsigned long)*10);
	ll[3] = &abc;
	ll[2] = &sds; 
	printf("Val =%d \n",ll[3]->a);
	return 0;
}
