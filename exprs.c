#include <stdio.h>
#include <stdbool.h>
#include <math.h>

int a;
int b;
int c;
int d;
int e;
int x;
int y;
int z;

bool b1;
bool b2;
bool b3;
bool b4;

int main(int argc, char * argv[]) {

a = 2;
b = ( int) pow(a,(int) pow(3,a));		//a ^ 3 ^ a;
c = -b + -a;
d = b % 7;
e = (int) pow((a + d),(b-510));          //(a+d)^(b-510);
printf("a b c d e   ");
printf ("%d %d %d %d %d",a, b, c, d, e);
printf("\n");

x = 2*a;
y = d+a-e;
z = d*2;

printf("x y z  ");
printf ("%d %d %d",x, y, z);
printf("\n");

b1 = false;

b2 = (2*a > d+a-e && b > d*2  ) || b1;

printf("%d %d %d",b1, b2, 2*a > d+a-e && b > d*2);
printf("\n");

b1 = false;

b2 = true;

    printf("DeMorgans  ");
printf ("%d %d %d %d %d %d",b1, b2, !(b1 && b2), !b1 || !b2, !(b1 || b2), !b1 && !b2);
printf("\n");
    
a = 10;
b = 20;
    
printf("a < b %d\na <= b %d\na == b %d\na >= b %d\na > b %d\na != b %d\n",a < b, a <= b, a == b, a >= b, a > b, a != b);

}

