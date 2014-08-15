#include "quakedef.h"
#include "gl_model.h"
#include "gl_local.h"
#include "new_menu.h"
#include "teamplay.h"
#include "splines.h"
#include "linked_list.h"

struct event_list
{
	struct linked_list *events;
	int events_count;
	int selected_event;
};

struct event_list_entry
{
	struct linked_list_node node;
	struct linked_list *values;
	int values_count;
	double starttime, stoptime;
	int type;
	char *name;
};

struct event_list_entry_value
{
	struct linked_list node;
	double time;
	double value;
	char *text;
	int done;
};


void CamSys_Event_Free_Event(void *data)
{
	struct event_list_entry *entry;
	
	if (!data)
		return;

	entry = data;

	free(entry->name);
	List_Cleanup(entry->values);
	free(data);
}

void CamSys_Event_Free_Event_Value(void *data)
{
	struct event_list_entry_value *entry;
	
	if (!data)
		return;

	entry = data;

	free(entry->text);
	free(data);
}

struct event_list *CamSys_Event_List_Alloc(void)
{
	struct event_list *cs_event;

	cs_event = calloc(1, sizeof(struct event_list));
	if (!cs_event)
		return NULL;

	cs_event->events = List_Add(0, NULL, &CamSys_Event_Free_Event);

	return cs_event;

}

int entry_value_compare(void *data1, void *data2)
{
	struct event_list_entry_value *d1, *d2;

	if (!data1 || !data2)
		return -1;

	d1 = data1;
	d2 = data2;


	if (d1->time > d2->time)
		return 1;
	
	if (d1->time < d2->time)
		return 2;

	if (d1->time == d2->time)
		return 3;

	return 0;
}

struct event_list_entry *CamSys_Event_Add_Entry(struct event_list *event_list, char *name)
{
	struct event_list_entry *entry;

	if (!event_list || !name)
		return NULL;

	if (!event_list->events)
		return NULL;

	entry = calloc(1, sizeof(struct event_list_entry));
	if (!entry)
		return NULL;

	entry->values = List_Add(1, &entry_value_compare, &CamSys_Event_Free_Event_Value);
	entry->starttime = 2;
	entry->stoptime = -1;

	entry->name = strdup(name);

	if (!strcmp(name, "rotation"))
	{
		entry->type = 1;
	}

	if (!strcmp(name, "fov"))
	{
		entry->type = 2;
	}

	if (!strcmp(name, "demospeed"))
	{
		entry->type = 3;
	}

	List_Add_Node(event_list->events, entry);
	event_list->events_count++;

	return entry;
}

struct event_list_entry_value *CamSys_Event_Add_Entry_Value(struct event_list_entry *entry_list, char *text, double value, double time)
{
	struct event_list_entry_value *entry_value;

	if (!entry_list)
		return NULL;

	entry_value = calloc(1, sizeof(struct event_list_entry_value));
	
	if (!entry_value)
		return NULL;

	if (text)
		entry_value->text = strdup(text);

	entry_value->time = time;
	if (entry_list->starttime > time)
		entry_list->starttime = time;
	if (entry_list->stoptime < time)
		entry_list->stoptime = time;
	entry_value->value = value;
	List_Add_Node(entry_list->values, entry_value);
	entry_list->values_count++;
	return entry_value;
}

void CamSys_Event_Handle_Entry(struct event_list_entry *entry, double time)
{
	int i,j;
	double real_time;
	double value;
	struct event_list_entry_value *event_list_entry_value, *event_list_entry_value_start, *event_list_entry_value_stop;


	if (!entry)
		return;

	if (time < 0 || time > 1)
		return;

	i = entry->values_count;

	if (i == 0)
		return;

	event_list_entry_value = event_list_entry_value_start = event_list_entry_value_stop = NULL;

	for (j = 0; j<i ;j++)
	{
		event_list_entry_value = List_Get_Node(entry->values, j);
		if (time < event_list_entry_value->time)
		{
			event_list_entry_value_stop = event_list_entry_value;
			break;
		}
		if (time >= event_list_entry_value->time)
			event_list_entry_value_start = event_list_entry_value;
	}

	if (event_list_entry_value_start && event_list_entry_value_stop)
	{
		real_time = time - event_list_entry_value_start->time;
		real_time = real_time / (event_list_entry_value_stop->time - event_list_entry_value_start->time);
		
		value = real_time * (event_list_entry_value_stop->value - event_list_entry_value_start->value) +event_list_entry_value_start->value;
		if (entry->type == 1)
		{
			r_refdef.viewangles[2] = value;
		}

		if (entry->type == 2)
		{
			Cbuf_AddText(va("fov %f\n", value));
		}

		if (entry->type == 3)
		{
			Cbuf_AddText(va("demospeed %f\n", value));
		}
	}




}

