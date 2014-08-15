#include "quakedef.h"
#include "new_menu.h"
#include "linked_list.h"
#include "utils.h"
#include "timer.h"

#define DM_IDENT "demo_menu_ident"

int dm_running;

static struct timer *timer;


struct linked_list *bookmark_list;

double real_playback_speed = 1;
double *playback_speed = &real_playback_speed;

struct menu_s *demo_menu;

int selected_bookmark;
int bookmark_count;

struct demo_bookmark
{
	struct linked_list_node node;
	double	time;
	vec3_t	origin, angles;
};

int bookmark_compare(void *data1, void *data2)
{
	struct demo_bookmark *d1, *d2;

	if (!data1 || !data2)
		return -1;

	d1 = data1;
	d2 = data2;


	if (d1->time > d2->time)
		return 0;
	
	if (d1->time < d2->time)
		return 2;

	if (d1->time == d2->time)
		return 3;

	return 0;
}


void Bookmark_Update(void);
void NMENU_Add_Bookmark(void)
{
	struct demo_bookmark *bm;
	bm = malloc(sizeof(struct demo_bookmark));
	bm->time = cls.demotime - demostarttime ;	
	VectorCopy(r_refdef.vieworg, bm->origin);
	VectorCopy(r_refdef.viewangles, bm->angles);
	List_Add_Node(bookmark_list, bm);
	Bookmark_Update();
}

void NMENU_Play_Button(void)
{
	if (dm_running)
	{
		dm_running = 0;
		Cbuf_AddText("cl_demospeed 0\n");
	}
	else
	{
		dm_running = 1;
		Cbuf_AddText(va("cl_demospeed %f\n", real_playback_speed));
	}
}

void NMENU_Play_Button_Update(struct menu_ct *self)
{
	if (dm_running)
	{
		MS_CT_Change_Text(self, "stop");
	}
	else
	{
		MS_CT_Change_Text(self, "start");
	}
}

double bookmark_time;
vec3_t	bookmark_origin, bookmark_angles;
int bookmark_changed;

void Bookmark_Update(void)
{

	void *data;
	struct demo_bookmark *db;

	bookmark_changed = 1;

	bookmark_count = List_Node_Count(bookmark_list);

	if (selected_bookmark >= bookmark_count)
		selected_bookmark = 0;

	if (selected_bookmark < 0)
		selected_bookmark = bookmark_count-1;

	if (!bookmark_count)
		return;

	data = List_Get_Node(bookmark_list, selected_bookmark);
	if (!data)
	{
		bookmark_time = 0;
		return;
	}

	db = data;

	bookmark_time = db->time;
	VectorCopy(db->origin, bookmark_origin);
	VectorCopy(db->angles, bookmark_angles);

}


void NMENU_Bookmark_Next(void)
{
	selected_bookmark++;
	Bookmark_Update();
}

void NMENU_Bookmark_Prev(void)
{
	selected_bookmark--;
	Bookmark_Update();
}


void NMENU_Bookmark_Text_Update(struct menu_ct *self)
{
	char buf[128];

	bookmark_count = List_Node_Count(bookmark_list);


	if (bookmark_count == 0)
	{
		snprintf(buf,sizeof(buf), "No bookmarks!");
	}
	else
	{
		snprintf(buf,sizeof(buf), "%i/%i bookmarks", selected_bookmark +1, bookmark_count);
	}

	MS_CT_Change_Text(self, buf);


}






void NMENU_Bookmark_Del(void)
{
	if (bookmark_count == 0)
		return;

	List_Remove_Node(bookmark_list, selected_bookmark, 1);
	Bookmark_Update();
}


void Timer_Function(void)
{
	extern qbool clpred_newpos;

	if (!timer)
		return;
	
	VectorCopy(bookmark_origin, cl.simorg);
	VectorCopy(bookmark_angles, cl.simangles);
	clpred_newpos = true;
	VectorCopy (cl.simorg, cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin);
	VectorCopy (cl.simangles, cl.viewangles);

	Timer_Delete_Struct(timer);

	timer = NULL;


}


