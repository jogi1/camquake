
struct event_list
{
	void *nil;
};

struct event_list_entry
{
	void *nil;
};

struct event_list *CamSys_Event_List_Alloc(void);
void CamSys_Event_Handle(struct event_list *event_list, double time);
void CamSys_Event_Print(struct event_list *event_list);
struct event_list_entry *CamSys_Event_Add_Entry(struct event_list *event_list, char *name);
void CamSys_Event_Add_Value_By_Number(struct event_list *event_list, int event_number, double time, double value, char *text);
