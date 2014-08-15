#include <windows.h>
#include <stdio.h>

#define MAX_THREADS 16

DWORD   dwThreadIdArray[MAX_THREADS];
HANDLE  hThreadArray[MAX_THREADS]; 


static int count = 0;

int Thread_Run(void *f)
{
	if (count >= MAX_THREADS)
		return 0;

	hThreadArray[count] = CreateThread(NULL, 0, f, NULL, 0, &dwThreadIdArray[count]);

	if (hThreadArray[count] == NULL)
	{
		printf("error creating thread\n");
		return 0;
	}

	
	
	count++;
	return 1;
}

void Thread_Cleanup(void)
{
	int i;
	WaitForMultipleObjects(count, hThreadArray, TRUE, INFINITE);
	for(i=0; i<count; i++)
		CloseHandle(hThreadArray[i]);

}