void NMENU_Bookmark_Jump(void)
{


	if (bookmark_count == 0)
		return;
	
	Cbuf_AddText(va("demo_jump %s\n", SecondsToMinutesString((int) bookmark_time)));
	if (timer == NULL)
		timer = Timer_Add(1.0f, NULL, &Timer_Function, 1);



}

void NMENU_Bookmark_Move_To(void)
{
	extern qbool clpred_newpos;
	if (bookmark_count == 0)
		return;
	VectorCopy(bookmark_origin, cl.simorg);
	VectorCopy(bookmark_angles, cl.simangles);
	clpred_newpos = true;

	VectorCopy (cl.simorg, cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin);

	VectorCopy (cl.simangles, cl.viewangles);



}





void NMENU_Bookmark_Jump_Update(struct menu_ct *self)
{

	if (!bookmark_changed)
		return;

	bookmark_changed = 0;
	MS_CT_Change_Text(self, va("jump to %s\n", SecondsToMinutesString((int)bookmark_time)));
}




void NMENU_Demo_Init(void)
{
	extern struct MS_Popup_Menu_Menu MS_Popup_Menu_Root;
	struct MS_Popup_Menu_Entry *popup;

	
	popup = MS_Popup_Add_Entry(&MS_Popup_Menu_Root, NULL, "Demo Menu", NULL, NULL, "MS_Activate_Menu demo_menu_ident\n");

	if (!popup)
		return;

	demo_menu = MS_Add_Menu(0, 0, 200, 100, "Demo Menu", DM_IDENT, NULL);

	if (!demo_menu)
		return;

	MS_Add_Content(DM_IDENT, NULL,"play_button",2,2,1,"test",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_Play_Button, &NMENU_Play_Button_Update);
	MS_Add_Content_Text(DM_IDENT, "play_button", "playback_speed_text", 1, "Playback speed: ");
	MS_Add_Content_Slider_Width(DM_IDENT,"playback_speed_text","playback_speed_slider",2,0,"d",&playback_speed,0.001, 1, 2);
	MS_Add_Content(DM_IDENT,"playback_speed_slider","playback_speed_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&playback_speed, NULL);

	MS_Add_Content(DM_IDENT, "playback_speed_text","add_bookmark_button",2,2,1,"Add",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_Add_Bookmark, NULL);
 
	MS_Add_Content_Text(DM_IDENT, "add_bookmark_button", "add_bookmark_text", 0, " bookmark");

	MS_Add_Content(DM_IDENT, "add_bookmark_button","bookmark_button_next",2,2,1,"Next",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_Bookmark_Next, NULL);
		MS_Add_Content_Text(DM_IDENT, "bookmark_button_next", "bookmark_button_div", 0, " - ");
		MS_Add_Content(DM_IDENT, "bookmark_button_div","bookmark_button_prev",2,2,0,"Previous",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_Bookmark_Prev, NULL);
	MS_Add_Content(DM_IDENT, "bookmark_button_next", "bookmark_text_count", 1,1,1, "test", NULL , NULL, NULL, NULL, NULL, 0, 0, 0, NULL, NULL, &NMENU_Bookmark_Text_Update);
		MS_Add_Content_Text(DM_IDENT, "bookmark_text_count", "bookmark_text_count_div", 0, " - ");
		MS_Add_Content(DM_IDENT, "bookmark_text_count_div","bookmark_button_del",2,2,0,"Delete",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_Bookmark_Del, NULL);

	MS_Add_Content(DM_IDENT, "bookmark_text_count","bookmark_button_jump",2,2,1,"Delete",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_Bookmark_Jump, &NMENU_Bookmark_Jump_Update);
	MS_Add_Content(DM_IDENT, "bookmark_button_jump","bookmark_button_move_to",2,2,1,"Move to saved location",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_Bookmark_Move_To, NULL);



	bookmark_list = List_Add(1, &bookmark_compare, NULL);

}


