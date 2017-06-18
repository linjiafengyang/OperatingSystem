#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

#define MAX_RAND 1000

int data = 0;
int read_count = 0, write_count = 0;// 记录读者的数量和写者的数量
// writeAccess:对全局变量write_count实现互斥
// readAccess：对全局变量read_count实现互斥
// mutexR：对阻塞read这一过程实现互斥
// mutexW：当有写者在写文件或者等待时，读者阻塞在mutexW上
sem_t writeAccess, readAccess, mutexR, mutexW;

struct command
{
	int pid;// 线程号
	char type;// 线程角色（R：读者；W：写者）
	int startTime;// 操作开始的时间
	int lastTime;// 操作的持续时间
};

// 写操作函数
void write() {
	int rd = rand() % MAX_RAND;
	cout << "Write data " << rd << "." << endl;
	data = rd;
}

// 读操作函数
void read() {
	cout << "Read data " << data << "." << endl;
}

// 写者线程
void *writer(void *param) {
	struct command* c = (struct command*)param;
	while (true) {
		sleep(c->startTime);
		cout << "Writer(the " << c->pid << " pthread) requests to write." << endl;
		sem_wait(&writeAccess);

		write_count++;
		if (write_count == 1) 
			sem_wait(&mutexR);
		sem_post(&writeAccess);

		sem_wait(&mutexW);
		cout << "Writer(the " << c->pid << " pthread) begins to write." << endl;
		write();

		sleep(c->lastTime);
		cout << "Writer(the " << c->pid << " pthread) stops writing." << endl;
		sem_post(&mutexW);

		sem_wait(&writeAccess);
		write_count--;
		if (write_count == 0) 
			sem_post(&mutexR);
		sem_post(&writeAccess);

		pthread_exit(0);
	}
}

// 读者线程
void *reader(void *param) {
	struct command* c = (struct command*)param;
	while (true) {
		sleep(c->startTime);
		cout << "Reader(the " << c->pid << " pthread) requests to read." << endl;
		sem_wait(&mutexR);
		sem_wait(&readAccess);

		read_count++;
		if (read_count == 1) 
			sem_wait(&mutexW);
		sem_post(&readAccess);
		sem_post(&mutexR);

		cout << "Reader(the " << c->pid << " pthread) begins to read." << endl;
		read();

		sleep(c->lastTime);
		cout << "Reader(the " << c->pid << " pthread) stops reading." << endl;
		
		sem_wait(&readAccess);
		read_count--;
		if (read_count == 0) 
			sem_post(&mutexW);
		sem_post(&readAccess);

		pthread_exit(0);
	}
}

int main(int argc, char const *argv[])
{
	int number_person = atoi(argv[1]);
	sem_init(&writeAccess, 0, 1);
	sem_init(&readAccess, 0, 1);
	sem_init(&mutexR, 0, 1);
	sem_init(&mutexW, 0, 1);

	struct command information[number_person];
	pthread_t pid[number_person];
	cout << endl << "The default number of the processes is 5." << endl;
	cout << "Please input the test data:" << endl;
	cout << "An example of test data is:" << endl;
	cout << "1 R 2 3" << endl << "2 W 1 4" << endl;
	cout << "3 W 2 1" << endl << "4 R 2 2" << endl;
	cout << "5 R 5 1" << endl << endl;
	
	for (int i = 0; i < number_person; i++) {
		cin >> information[i].pid >> information[i].type 
			>> information[i].startTime >> information[i].lastTime;
	}

	for (int i = 0; i < number_person; i++) {
		if (information[i].type == 'R') {
			cout << "Create a reader pthread-No." << information[i].pid << " pthread." << endl;
			pthread_create(&pid[i], NULL, reader, &information[i]);
		}

		if (information[i].type == 'W') {
			pthread_create(&pid[i], NULL, writer, &information[i]);
			cout << "Create a writer pthread-No." << information[i].pid << " pthread." << endl;
		}
	}

	for (int i = 0; i < number_person; i++) {
		pthread_join(pid[i], NULL);
	}

	sem_destroy(&writeAccess);
	sem_destroy(&readAccess);
	sem_destroy(&mutexW);
	sem_destroy(&mutexR);
	
	return 0;
}