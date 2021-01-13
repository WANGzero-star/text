#include "head.h"

void handler1(int signo)
{
	printf("SIGINT来了!\n");

	return;
}

void handler2(int signo)
{
	printf("SIGQUIT来了!\n");

	return;
}

void handler3(int signo)
{
	printf("SIGTSTP来了!\n");

	return;
}

void handler(int signo)
{
	if (SIGINT == signo)
	{
		printf("SIGINT来了!\n");
	}
	else if (SIGQUIT == signo)
	{
		printf("SIGQUIT来了!\n");
	}
	else if (SIGTSTP == signo)
	{
		printf("SIGTSTP来了!\n");
	}
	
	return;
}

int main(int argc, const char *argv[])
{
	signal(SIGINT, handler);
	signal(SIGQUIT, handler);
	signal(SIGTSTP, handler);

#if 0
	signal(SIGINT, handler1);
	signal(SIGQUIT, handler2);
	signal(SIGTSTP, handler3);
#endif
	while (1)
	{
		printf("我很闲!\n");
		sleep(1);
	}

	return 0;
}
