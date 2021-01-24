#include "head.h"

pid_t pid;

void handler_child(int signo)
{
	if (SIGINT == signo)
	{
		printf("爸，我回来了\n");
		kill(getppid(), SIGUSR1);
	}
	else if (SIGUSR2 == signo)
	{
		printf("好的\n");
	}

	return;
}

void handler_parent(int signo)
{
	if (SIGTSTP == signo)
	{
		printf("儿子，我回来了!\n");
		kill(pid, SIGUSR2);
	}
	else if (SIGUSR1 == signo)
	{
		printf("快去写作业!\n");
	}

	return;
}

int main(int argc, const char *argv[])
{
	pid = fork();
	if (-1 == pid)
	{
		perror("fail to fork");
		return -1;
	}
	if (0 == pid)
	{
		signal(SIGTSTP, SIG_IGN);
		signal(SIGINT, handler_child);
		signal(SIGUSR2, handler_child);
	}
	else if (pid > 0)
	{
		signal(SIGINT, SIG_IGN);
		signal(SIGTSTP, handler_parent);
		signal(SIGUSR1, handler_parent);
	}

	while (1);

	return 0;
}
