#ifdef __WIN32__
#include "pthread.h"
#else
#include <pthread.h>
#endif
#include <stdio.h>

static pthread_t main_thread;
static pthread_t child_thread[256];
static int count = 0;

int Thread_Run(void *f)
{
	if (count == 0)
		main_thread = pthread_self();

	if (count >= 256)
	{
		printf("CORE_THREAD_ERROR", "thread limit reached");
		return 0;
	}

	if (pthread_create(&child_thread[count], NULL, f, NULL) != 0)
	{
		printf("CORE_THREAD_ERROR", "could not create thread");
		return 0;
	}
	printf("thread %i created\n", count);
	count++;
	return 1;

}

void Thread_Cleanup(void)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (pthread_cancel(child_thread[i]))
		{
			printf("error canceling thread %i\n", i);
		}
		else
		{
			printf("thread %i canceled\n", i);
		}
	}


}
