#include <stdio.h>
#include <stdbool.h>

bool binSearch(int x[], int size, int key) {
	int high;
	int low;
	int mid;

	low = 0;
	high = size -1;
	while (low < high) {
		mid = (high+low)/2;
		if (x[mid] < key)
			low = mid+1;
		else
			high = mid;
	}
	return low == high && x[low] == key;
}

void bubbleSort(int x[], int size) {
	int temp;
	int j;
	int k;
	bool swapped;
	k = size-1;
	swapped = true;
	while ( swapped) {
		swapped = false;
		j = 0;
		while (j < k) {
			if (x[j] > x[j+1]) {
				temp = x[j];
				x[j] = x[j+1];
				x[j+1] = temp;
				swapped = true;
			}
			j = j+1;
		}
		k = k-1;
	}
}

int main(int argc, char * argv[]) {
	int y[10];
	int size;
	int i;
	int k;

	printf("Enter an integer between 1 and 10: ");
	scanf("%d", &size);
	i = 0;
	while (i < size) {
		printf("Enter a positive integer: ");
		scanf("%d", &(y[i]));
		i = i + 1;
	}

	bubbleSort(y, size);
	i = 0;
	printf("The array sorted: ");
	while (i < size) {
		printf("%d ", y[i]);
		i = i + 1;
	}
	printf("\n");

	printf("Enter a positive integer or 0 to quit: ");
	scanf("%d", &k);
	while (k != 0) {
		printf("%d\n", binSearch(y, size, k));
		printf("Enter a positive integer or 0 to quit: ");
		scanf("%d", &k);
	}
}
		