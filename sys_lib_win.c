#include <dlfcn.h>
#ifndef GCC_TEST
#include "quakedef.h"
#else
#include <stdlib.h>
#define Com_Printf printf
#endif
#include "sys_lib.h"
 

int LIB_Load(char *name, struct lib_functions *functions, int debug)
{
	HMODULE *lib;
	struct lib_functions *cf;
	int i;

	if (functions == NULL)
	{
		if (debug)
		{
			Com_Printf("LIB_Load: functions pointer is NULL.\n");
		}
		return 1;
	}

	lib = LoadLibrary(name);

	if (lib == NULL)
	{
		if (debug)
		{
			Com_Printf("LIB_Load: could not open \"%s\" failed.\n error: \"%i\"\n", GetLastError());
		}
		return 1;
	}

	cf = functions;

	for (i = 0; cf[i].name; i++)
	{
		*cf[i].ptr = GetProcAddress(lib, cf[i].name);
		if (*cf[i].ptr == NULL)
		{
			if (debug)
			{
				Com_Printf("LIB_Load: could not load \"%s\" in \"%s\".\n error: \"%i\"\n", cf[i].name, name, GetLastError());
			}
			dlclose(lib);
			return 1;
		}
	}

	return 0;
}

#ifdef GCC_TEST
double (*my_sqrt)(double x);
float (*my_sqrtf)(float x);

struct lib_functions sqrt_funcs[] =
	{
	{	"sqrt", (void **)&my_sqrt},
	{	"sqrtf", (void **)&my_sqrtf},
	{NULL}
	};

int main (void)
{
	LIB_Load("libm.so", sqrt_funcs, 1);
	printf("%f %f\n", my_sqrt(2.0f), my_sqrtf(2.0f));
}
#endif

