#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>

// 斐波那契数列最大个数
#define MAX_SIZE 30

// 定义结构体share_data
// 包含最大长度为MAX_SIZE的数组fib_sequence(int)和斐波那契数列的个数(int)
typedef struct 
{
	int fib_sequence[MAX_SIZE];
	int sequence_size;
} share_data;

int main(int argc, char const *argv[])
{
	// 程序运行时需要输入argv的大小
	if (argc != 2) {
		printf("Error:only one arguement is input.\n");
		exit(0);
	}
	// 利用atoi函数将字符串转化为整型数
	else if (atoi(argv[1]) <= 0 || atoi(argv[1]) > 30) {
		printf("Error:arguement should be between 0 and 30.\n");
		exit(0);
	}
	// 共享存储区id
	int segment_id;
	const int size = sizeof(share_data);
	share_data *shared_memory;
	// 创建或打开共享存储区shmget
	segment_id = shmget(IPC_PRIVATE, size, S_IRUSR | S_IWUSR);
	// 连接共享存储区shmat
	shared_memory = (share_data *)shmat(segment_id, NULL, 0);
	shared_memory -> sequence_size = atoi(argv[1]);
	int pid;
	pid = fork();
	if (pid < 0) {
		printf("Error:fail to create a child process.\n");
		exit(0);
	}
	// 子进程执行中，共享内存(数据)
	else if (pid == 0) {
		shared_memory -> fib_sequence[0] = 0;
		shared_memory -> fib_sequence[1] = 1;
		if (atoi(argv[1]) > 2) {
			int i = 2;
			for (; i < shared_memory -> sequence_size; i++) {
				shared_memory -> fib_sequence[i] = shared_memory -> fib_sequence[i - 1]
				 + shared_memory -> fib_sequence[i - 2];
			}
		}
	}
	// 子进程执行完毕，main执行，从共享内存中读取并输出结果
	else {
		wait(0);
		printf("The child process is finished.\nThe result is:\n");
		for (int j = 0; j < atoi(argv[1]); j++) {
			printf("%d\n", shared_memory -> fib_sequence[j]);
		}
		printf("\n");
	}
	// 拆除共享存储区连接shmdt
	shmdt(shared_memory);
	// 共享存储区控制shmctl
	shmctl(segment_id, IPC_RMID, NULL);
	return 0;
}