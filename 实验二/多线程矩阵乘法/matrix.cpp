#include <iostream>
#include <pthread.h>
#include <stdlib.h>

using namespace std;

int M, K, N;
// the size of matrix
int A[100][100];
int B[100][100];
int C[100][100];
// structure for passing data to threads
struct v
{
	int i, j;
};
// calculate the matrix product in C[row][col]
void *calculate(void *data) {
	struct v *a = (struct v*)data;
	int i = a->i;
	int j = a->j;
	for (int k = 0; k < K; k++) {
		C[i][j] += A[i][k] * B[k][j];
	}
	pthread_exit(NULL);
}

int main() {
	cout << "Please enter three numbers(M/K/N) that are less than 100:" << endl;
	cin >> M >> K >> N;
	cout << "Please enter the first matrix(M*K):" << endl;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < K; j++) {
			cin >> A[i][j];
		}
	}
	cout << "Please enter the second matrix(K*N):" << endl;
	for (int i = 0; i < K; i++) {
		for (int j = 0; j < N; j++) {
			cin >> B[i][j];
		}
	}
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			C[i][j] = 0;
		}
	}
	pthread_t tid[M * N];
	pthread_attr_t attr;
	// get the default attributes
	pthread_attr_init(&attr);
	// we have to create M*N pthreads
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			struct v *a = (struct v*)malloc(sizeof(struct v));
			a->i = i;
			a->j = j;
			pthread_create(&tid[i * N + j], &attr, calculate, (void*)a);
		}
	}
	// join upon each thread
	for (int i = 0; i < M * N; i++) {
		pthread_join(tid[i], NULL);
	}
	// output the result
	cout << "The result(M*N) is:" << endl;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			cout << C[i][j] << " ";
			if (j == N - 1) cout << endl;
		}
	}
	return 0;
}