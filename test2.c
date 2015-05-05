int size;
int i;
int j;
int k;
int temp;
bool swapped;
bool binRes;
int low;
int high;
int mid;

read(size);
i = 0;
int x[10];
while (i < size) {
  read(x[i]);
  i = i + 1;
 }
  k = size-1;
  swapped = true;
  
  while (swapped) {
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

i = 0;
while (i < size) {
  print x[i];
  i = i + 1;
}
println;

read(k);
while (k != 0) {
  low = 0;
  high = size -1;
  while (low < high) {
    mid = (high+low)/2;
    if (x[mid] < k){
      low = mid+1;
    }
    else{
      high = mid;
    }
  }
  print(low == high && x[low] == k);
  read(k);
}
