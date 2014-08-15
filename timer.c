#include "quakedef.h"

struct timer
{
	double starttime;
	double lasttime;
	double interval;
	char *buf_cmd;
	struct timer *next, *prev;
	void (*function)(void);
};


static struct timer *timer_entries;


void Timer_Run(struct timer *timer)
{
//	Com_Printf("%f %f %f\n", cls.realrealtime, timer->lasttime, timer->interval);
	timer->lasttime = cls.realrealtime;
	if (timer->buf_cmd)
		Cbuf_AddText(timer->buf_cmd);
	if (timer->function)
		timer->function();

}

void Timer_Delete(int i)
{
	struct timer *timer;
	int x;

	if (timer_entries == NULL)
	{
		Com_Printf("There are no timers to delete\n");
		return;
	}

	if (i < 1)
	{
		Com_Printf("The delete number can't be <1, %i\n", i);
		return;
	}

	x = 1;
	timer = timer_entries;
	while (timer->next)
	{
		if (x >= i)
			break;
		timer = timer->next;
		x++;
	}

	if (x != i)
	{
		Com_Printf("The delete number was out of range, %i max is %i\n", i, x);
		return;
	}

	
	if (timer->next && timer->prev)
	{
		timer->next->prev = timer->prev;
		timer->prev->next = timer->next;
	}
	else if (timer->next && !timer->prev)
	{
		timer->next->prev = NULL;
		timer_entries = timer->next;
	}
	else if (!timer->next && timer->prev)
	{
		timer->prev->next = NULL;
	}
	else if (!timer->next && !timer->prev)
	{
		timer_entries = NULL;
	}
	
	free(timer->buf_cmd);
	free(timer);

}

void Timer_Delete_Struct(struct timer *timer)
{
	if (!timer)
		return;

	if (timer->next && timer->prev)
	{
		timer->next->prev = timer->prev;
		timer->prev->next = timer->next;
	}
	else if (timer->next && !timer->prev)
	{
		timer->next->prev = NULL;
		timer_entries = timer->next;
	}
	else if (!timer->next && timer->prev)
	{
		timer->prev->next = NULL;
	}
	else if (!timer->next && !timer->prev)
	{
		timer_entries = NULL;
	}
	
	free(timer->buf_cmd);
	free(timer);

}



struct timer *Timer_Add(double interval, char *text, void (*function)(void), int no_instant)
{
	struct timer *timer;

	if (text == NULL && function == NULL)
	{
		Com_Printf("danger! danger! no command! not adding the timer\n");
		return NULL;
	}

	if (interval < 0.01)
		Com_Printf("danger! danger! the call interval is quite low with %f\n", interval);
	
	if (timer_entries == NULL)
	{
		timer_entries = calloc(1, sizeof(struct timer));
		timer = timer_entries;
	}
	else
	{
		timer = timer_entries;
		while (timer->next)
		{
			timer= timer->next;
		}
		timer->next = calloc(1, sizeof(struct timer));
		timer->next->prev = timer;
		timer = timer->next;
	}

	if (text)
		timer->buf_cmd = strdup(text);
	timer->starttime = cls.realrealtime;
	if (no_instant)
		timer->lasttime = cls.realrealtime;
	timer->interval = interval;
	timer->function = function;

	return timer;

}




void Timer_Console(void)
{
	int i;
	struct timer *timer;

	if (Cmd_Argc() < 2)
	{
		Com_Printf("use either: add, remove or list\n");
		return;
	}

	
	if (Cmd_Argc() >= 2)
	{
		if (!strcmp(Cmd_Argv(1), "add"))
		{
			if (Cmd_Argc() != 4)
			{
				Com_Printf("%s add interval \"command\"\n", Cmd_Argv(0));
				return;
			}

			if (Cmd_Argc() == 4)
			{
				Timer_Add(atof(Cmd_Argv(2)), Cmd_Argv(3), NULL, 0);
			}
		}

		if (!strcmp(Cmd_Argv(1), "remove"))
		{
			if (Cmd_Argc() != 3)
			{
				Com_Printf("use %s list to get the number then do \"%s remove number\"\n", Cmd_Argv(0), Cmd_Argv(0));
				return;
			}

			if (Cmd_Argc() == 3)
			{
				Timer_Delete(atoi(Cmd_Argv(2)));
			}
		}

		if (!strcmp(Cmd_Argv(1), "list"))
		{
			if (timer_entries == 0)
				Com_Printf("No timers running!\n");
			else
			{
				timer = timer_entries;
				i = 1;
				while(timer)
				{
					Com_Printf("timer %2i:\n start_time: %f\n interval  : %f\n command   : \"%s\"\n",i++, timer->starttime, timer->interval, timer->buf_cmd);
					timer = timer->next;
				}
			}
		}



	}

}


void Time_Frame(void)
{
	struct timer *ltimer, *timer;

	if (timer_entries == NULL)
		return;

	ltimer = timer_entries;

	while(ltimer)
	{
		timer = ltimer->next;
		if(ltimer->lasttime + ltimer->interval <= cls.realrealtime)
		{
			Timer_Run(ltimer);
		}
		ltimer = timer;
	}

}

void Timer_Init(void)
{
	Cmd_AddCommand("timer", Timer_Console);	
}
