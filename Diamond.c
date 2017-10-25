#include <stdio.h>

void single_star(int n){
	int i,j,k;
	for (i = 1; i <=n; ++i)
	{
		for (j = 0; j < n - i; ++j)
		{
			printf(" ");
		}
		for (k = 1; k <=i; ++k)
			printf("*");
		printf("\n");
	}
}

void double_star(int n){
	int i,j,k;
	for (i = 1; i <=n; ++i)
	{
		for (j = 0; j < n - i; ++j)
		{
			printf(" ");
		}
		for (k = 1; k <=i; ++k)
			printf("**");
		printf("\n");
	}
}

void diamond_star(int n){
	int i,j,k;
	for (i = 1; i <=n; ++i)
	{
		for (j = 0; j < n - i; ++j)
		{
			printf(" ");
		}
		for (k = 1; k <=i; ++k)
			printf("**");
		printf("\n");
	}

	for (i = n; i >= 1; i--)
	{
		for (j = 0; j < n - i; ++j)
		{
			printf(" ");
		}
		for (k = 1; k <=i; ++k)
			printf("**");
		printf("\n");
	}
}

int main(int argc, char const *argv[]){
	int i,j,n,k;
	printf("Please enter the n value\n");
	scanf("%d",&n);
	single_star(n);
	printf("-----------------------------\n");
	double_star(n);
	printf("-----------------------------\n");
	diamond_star(n);
	return 0;
}