void CamSys_Event_Handle(struct event_list *event_list, double time)
{
	int i,j;
	struct event_list_entry *event_list_entry;

	i = List_Node_Count(event_list->events);

	for (j=0;j<i;j++)
	{
		event_list_entry = List_Get_Node(event_list->events, j);
		if (!event_list_entry)
			continue;

		CamSys_Event_Handle_Entry(event_list_entry, time);

	
	}



}





void CamSys_Event_Print(struct event_list *event_list)
{
	int i,j,k,l;
	struct event_list_entry *entry;
	struct event_list_entry_value *entry_value;

	if (!event_list)
		return;

	i = List_Node_Count(event_list->events);

	Com_Printf("Events - %i:\n", i);


	for (j = 0; j < i; j++)
	{
		entry = List_Get_Node(event_list->events, j);
		if (!entry)
			continue;

		k = List_Node_Count(entry->values);


		Com_Printf("Event %2i - \"%s\" - %2i entries :\n", j, entry->name, k);

		for (l = 0; l < k; l++)
		{
			entry_value  = List_Get_Node(entry->values, l);
			if (!entry_value)
				continue;
			

			if ( entry_value->text)
				Com_Printf(" %2i: time: %f - value: %f - text: %s\n",l, entry_value->time, entry_value->value, entry_value->text);
			else
				Com_Printf(" %2i: time: %f - value: %f\n",l, entry_value->time, entry_value->value);
		}



	}


}


void CamSys_Event_Add_Value_By_Number(struct event_list *event_list, int event_number, double time, double value, char *text)
{
	struct event_list_entry *entry;


	if (!event_list)
		return;

	if (time < 0 || time > 1)
		return;

	entry = List_Get_Node(event_list->events, event_number);


	if (!entry)
		return;

	CamSys_Event_Add_Entry_Value(entry, text, value,time);


	return;
}

void CamSys_Remove_Value_Entry_By_Number(struct event_list_entry *entry, int number)
{

	if (!entry)
		return;

	List_Remove_Node(entry->values, number, 1);
	entry->values_count--;
}


double dummy_double;
double c_event_time_real, c_event_value_real;
double *c_event_time, *c_event_value, *c_selected_event_value;
#define CEM_IDENT "cam_event_menu_ident"
int c_selected_entry, c_selected_entry_value, c_event_type;
struct event_list *CamSys_Get_Active_Event_List(int type);

void CamSys_Event_Update_Pointer(void)
{
	struct event_list *event_list;
	struct event_list_entry *event_list_entry;
	struct event_list_entry_value *entry_value;

	event_list = CamSys_Get_Active_Event_List(0);
	c_event_time = &c_event_time_real;
	c_event_value = &c_event_value_real;

	c_selected_event_value =&dummy_double;

	if (event_list)
	{
		if (c_selected_entry >= event_list->events_count)
			c_selected_entry = 0;
		if (c_selected_entry < 0)
			c_selected_entry = event_list->events_count;
	}
	else
		return;


	event_list_entry = List_Get_Node(event_list->events, c_selected_entry);
	if (event_list_entry)
	{
		if (c_selected_entry_value >= event_list_entry->values_count)
			c_selected_entry_value = 0;
		if (c_selected_entry_value < 0)
			c_selected_entry_value = event_list_entry->values_count;


	}
	else
		return;

	
	entry_value = List_Get_Node(event_list_entry->values, c_selected_entry_value);

	if (entry_value)
	{
		c_selected_event_value = &entry_value->value;
	}
	else
	{
		return;
	}



}



