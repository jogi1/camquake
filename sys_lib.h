struct lib_functions
{
	char *name;
	void **ptr;
};
int LIB_Load(char *name, struct lib_functions *functions, int debug);

