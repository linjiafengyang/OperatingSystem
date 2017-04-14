#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

void waiting();
void stop(int signum);

int wait_mark;

int main() {
	int pid1, pid2;
	pid1 = fork();
	if (pid1 > 0) {
		pid2 = fork();
		if (pid2 > 0) {
			wait_mark = 1;
			signal(SIGINT, stop);//设置收到信号ctrl c时执行stop函数
			waiting();
			kill(pid1, SIGINT);//向进程p1发出信号SIGINT
			kill(pid2, SIGINT);//向进程p2发出信号SIGINT
			waitpid(pid1, NULL, 0);
			waitpid(pid2, NULL, 0);
			printf("parent process is killed!\n");
			exit(0);
		}
		else {
			wait_mark = 1;
			signal(SIGINT, stop);
			waiting();
			printf("child process 2 is killed by parent!\n");
			exit(0);
		}
	}
	else {
		wait_mark = 1;
		signal(SIGINT, stop);//设置收到信号SIGINT时执行stop函数
		waiting();
		printf("child process 1 is killed by parent!\n");
		exit(0);
	}
}
void waiting() {
	while (wait_mark != 0);
}
void stop(int signum) {
	wait_mark = 0;
}