void CEM_Update_Event_Text_ce(struct menu_ct *self)
{
	struct event_list *event_list;
	struct event_list_entry *entry;
	struct event_list_entry_value *entry_value;

	event_list = CamSys_Get_Active_Event_List(0);
	if (!event_list)
	{
		MS_CT_Change_Text(self, va("No Spline!"));
		return;
	}

	entry = List_Get_Node(event_list->events, c_selected_entry);
	if (!entry)
	{
		MS_CT_Change_Text(self, va("No Events"));
		return;
	}

	entry_value = List_Get_Node(entry->values, c_selected_entry_value);

	if (!entry_value)
	{
		if (event_list->events_count == 0)
			MS_CT_Change_Text(self, va("%s - no entries\n", entry->name));
		else
			MS_CT_Change_Text(self, va("%2i/%2i:%s - %i entries\n",c_selected_entry+1, event_list->events_count, entry->name, entry->values_count));

		return;
	}

	MS_CT_Change_Text(self, va("%2i/%2i:%s - %i/%i entries\n",c_selected_entry+1, event_list->events_count, entry->name, c_selected_entry_value +1, entry->values_count));
}


void CEM_Next_Entry_ce(void)
{
	c_selected_entry++;
	CamSys_Event_Update_Pointer();
}

void CEM_Prev_Entry_ce(void)
{
	c_selected_entry--;
	CamSys_Event_Update_Pointer();
}

void CEM_Next_Entry_Value_ce(void)
{
	c_selected_entry_value++;
	CamSys_Event_Update_Pointer();
}

void CEM_Prev_Entry_Value_ce(void)
{
	c_selected_entry_value--;
	CamSys_Event_Update_Pointer();
}


void CEM_Type_Button_ce(void)
{
	c_event_type++;
	if (c_event_type > 2)
		c_event_type = 0;

}

void CEM_Type_Button_Update_ce(struct menu_ct *self)
{
	static int last_time = -1;

	if (last_time == c_event_type)
		return;

	last_time = c_event_type;


	if (c_event_type == 0)
	{
		MS_CT_Change_Text(self, "rotation");
	}
	else if (c_event_type == 1)
	{
		MS_CT_Change_Text(self, "fov");
	}
	else if (c_event_type == 2)
	{
		MS_CT_Change_Text(self, "demospeed");
	}

}


void CEM_Add_Button_ce(void)
{
	struct event_list *event_list;
	event_list = CamSys_Get_Active_Event_List(0);

	if (!event_list)
		return;

	if (c_event_type == 0)
		CamSys_Event_Add_Entry(event_list, "rotation");

	if (c_event_type == 1)
		CamSys_Event_Add_Entry(event_list, "fov");

	if (c_event_type == 2)
		CamSys_Event_Add_Entry(event_list, "demospeed"); 

}

void CEM_Value_Add_Button_ce(void)
{

	struct event_list *event_list;
	struct event_list_entry *entry;

	event_list = CamSys_Get_Active_Event_List(0);
	if (!event_list)
		return;

	entry = List_Get_Node(event_list->events, c_selected_entry);
	if (!entry)
		return;

	CamSys_Event_Add_Entry_Value(entry, NULL, c_event_value_real, c_event_time_real);
}

void CEM_Del_Entry_Value_ce(void)
{
	struct event_list *event_list;
	struct event_list_entry *entry;

	event_list = CamSys_Get_Active_Event_List(0);
	if (!event_list)
		return;

	entry = List_Get_Node(event_list->events, c_selected_entry);
	if (!entry)
		return;

	CamSys_Remove_Value_Entry_By_Number(entry, c_selected_entry_value);

}

