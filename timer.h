struct timer
{
	void *nix;
};

struct timer *Timer_Add(double interval, char *text, void (*function)(void), int no_instant);
void Timer_Delete_Struct(struct timer *timer);


