#include <stdio.h>

int n;
int temp;
int factorial;


int fact(int n) {
	if (n == 0) return 1;
	return n * fact(n-1);
}

int fib(int n) {
	if (n == 0 || n == 1) return 1;
	return fib(n-1)+fib(n-2);
}

int main(int argc, char * argv[]) {
	int k;
	printf("Enter a non-negitive integer or -1 to quit: ");
	scanf("%d", &k);
	while (k >= 0) {
		temp = 1;
		factorial = 1;
		printf("%d  %d\n", fact(k), fib(k));
		printf("Enter a non-negitive integer or -1 to quit: ");
		scanf("%d", &k);
	}
}