void CamSys_Event_Init(void)
{
	extern struct MS_Popup_Menu_Menu MS_Popup_Menu_Root;
	struct MS_Popup_Menu_Menu *popup;
	struct menu_s *auto_save_menu;

	popup = MS_Popup_Add_Entry(&MS_Popup_Menu_Root, NULL, "Cam Event Menu", NULL, NULL, "MS_Activate_Menu cam_event_menu_ident\n");

	if (!popup)
		return;

	auto_save_menu = MS_Add_Menu(0, 0, 200, 100, "Cam Event Menu", CEM_IDENT, NULL);

	if (!auto_save_menu)
		return;

	MS_Add_Content(CEM_IDENT, NULL, "cem_event_text", 1,1,1, "not working!", NULL , NULL, NULL, NULL, NULL, 0, 0, 0, NULL, NULL, &CEM_Update_Event_Text_ce);
	MS_Add_Content(CEM_IDENT, "cem_event_text","cem_entry_next",2,2,1,"Next",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CEM_Next_Entry_ce, NULL);
		MS_Add_Content_Text(CEM_IDENT, "cem_entry_next","cem_entry_div1",0," - ");
		MS_Add_Content(CEM_IDENT, "cem_entry_div1","cem_entry_prev",2,2,0,"Prev",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CEM_Prev_Entry_ce, NULL);
	MS_Add_Content_Text(CEM_IDENT, "cem_entry_next","cem_entry_type_text",1,"type: ");
		MS_Add_Content(CEM_IDENT, "cem_entry_type_text","cem_type_button",2,2,0,"Prev",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CEM_Type_Button_ce, &CEM_Type_Button_Update_ce);
		MS_Add_Content_Text(CEM_IDENT, "cem_type_button","cem_entry_type_button_div",0," - ");
		MS_Add_Content(CEM_IDENT, "cem_entry_type_button_div","cem_add_button",2,2,0,"Add",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CEM_Add_Button_ce, NULL);
	MS_Add_Content_Text(CEM_IDENT, "cem_entry_type_text","cem_entry_type_text_div1",1,"-------- entry values -------");
	MS_Add_Content_Text(CEM_IDENT, "cem_entry_type_text_div1","cem_value_time_text",1,"time: ");
		MS_Add_Content(CEM_IDENT,"cem_value_time_text","cem_value_time_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&c_event_time, NULL);
	MS_Add_Content_Text(CEM_IDENT, "cem_value_time_text","cem_value_value_text",1,"value: ");
		MS_Add_Content(CEM_IDENT,"cem_value_value_text","cem_value_value_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&c_event_value, NULL);

	MS_Add_Content(CEM_IDENT, "cem_value_value_text","cem_value_add_button",2,2,1,"Add",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CEM_Value_Add_Button_ce, NULL);

	MS_Add_Content_Text(CEM_IDENT, "cem_value_add_button","cem_div2",1,"-------- selected entry -------");
	MS_Add_Content_Text(CEM_IDENT, "cem_div2","cem_test2",1,"value: ");
	MS_Add_Content(CEM_IDENT,"cem_test2","cem_selected_value_value_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&c_selected_event_value, NULL);

	MS_Add_Content(CEM_IDENT, "cem_selected_value_value_editbox","cem_entry_value_next",2,2,1,"Next",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CEM_Next_Entry_Value_ce, NULL);
		MS_Add_Content_Text(CEM_IDENT, "cem_entry_value_next","cem_entry_value_div1",0," - ");
		MS_Add_Content(CEM_IDENT, "cem_entry_value_div1","cem_entry_prev",2,2,0,"Prev",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CEM_Prev_Entry_Value_ce, NULL);
		MS_Add_Content_Text(CEM_IDENT, "cem_entry_prev","cem_entry_prev_div1",0," - ");
		MS_Add_Content(CEM_IDENT, "cem_entry_prev_div1","cem_entry_del",2,2,0,"Del",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CEM_Del_Entry_Value_ce, NULL);

	CamSys_Event_Update_Pointer();

}




void CamSys_Write_Events(struct event_list *list, int camnum, int type, FILE *f)
{
	char *vc;
	int j,k,x,y;
	int event_count;
	struct event_list_entry *entry;
	struct event_list_entry_value *entry_value;

	if (!list || !f)
		return;

	if (type == 0)
		vc = "cam";
	else if (type == 1)
		vc = "view";
	else
		return;

	
	k = list->events_count;


	event_count = 0;
	for (j = 0; j < k; j++)
	{
		entry = List_Get_Node(list->events, c_selected_entry);
		if (!entry)
			continue;

		y = entry->values_count;
		fprintf(f, "camsys %s %i event add %s\n", vc, camnum, entry->name);

		for (x = 0; x < y; x++)
		{
			entry_value = List_Get_Node(entry->values, x);

			if (!entry_value)
				continue;

			fprintf(f, "camsys %s %i event add_value %i %f %f\n", vc, camnum, event_count, entry_value->time, entry_value->value);
		}
		event_count++;
	}


}
