#include <iostream>
#include <cstring>
#include <cstdlib>
#include <malloc.h>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

#define BUFFER_SIZE 4

int total_producer = 0, total_consumer = 0;// 生产者总数、消费者总数
int buffer[BUFFER_SIZE];// 生产者存放的产品
int nextP = 0, nextC = 0;// 下一个生产者存放的产品、下一个消费者消费的产品
sem_t empty, full, mutex;// 记录有多少空位、记录有多少满位、保护对缓冲区插入与成熟的操作

struct command
{
	int pid;// 线程号
	char type;// 线程角色（P：生产者；C：消费者）
	int startTime;// 操作开始的时间
	int lastTime;// 操作的持续时间
	int num;// 生产者存放产品的编号
};

// 生产者线程
void *producer(void *param) {
	struct command* data = (struct command*)param;

	while (true) {
		sem_wait(&empty);
		sleep(data->startTime);
		sem_wait(&mutex);

		buffer[nextP] = data->num;
		cout << "Producer No." << data->pid 
			<< " produces " << "product No." << data->num << endl;
		nextP = (nextP + 1) % BUFFER_SIZE;

		sleep(data->lastTime);
		sem_post(&mutex);
		sem_post(&full);

		pthread_exit(0);
	}
}


// 消费者线程
void *consumer(void *param) {
	struct command* data = (struct command*)param;

	while (true) {
		sem_wait(&full);
		sleep(data->startTime);
		sem_wait(&mutex);

		cout << "Consumer No." << data->pid 
			<< " consumes " << "product No." << buffer[nextC] << endl;
		buffer[nextC] = 0;
		nextC = (nextC + 1) % BUFFER_SIZE;

		sleep(data->lastTime);
		sem_post(&mutex);
		sem_post(&empty);

		pthread_exit(0);
	}
}

int main(int argc, char const *argv[])
{
	int total = atoi(argv[1]);
	struct command information[total];
	pthread_t Pid[total];

	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);
	sem_init(&mutex, 0, 1);

	for (int i = 0; i < BUFFER_SIZE; i++) {
		buffer[i] = 0;
	}

	ifstream in("data.txt");
	if (!in.is_open()) {
		exit(1);
	}

	char buffer[256];
	int j = 0;
	char ch;
	while (!in.eof()) {
		in.read(&ch, 1);
		if (ch != ' ' && ch != '\n' && ch != '\r') 
			buffer[j++] = ch;
	}
	j = 0;

	for (int i = 0; i < total; i++) {
		information[i].pid = buffer[j++] - '0';
		information[i].type = buffer[j++];
		information[i].startTime = buffer[j++] - '0';
		information[i].lastTime = buffer[j++] - '0';
		if (information[i].type == 'P') 
			information[i].num = buffer[j++] - '0';
	}

	for (int i = 0; i < total; i++) {
		if (information[i].type == 'P') {
			total_producer++;
			cout << "Create Producer No." << information[i].pid << endl;
			pthread_create(&Pid[i], NULL, producer, &information[i]);
		}

		if (information[i].type == 'C') {
			total_consumer++;
			pthread_create(&Pid[i], NULL, consumer, &information[i]);
			cout << "Create Consumer No." << information[i].pid << endl;
		}
	}

	for (int i = 0; i < total; i++) {
		pthread_join(Pid[i], NULL);
	}

	sem_destroy(&full);
	sem_destroy(&empty);
	sem_destroy(&mutex);

	return 0;
}