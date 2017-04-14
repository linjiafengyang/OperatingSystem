#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAXLINE 80
#define BUFFER_SIZE 50
char buffer[BUFFER_SIZE];
char *history[10][10];// 最多存十条指令，第一个下标表示命令的数量，第二个下标表示这些命令的第n个命令
int pos = 0;// 下一条即将被输入的命令的位置
int com_l[11] = {0};

/*
 * setup()用于读入下一行输入的命令，并将它分成没有空格的命令和参数存于数组args[]中，
 * 用NULL作为数组结束的标志
 */
void setup(char inputBuffer[], char *args[], int *background) {
	/*
	 * length:命令的字符数目
	 * i:循环变量
	 * start:命令的第一个字符位置
	 * ct:下一个参数存入args[]的位置
	 */
	int length, i, start, ct;
	ct = 0;
	// 读入命令行字符，存入inputBuffer
	length = read(STDIN_FILENO, inputBuffer, MAXLINE);
	start = -1;
	// 输入ctrl d，结束shell程序
	if (length == 0) {
		exit(0);
	}
	// 出错时用错误码-1结束shell
	else if (length < 0) {
		printf("Error in reading command.\n");
		exit(-1);
	}
	else {
		// 检查inputBuffer中的每一个字符
		for (i = 0; i < length; i++) {
			switch(inputBuffer[i]) {
				// 字符为分割参数的空格或制表符
				case ' ':
				case '\t':
					if (start != -1) {
						args[ct] = &inputBuffer[start];
						ct++;
					}
					inputBuffer[i] = '\0';// 设置C string的结束符
					start = -1;
					break;
				// 命令行结束
				case '\n':
					if (start != -1) {
						args[ct] = &inputBuffer[start];
						ct++;
					}
					inputBuffer[i] = '\0';
					args[ct] = NULL;// 命令及参数结束
					break;
				// 置命令在后台运行
				case '&':
					*background = 1;
					inputBuffer[i] = '\0';
					break;
				// 其他字符
				default:
					if (start == -1) {
						start = i;
					}
			}
		}
	}
	if (ct != 0) {
		args[ct] = NULL;
	}
}
// 按ctrl c输出存放在history中的命令记录
void handle_SIGINT(int signum) {
	write(STDOUT_FILENO, buffer, strlen(buffer));
	printf("The command history is:\n");
	int i = pos;
	int j;
	for (int count = 10; count > 0; count--) {
		for (j = 0; j < com_l[i]; j++) {
			printf("%s ", history[i][j]);
		}
		printf("\n");
		i = (i + 1) % 10;
	}
	printf("\nCOMMAND->");
	fflush(stdout);
	return;
}

int main() {
	char inputBuffer[MAXLINE];// 这个缓存用来存放输入的命令
	int background;// ==1时，表示在后台运行命令，即在命令后加上'&'
	char *args[MAXLINE / 2 + 1];// 命令最多40个参数
	int i, j;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			history[i][j] = (char*)malloc(80 * sizeof(char));
		}
	}

	strcpy(buffer, "\nCaught Control C\n");
	signal(SIGINT, handle_SIGINT);

	while (1) {
		background = 0;
		printf("COMMAND->");
		fflush(stdout);// 输出输出缓存内容用fflush(stdout)
		// 获取下一个输入的命令
		setup(inputBuffer, args, &background);

		// 如果不是r型指令
		if ((args[0] != NULL) && (strcmp(args[0], "r") != 0)) {
			if (args[0] != "\n") {		// 存入二维数组history中
				for (i = 0; args[i] != NULL; i++) {
					strcpy(history[pos][i], args[i]);
				}
				com_l[pos] = i;
				pos = (pos + 1) % 10;
			}
		}
		// 如果是r型指令
		if ((args[0] != NULL) && (strcmp(args[0], "r") == 0)) {
			// 只是r型指令
			// 存入二维数组history中
			if (args[1] == NULL) {
				i = (pos + 9) % 10;
				for (j = 0; j < com_l[i]; j++) {
					strcpy(history[pos][j], history[i][j]);
				}
				com_l[pos] = j;
				pos = (pos + 1) % 10;
			}
			// 是r x指令
			else {
				i = pos;
				for (int count = 10; count > 0; count--) {
					i = (i + 9) % 10;
					if (strncmp(args[1], history[i][0], 1) == 0) {
						for (j = 0; j < com_l[i]; j++) {
							strcpy(history[pos][j], history[i][j]);
						}
						com_l[pos] = j;
						pos = (pos + 1) % 10;
					}
				}
			}
		}
		// 用fork()创建一个子进程
		pid_t pid = fork();
		// 创建失败
		if (pid < 0) {
			printf("Fork failed!\n");
		}
		// 子进程将调用execvp()执行命令，即execvp(args[0], args);
		else if (pid == 0) {
			if (strcmp(args[0], "r") != 0) {
				execvp(args[0], args);
				int k = (pos - 1) % 10;
			}
			else {
				char *newargs[MAXLINE / 2 + 1];
				for (i = 0; i < MAXLINE / 2 + 1; i++) {
					newargs[i] = (char*)malloc((MAXLINE / 2 + 1) * sizeof(char));
				}
				pos = (pos + 9) % 10;
				history[pos][0] = '\0';
				if (args[1] == NULL) {
					i = (pos + 9) % 10;
					for (j = 0; j < com_l[i]; j++) {
						strcpy(newargs[j], history[i][j]);
					}
					newargs[j] = NULL;
					execvp(newargs[0], newargs);
					exit(0);
				}
				else {
					i = pos;
					for (int count2 = 10; count2 > 0; count2--) {
						i = (i + 9) % 10;
						if (strncmp(args[1], history[i][0], 1) == 0) {
							for (j = 0; j < com_l[i]; j++) {
								strcpy(newargs[j], history[i][j]);
							}
							newargs[j] = NULL;
							execvp(newargs[0], newargs);
						}
					}
				}
				exit(0);
			}
			exit(0);
		}
		else {
			// 如果background==0，父进程将等待子进程结束
			// 否则将回到函数setup()中等待新命令输入
			if (background == 0) {
				wait(NULL);
			}
			else {
				setup(inputBuffer, args, &background);
			}
		}
	}
}