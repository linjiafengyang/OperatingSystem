#include <iostream>
#include <pthread.h>

using namespace std;

int n;// the size of fibonacci array

void *fibonacci(void *data) {
	int *a = (int*)data;
	// calculate the fibonacci array
	for (int i = 2; i < n; i++) {
		a[i] = a[i - 1] + a[i - 2];
	}
	pthread_exit(NULL);
}

int main() {
	cout << "Please enter the number n(n>2):" << endl;
	cin >> n;
	while (n <= 2) {
		cout << "The number should be larger than 2." << endl;
		cout << "Please enter the number n(n>2):" << endl;
		cin >> n;
	}
	int a[1000];
	// initial a[0] and a[1]
	a[0] = 0;
	a[1] = 1;
	pthread_t th;
	// create a thread to calculate
	pthread_create(&th, NULL, fibonacci, (void*)a);
	// a thread to be joined upon
	pthread_join(th, NULL);

	cout << "Fibonacci:" << endl;
	// output the result
	for (int i = 0; i < n; i++) {
		cout << a[i] << " ";
	}
	cout << endl;
	return 0;
}