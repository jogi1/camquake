#include "quakedef.h"

struct menu_ct
{
	void *ptr;
};

struct menu_s
{
	void *ptr;
};

struct menu_ct *MS_Add_Content(char *mid, char *pid, char *identifier, int type, int subtype, int align, char *text, char *tooltip, char *name, char *color, mpic_t *picture, cvar_t *variable, int width , float limmin, float limmax, char *ptype, void *vptr, void (*update_function)(struct menu_ct *self));
struct menu_s *MS_Add_Menu(int lx, int ly, int rx, int ry, char *name, char *identifier, void (*function)(struct menu_s *self));
void MS_Activate_Menu_Struct(struct menu_s *menu);
struct menu_ct *MS_Add_Content_Text(char *mid,char *pid,char *id,int align,char *text);
void MS_Add_Content_Slider_Width(char *mid, char *pid, char *id, int subtype, int align, char *ptype, void *vptr,float llim,float mlim, int width);

void MS_CT_Change_Text(struct menu_ct *content, char *text);
void MS_CT_Change_Limits(struct menu_ct *content, int lower, int upper);


void CamSys_Add_Cam(void);
void CamSys_Add_View(void);
char *CamSys_Menu_Cam_Numbers(void);
int CamSys_Get_Spline_Count(int type);
int CamSys_Get_Active_Spline_Show(int type);
int CamSys_Get_Active_Spline_Enable(int type);
void CamSys_Toggle_Active_Spline_Show(int type);
void CamSys_Toggle_Active_Spline(int type, int var);
int CamSys_Get_Active_Spline_Number(int type);
void CamSys_Remove_Active_Spline(int type);
void CamSys_Active_Spline_Change(int type, int dir);
int CamSys_Get_Active_Spline_Point_Count(int type);
int CamSys_Get_Active_Spline_Active_Point(int type);





struct menu_s *cam_menu;

void MSCM_Update_ce_camnumtext(struct menu_ct *content)
{
	static int lastcount = -1;
	static int lastselect = -1;
	char text[128];

	if (lastcount == CamSys_Get_Spline_Count(0) && lastselect == CamSys_Get_Active_Spline_Number(0))
		return;

	lastcount = CamSys_Get_Spline_Count(0);
	lastselect = CamSys_Get_Active_Spline_Number(0);

	if (lastcount == 0)
		MS_CT_Change_Text(content, "no cameras available");
	else
	{
		snprintf(text, 128, "%i/%i", lastselect + 1, lastcount);
		MS_CT_Change_Text(content, text);
	}
}

void MSCM_Update_ve_camnumtext(struct menu_ct *content)
{
	static int lastcount = -1;
	static int lastselect = -1;
	char text[128];

	if (lastcount == CamSys_Get_Spline_Count(1) && lastselect == CamSys_Get_Active_Spline_Number(1))
		return;

	lastcount = CamSys_Get_Spline_Count(1);
	lastselect = CamSys_Get_Active_Spline_Number(1);

	if (lastcount == 0)
		MS_CT_Change_Text(content, "no views available");
	else
	{
		snprintf(text, 128, "%i/%i", lastselect + 1, lastcount);
		MS_CT_Change_Text(content, text);
	}
}

void MSCM_Update_ce_camshowtoggle(struct menu_ct *content)
{
	static int lastshow = -1;
	int currentshow;

	if (lastshow == (currentshow = CamSys_Get_Active_Spline_Show(0)))
		return;
	
	lastshow = currentshow;
	if (currentshow == 0)
	{
		MS_CT_Change_Text(content, "off");
	}
	else if (currentshow == 1)
	{
		MS_CT_Change_Text(content, "on");
	}
	else
	{
		MS_CT_Change_Text(content, "erroniusmaximus");
	}

}

void MSCM_Update_ve_camshowtoggle(struct menu_ct *content)
{
	static int lastshow = -1;
	int currentshow;

	if (lastshow == (currentshow = CamSys_Get_Active_Spline_Show(1)))
		return;
	
	lastshow = currentshow;
	if (currentshow == 0)
	{
		MS_CT_Change_Text(content, "off");
	}
	else if (currentshow == 1)
	{
		MS_CT_Change_Text(content, "on");
	}
	else
	{
		MS_CT_Change_Text(content, "erroniusmaximus");
	}
	
}

void MSCM_Update_ce_camshowtoggle_Function(void)
{
	CamSys_Toggle_Active_Spline(0, 1);
}

void MSCM_Update_ve_camshowtoggle_Function(void)
{
	CamSys_Toggle_Active_Spline(1, 1);
}

void MSCM_Update_ce_camenabletoggle(struct menu_ct *content)
{
	static int lastenable = -1;
	int currentenable;

	if (lastenable == (currentenable = CamSys_Get_Active_Spline_Enable(0)))
		return;
	
	lastenable = currentenable;
	if (currentenable == 0)
	{
		MS_CT_Change_Text(content, "off");
	}
	else if (currentenable == 1)
	{
		MS_CT_Change_Text(content, "on");
	}
	else
	{
		MS_CT_Change_Text(content, "erroniusmaximus");
	}
	
}

void MSCM_Update_ve_camenabletoggle(struct menu_ct *content)
{
	static int lastenable = -1;
	int currentenable;

	if (lastenable == (currentenable = CamSys_Get_Active_Spline_Enable(1)))
		return;
	
	lastenable = currentenable;
	if (currentenable == 0)
	{
		MS_CT_Change_Text(content, "off");
	}
	else if (currentenable == 1)
	{
		MS_CT_Change_Text(content, "on");
	}
	else
	{
		MS_CT_Change_Text(content, "erroniusmaximus");
	}
	
}

void MSCM_Update_ce_camenabletoggle_Function(void)
{
	CamSys_Toggle_Active_Spline(0, 0);
}
void MSCM_Update_ve_camenabletoggle_Function(void)
{
	CamSys_Toggle_Active_Spline(1, 0);
}


void MSCM_SS_Next_ce(void)
{
	CamSys_Active_Spline_Change(0, 1);
}
void MSCM_SS_Next_ve(void)
{
	CamSys_Active_Spline_Change(1, 1);
}

void MSCM_SS_Prev_ce(void)
{
	CamSys_Active_Spline_Change(0, 0);
}
void MSCM_SS_Prev_ve(void)
{
	CamSys_Active_Spline_Change(1, 0);
}

void MSCM_SS_Remove_Active_Spline_ce(void)
{
	CamSys_Remove_Active_Spline(0);
}

void MSCM_SS_Remove_Active_Spline_ve(void)
{
	CamSys_Remove_Active_Spline(1);
}




void MSCM_Update_ce_pointumtext(struct menu_ct *content)
{
	
	static int lastcount = -1;
	static int lastselect = -1;
	char text[128];

	if (lastcount == CamSys_Get_Active_Spline_Point_Count(0) && lastselect == CamSys_Get_Active_Spline_Active_Point(0))
		return;

	lastcount = CamSys_Get_Active_Spline_Point_Count(0);
	lastselect = CamSys_Get_Active_Spline_Active_Point(0);

	if (lastcount == 0)
		MS_CT_Change_Text(content, "no points available");
	else
	{
		snprintf(text, 128, "Selected Point: %i/%i", lastselect + 1, lastcount);
		MS_CT_Change_Text(content, text);
	}
	
}
void MSCM_Update_ve_pointumtext(struct menu_ct *content)
{
	
	static int lastcount = -1;
	static int lastselect = -1;
	char text[128];

	if (lastcount == CamSys_Get_Active_Spline_Point_Count(1) && lastselect == CamSys_Get_Active_Spline_Active_Point(1))
		return;

	lastcount = CamSys_Get_Active_Spline_Point_Count(1);
	lastselect = CamSys_Get_Active_Spline_Active_Point(1);

	if (lastcount == 0)
		MS_CT_Change_Text(content, "no points available");
	else
	{
		snprintf(text, 128, "Selected Point: %i/%i", lastselect + 1, lastcount);
		MS_CT_Change_Text(content, text);
	}
	
}


void CamSys_Add_Point_Active_Spline(int type);
void MSCM_SS_Addpoint_ce(void)
{
	CamSys_Add_Point_Active_Spline(0);
}
void MSCM_SS_Addpoint_ve(void)
{
	CamSys_Add_Point_Active_Spline(1);
}


void CamSys_Add_Point_Active_Spline_BA(int type, int ba);
void MSCM_SS_Addpoint_before_selected_ce(void)
{
	CamSys_Add_Point_Active_Spline_BA(0, 1);
}
void MSCM_SS_Addpoint_before_selected_ve(void)
{
	CamSys_Add_Point_Active_Spline_BA(1, 1);
}

void MSCM_SS_Addpoint_after_selected_ce(void)
{
	CamSys_Add_Point_Active_Spline_BA(0, 0);
}
void MSCM_SS_Addpoint_after_selected_ve(void)
{
	CamSys_Add_Point_Active_Spline_BA(1, 0);
}

void CamSys_Active_Spline_Change_Active_Point(int type, int dir);

void MSCM_SS_Next_Point(void)
{
	CamSys_Active_Spline_Change_Active_Point(0, 1);
}
void MSCM_SS_Prev_Point(void)
{
	CamSys_Active_Spline_Change_Active_Point(0, 0);
}

void MSCM_SS_Next_Point_ve(void)
{
	CamSys_Active_Spline_Change_Active_Point(1, 1);
}
void MSCM_SS_Prev_Point_ve(void)
{
	CamSys_Active_Spline_Change_Active_Point(1, 0);
}


void CamSys_Remove_Point_Active_Spline(int type);
void MSCM_SS_Del_Point(void)
{
	CamSys_Remove_Point_Active_Spline(0);
}
void MSCM_SS_Del_Point_ve(void)
{
	CamSys_Remove_Point_Active_Spline(1);
}

void CamsSys_Set_Active_Point(int type);
void MSCM_SS_Setpoint_ce(void)
{
	CamsSys_Set_Active_Point(0);
}
void MSCM_SS_Setpoint_ve(void)
{
	CamsSys_Set_Active_Point(1);
}

char *CamSys_Menu_View_Numbers(void);


void CamsSys_Even_Out(int type);
void MSCM_SS_Even_Out_ce(void)
{
	CamsSys_Even_Out(0);
}

void MSCM_SS_Even_Out_ve(void)
{
	CamsSys_Even_Out(1);
}


void CamSys_Set_Starttime(int type);
void CamSys_Set_Stoptime(int type);
void MSCM_SS_Set_Starttime_ce(void)
{
	CamSys_Set_Starttime(0);
}

void MSCM_SS_Set_Stoptime_ce(void)
{
	CamSys_Set_Stoptime(0);
}

void MSCM_SS_Set_Starttime_ve(void)
{
	CamSys_Set_Starttime(1);
}

void MSCM_SS_Set_Stoptime_ve(void)
{
	CamSys_Set_Stoptime(1);
}

struct MS_Popup_Entry
{
	void *null;
};

struct MS_Popup_Menu_Menu
{
	void *null;
};

void CamSys_Copy_Active_Point(int type);
void MSCM_SS_Copypoint_ce(void)
{
	CamSys_Copy_Active_Point(0);
}
void MSCM_SS_Copypoint_ve(void)
{
	CamSys_Copy_Active_Point(1);
}

void CamSys_Paste_Active_Point(int type);
void MSCM_SS_Pastepoint_ce(void)
{
	CamSys_Paste_Active_Point(0);
}
void MSCM_SS_Pastepoint_ve(void)
{
	CamSys_Paste_Active_Point(1);
}

void CamSys_Time_CP(int type, int ss, int cp);
void MSCM_Copy_ce_starttime(void)
{
	CamSys_Time_CP(0, 0, 0);
}

void MSCM_Paste_ce_starttime(void)
{
	CamSys_Time_CP(0, 0, 1);
}
void MSCM_Copy_ce_stoptime(void)
{
	CamSys_Time_CP(0, 1, 0);
}

void MSCM_Paste_ce_stoptime(void)
{
	CamSys_Time_CP(0, 1, 1);
}
void MSCM_Copy_ve_starttime(void)
{
	CamSys_Time_CP(1, 0, 0);
}

void MSCM_Paste_ve_starttime(void)
{
	CamSys_Time_CP(1, 0, 1);
}
void MSCM_Copy_ve_stoptime(void)
{
	CamSys_Time_CP(1, 1, 0);
}

void MSCM_Paste_ve_stoptime(void)
{
	CamSys_Time_CP(1, 1, 1);
}


void CamSys_Duplicate_Selected_Spline(int type);
void MSCM_Duplicate_Spline_ce(void)
{
	CamSys_Duplicate_Selected_Spline(0);
}

void MSCM_Duplicate_Spline_ve(void)
{
	CamSys_Duplicate_Selected_Spline(1);
}


struct MS_Popup_Entry *MS_Popup_Add_Entry(struct MS_Popup_Menu_Menu *menu, struct MS_Popup_Entry *entry, char *name, void (*f)(void *data), void *data, char *command);
struct MS_Popup_Menu_Menu *MS_Popup_Add_Menu(void);
void MS_Popup_Menu_Set(struct MS_Popup_Menu_Menu *menu, int x, int y);
void Cam_Menu_Init(void)
{
	// Camera
	extern int *cem_resolution, *vem_resolution;
	extern float *cem_point_x, *cem_point_y, *cem_point_z,  *cem_curve_r, *cem_curve_g, *cem_curve_b, *cem_curve_a;
	extern float *vem_point_x, *vem_point_y, *vem_point_z, *vem_curve_r, *vem_curve_g, *vem_curve_b, *vem_curve_a;
	extern double *cem_starttime, *cem_stoptime, *vem_starttime, *vem_stoptime, *cem_point_time, *vem_point_time;
	extern float *cem_move_spline_x, *cem_move_spline_y, *cem_move_spline_z;
	extern float *vem_move_spline_x, *vem_move_spline_y, *vem_move_spline_z;
	extern struct MS_Popup_Menu_Menu MS_Popup_Menu_Root;
	

	MS_Popup_Add_Entry(&MS_Popup_Menu_Root, NULL, "Cam Menu", NULL, NULL, "MS_Activate_Menu ce_ident");
	MS_Popup_Add_Entry(&MS_Popup_Menu_Root, NULL, "View Menu", NULL, NULL, "MS_Activate_Menu ve_ident");

	cam_menu = MS_Add_Menu(10,10,400,200,"Cam edit menu","ce_ident", NULL);
	if (!cam_menu)
		return;
		
	MS_Add_Content_Text("ce_ident",NULL,"ce_st1",1,"Camera : ");
		MS_Add_Content("ce_ident", "ce_st1","ce_viewbt",2,1,0,"View",NULL,"MS_Activate_Menu ve_ident\n","f00",NULL,NULL,0,0,0,NULL,NULL, NULL);
		MS_Add_Content_Text("ce_ident","ce_viewbt","ce_divider3",0,"  --  ");
		MS_Add_Content("ce_ident", "ce_divider3","ce_dop",2,1,0,"Draw Options",NULL,"MS_Activate_Menu ce_dop\n","f00",NULL,NULL,0,0,0,NULL,NULL, NULL);
	MS_Add_Content("ce_ident", "ce_st1","ce_bt1",2,2,1,"Add Camera",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CamSys_Add_Cam, NULL);
	MS_Add_Content("ce_ident", "ce_bt1", "ce_camnumtext", 1,2,1, NULL, NULL , NULL, NULL, NULL, NULL, 0, 0, 0, NULL, &CamSys_Menu_Cam_Numbers, NULL);
	MS_Add_Content_Text("ce_ident","ce_camnumtext","ce_divider1",1,"----------------------------------------------");
	MS_Add_Content_Text("ce_ident","ce_divider1","ce_cnt",1,"Selected Cam: ");
		MS_Add_Content("ce_ident", "ce_cnt", "ce_test", 1,1,0, "test", NULL , NULL, NULL, NULL, NULL, 0, 0, 0, NULL, NULL, &MSCM_Update_ce_camnumtext);
		MS_Add_Content("ce_ident", "ce_test", "ce_next", 2,2,0, "  next", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Next_ce, NULL);
		MS_Add_Content("ce_ident", "ce_next", "ce_prev", 2,2,0, "  prev", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Prev_ce, NULL);
		MS_Add_Content_Text("ce_ident","ce_prev","ce_prev_delim",0,"  ---  ");
		MS_Add_Content("ce_ident", "ce_prev_delim", "ce_del", 2,2,0, "remove", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Remove_Active_Spline_ce, NULL);
		MS_Add_Content_Text("ce_ident","ce_del","ce_prev_delim1",0,"  ---  ");
		MS_Add_Content("ce_ident", "ce_prev_delim1", "ce_ctv", 2,2,0, "copy to view", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_Duplicate_Spline_ce, NULL);
		
	MS_Add_Content_Text("ce_ident","ce_cnt","ce_show",1,"Show: ");
		MS_Add_Content("ce_ident", "ce_show","ce_showtoggle",2,2,0,"test",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Update_ce_camshowtoggle_Function, &MSCM_Update_ce_camshowtoggle);
	MS_Add_Content_Text("ce_ident","ce_show","ce_enable",1,"Enable: ");
		MS_Add_Content("ce_ident", "ce_enable","ce_enabletoggle",2,2,0,"test",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Update_ce_camenabletoggle_Function, &MSCM_Update_ce_camenabletoggle);
	MS_Add_Content_Text("ce_ident","ce_enable","ce_starttime",1,"Start Time: ");
		MS_Add_Content_Slider_Width("ce_ident","ce_starttime","ce_starttime_slider",2,0,"d",&cem_starttime,0, 10000, 2);
		MS_Add_Content("ce_ident","ce_starttime_slider","ce_starttime_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&cem_starttime, NULL);
		MS_Add_Content("ce_ident", "ce_starttime_editbox", "ce_starttime_set", 2,2,0, "set", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Set_Starttime_ce, NULL);
		MS_Add_Content_Text("ce_ident","ce_starttime_set","ce_starttime_set_div",0," - ");
		MS_Add_Content("ce_ident", "ce_starttime_set_div","ce_starttime_copy",2,2,0,"copy",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Copy_ce_starttime, NULL);
		MS_Add_Content_Text("ce_ident","ce_starttime_copy","ce_starttime_set_div2",0," - ");
		MS_Add_Content("ce_ident", "ce_starttime_set_div2","ce_starttime_copy",2,2,0,"paste",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Paste_ce_starttime, NULL);
	MS_Add_Content_Text("ce_ident","ce_starttime","ce_stoptime",1,"Stop Time : ");
		MS_Add_Content_Slider_Width("ce_ident","ce_stoptime","ce_stoptime_slider",2,0,"d",&cem_stoptime,0, 10000, 2);
		MS_Add_Content("ce_ident","ce_stoptime_slider","ce_stoptime_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&cem_stoptime, NULL);
		MS_Add_Content("ce_ident", "ce_stoptime_editbox", "ce_stoptime_set", 2,2,0, "set", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Set_Stoptime_ce, NULL);		
		MS_Add_Content_Text("ce_ident","ce_stoptime_set","ce_stoptime_set_div",0," - ");
		MS_Add_Content("ce_ident", "ce_stoptime_set_div","ce_stoptime_copy",2,2,0,"copy",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Copy_ce_stoptime, NULL);
		MS_Add_Content_Text("ce_ident","ce_stoptime_copy","ce_stoptime_set_div2",0," - ");
		MS_Add_Content("ce_ident", "ce_stoptime_set_div2","ce_stoptime_copy",2,2,0,"paste",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Paste_ce_stoptime, NULL);
	MS_Add_Content_Text("ce_ident","ce_stoptime","ce_move_spline_text",1,"Move Spline : ");
		MS_Add_Content_Text("ce_ident","ce_move_spline_text","ce_move_spline_x_text",0,"x - ");
		MS_Add_Content_Slider_Width("ce_ident","ce_move_spline_x_text","ce_move_spline_x_slider",2,0,"f",&cem_move_spline_x,-100, 100, 2);
		MS_Add_Content_Text("ce_ident","ce_move_spline_x_slider","ce_move_spline_y_text",0," y - ");
		MS_Add_Content_Slider_Width("ce_ident","ce_move_spline_y_text","ce_move_spline_y_slider",2,0,"f",&cem_move_spline_y,-100, 100, 2);
		MS_Add_Content_Text("ce_ident","ce_move_spline_y_slider","ce_move_spline_z_text",0," z - ");
		MS_Add_Content_Slider_Width("ce_ident","ce_move_spline_z_text","ce_move_spline_z_slider",2,0,"f",&cem_move_spline_z,-100, 100, 2);
	MS_Add_Content_Text("ce_ident","ce_move_spline_text","ce_divider2",1,"----------------------------------------------");

	MS_Add_Content("ce_ident", "ce_divider2", "ce_pointnumtext", 1,1,1, "not working!", NULL , NULL, NULL, NULL, NULL, 0, 0, 0, NULL, NULL, &MSCM_Update_ce_pointumtext);
		MS_Add_Content_Text("ce_ident","ce_pointnumtext","ce_pointnumtext_div1",0," : ");
		MS_Add_Content("ce_ident", "ce_pointnumtext_div1", "ce_pt_next", 2,2,0, "next", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Next_Point, NULL);
		MS_Add_Content_Text("ce_ident","ce_pt_next","ce_pointnumtext_div2",0," -- ");
		MS_Add_Content("ce_ident", "ce_pointnumtext_div2", "ce_pt_prev", 2,2,0, "prev", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Prev_Point, NULL);
		MS_Add_Content_Text("ce_ident","ce_pt_prev","ce_pointnumtext_div3",0," -- ");
		MS_Add_Content("ce_ident", "ce_pointnumtext_div3", "ce_pt_del", 2,2,0, "delete", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Del_Point, NULL);
	MS_Add_Content_Text("ce_ident","ce_pointnumtext","ce_addpointtext",1,"Add point: ");
		MS_Add_Content("ce_ident", "ce_addpointtext", "ce_addpoint", 2,2,0, "add point", "will add a point to the spline" , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Addpoint_ce, NULL);
		MS_Add_Content_Text("ce_ident","ce_addpoint","ce_addpointtext_div1",0," - ");
		MS_Add_Content("ce_ident", "ce_addpointtext_div1", "ce_addpoint_before_selected_point", 2,2,0, "before_selected_point", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Addpoint_before_selected_ce, NULL);
		MS_Add_Content_Text("ce_ident","ce_addpoint_before_selected_point","ce_addpointtext_div2",0," - ");
		MS_Add_Content("ce_ident", "ce_addpointtext_div2", "ce_addpoint_after_selected_point", 2,2,0, "after_selected_point", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Addpoint_after_selected_ce, NULL);
	MS_Add_Content_Text("ce_ident","ce_addpointtext","ce_setpointtext",1,"Set point: ");
		MS_Add_Content("ce_ident", "ce_setpointtext", "ce_setpoint", 2,2,0, "current_position", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Setpoint_ce, NULL);
	MS_Add_Content_Text("ce_ident","ce_setpointtext","ce_point_x_text",1,"X : ");
		MS_Add_Content_Slider_Width("ce_ident","ce_point_x_text","ce_point_x_slider",2,0,"f",&cem_point_x,-100000, 100000, 2);
		MS_Add_Content("ce_ident","ce_point_x_slider","ce_point_x_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&cem_point_x, NULL);
	MS_Add_Content_Text("ce_ident","ce_point_x_text","ce_point_y_text",1,"Y : ");
		MS_Add_Content_Slider_Width("ce_ident","ce_point_y_text","ce_point_y_slider",2,0,"f",&cem_point_y,-100000, 100000, 2);
		MS_Add_Content("ce_ident","ce_point_y_slider","ce_point_y_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&cem_point_y, NULL);
	MS_Add_Content_Text("ce_ident","ce_point_y_text","ce_point_z_text",1,"Z : ");
		MS_Add_Content_Slider_Width("ce_ident","ce_point_z_text","ce_point_z_slider",2,0,"f",&cem_point_z,-100000, 100000, 2);
		MS_Add_Content("ce_ident","ce_point_z_slider","ce_point_z_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&cem_point_z, NULL);
	MS_Add_Content_Text("ce_ident","ce_point_z_text","ce_point_time_text",1,"time : ");
		MS_Add_Content_Slider_Width("ce_ident","ce_point_time_text","ce_point_time_slider",2,0,"d",&cem_point_time,0, 1, 2);
		MS_Add_Content("ce_ident","ce_point_time_slider","ce_point_time_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&cem_point_time, NULL);
	MS_Add_Content("ce_ident", "ce_point_time_text", "ce_copy_point", 2,2,1, "copy", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Copypoint_ce, NULL);
		MS_Add_Content_Text("ce_ident","ce_copy_point","ce_copy_point_div",0," - ");
		MS_Add_Content("ce_ident", "ce_copy_point_div", "ce_paste_point", 2,2,0, "paste", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Pastepoint_ce, NULL);
	MS_Add_Content("ce_ident", "ce_copy_point", "ce_even_out_speed", 2,2,1, "Even out speed", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Even_Out_ce, NULL);


	// Camera Draw Options
	MS_Add_Menu(10,10,200,200,"Cam Drawoptions","ce_dop", NULL);
	MS_Add_Content("ce_dop", NULL,"ce_btmm",2,1,0,"Back to the cam edit menu",NULL,"MS_Activate_Menu ce_ident\n","f00",NULL,NULL,0,0,0,NULL,NULL, NULL);
	MS_Add_Content_Text("ce_dop", "ce_btmm", "ce_resolution", 1, "Resolution: ");
		MS_Add_Content_Slider_Width("ce_dop","ce_resolution","ce_resolution_slider",2,0,"i",&cem_resolution,0,666, 3);
		MS_Add_Content("ce_dop","ce_resolution_slider","ce_resolution_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"i",&cem_resolution, NULL);
	MS_Add_Content_Text("ce_dop", "ce_resolution", "ce_color_r", 1, "Red  : ");
		MS_Add_Content_Slider_Width("ce_dop","ce_color_r","ce_color_r_slider",2,0,"f",&cem_curve_r,0,1, 2);
		MS_Add_Content("ce_dop","ce_color_r_slider","ce_color_r_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&cem_curve_r, NULL);
	MS_Add_Content_Text("ce_dop", "ce_color_r", "ce_color_g", 1, "Green: ");
		MS_Add_Content_Slider_Width("ce_dop","ce_color_g","ce_color_g_slider",2,0,"f",&cem_curve_g,0,1, 2);
		MS_Add_Content("ce_dop","ce_color_g_slider","ce_color_g_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&cem_curve_g, NULL);
	MS_Add_Content_Text("ce_dop", "ce_color_g", "ce_color_b", 1, "Blue : ");
		MS_Add_Content_Slider_Width("ce_dop","ce_color_b","ce_color_b_slider",2,0,"f",&cem_curve_b,0,1, 2);
		MS_Add_Content("ce_dop","ce_color_b_slider","ce_color_b_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&cem_curve_b, NULL);
	MS_Add_Content_Text("ce_dop", "ce_color_b", "ce_color_a", 1, "Alpha: ");
		MS_Add_Content_Slider_Width("ce_dop","ce_color_a","ce_color_a_slider",2,0,"f",&cem_curve_a,0,1, 2);
		MS_Add_Content("ce_dop","ce_color_a_slider","ce_color_a_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&cem_curve_a, NULL);


	// View
	MS_Add_Menu(10,10,400,200,"View Edit menu","ve_ident", NULL);
	MS_Add_Content("ve_ident", NULL,"ve_viewbt",2,1,0,"Camera",NULL,"MS_Activate_Menu ce_ident\n","f00",NULL,NULL,0,0,0,NULL,NULL, NULL);
		MS_Add_Content_Text("ve_ident","ve_viewbt","ve_st1",0," : View");
		MS_Add_Content_Text("ve_ident","ve_st1","ve_divider3",0,"  --  ");
		MS_Add_Content("ve_ident", "ve_divider3","ve_dop",2,1,0,"Draw Options",NULL,"MS_Activate_Menu ve_dop\n","f00",NULL,NULL,0,0,0,NULL,NULL, NULL);
	MS_Add_Content("ve_ident", "ve_viewbt","ve_bt1",2,2,1,"Add Camera",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&CamSys_Add_View, NULL);
	MS_Add_Content("ve_ident", "ve_bt1", "ve_camnumtext", 1,2,1, NULL, NULL , NULL, NULL, NULL, NULL, 0, 0, 0, NULL, &CamSys_Menu_View_Numbers, NULL);
	MS_Add_Content_Text("ve_ident","ve_camnumtext","ve_divider1",1,"----------------------------------------------");
	MS_Add_Content_Text("ve_ident","ve_divider1","ve_cnt",1,"Selected Cam: ");
		MS_Add_Content("ve_ident", "ve_cnt", "ve_test", 1,1,0, "test", NULL , NULL, NULL, NULL, NULL, 0, 0, 0, NULL, NULL, &MSCM_Update_ve_camnumtext);
		MS_Add_Content("ve_ident", "ve_test", "ve_next", 2,2,0, "  next", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Next_ve, NULL);
		MS_Add_Content("ve_ident", "ve_next", "ve_prev", 2,2,0, "  prev", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Prev_ve, NULL);
		MS_Add_Content_Text("ve_ident","ve_prev","ve_prev_delim",0,"  ---  ");
		MS_Add_Content("ve_ident", "ve_prev_delim", "ve_del", 2,2,0, "remove", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Remove_Active_Spline_ve, NULL);
		MS_Add_Content_Text("ve_ident","ve_del","ve_prev_delim1",0,"  ---  ");
		MS_Add_Content("ve_ident", "ve_prev_delim1", "ve_ctc", 2,2,0, "copy to cam", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_Duplicate_Spline_ve, NULL);
		
	MS_Add_Content_Text("ve_ident","ve_cnt","ve_show",1,"Show: ");
		MS_Add_Content("ve_ident", "ve_show","ve_showtoggle",2,2,0,"test",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Update_ve_camshowtoggle_Function, &MSCM_Update_ve_camshowtoggle);
	MS_Add_Content_Text("ve_ident","ve_show","ve_enable",1,"Enable: ");
		MS_Add_Content("ve_ident", "ve_enable","ve_enabletoggle",2,2,0,"test",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Update_ve_camenabletoggle_Function, &MSCM_Update_ve_camenabletoggle);
	MS_Add_Content_Text("ve_ident","ve_enable","ve_starttime",1,"Start Time: ");
		MS_Add_Content_Slider_Width("ve_ident","ve_starttime","ve_starttime_slider",2,0,"d",&vem_starttime,0, 100000, 2);
		MS_Add_Content("ve_ident","ve_starttime_slider","ve_starttime_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&vem_starttime, NULL);
		MS_Add_Content("ve_ident", "ve_starttime_editbox", "ve_starttime_set", 2,2,0, "set", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Set_Starttime_ve, NULL);
		MS_Add_Content_Text("ve_ident","ve_starttime_set","ve_starttime_set_div",0," - ");
		MS_Add_Content("ve_ident", "ve_starttime_set_div","ve_starttime_copy",2,2,0,"copy",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Copy_ve_starttime, NULL);
		MS_Add_Content_Text("ve_ident","ve_starttime_copy","ve_starttime_set_div2",0," - ");
		MS_Add_Content("ve_ident", "ve_starttime_set_div2","ve_starttime_copy",2,2,0,"paste",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Paste_ve_starttime, NULL);
	MS_Add_Content_Text("ve_ident","ve_starttime","ve_stoptime",1,"Stop Time : ");
		MS_Add_Content_Slider_Width("ve_ident","ve_stoptime","ve_stoptime_slider",2,0,"d",&vem_stoptime,0, 100000, 2);
		MS_Add_Content("ve_ident","ve_stoptime_slider","ve_stoptime_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&vem_stoptime, NULL);
		MS_Add_Content("ve_ident", "ve_stoptime_editbox", "ve_stoptime_set", 2,2,0, "set", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Set_Stoptime_ve, NULL);
		MS_Add_Content_Text("ve_ident","ve_stoptime_set","ve_stoptime_set_div",0," - ");
		MS_Add_Content("ve_ident", "ve_stoptime_set_div","ve_stoptime_copy",2,2,0,"copy",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Copy_ve_stoptime, NULL);
		MS_Add_Content_Text("ve_ident","ve_stoptime_copy","ve_stoptime_set_div2",0," - ");
		MS_Add_Content("ve_ident", "ve_stoptime_set_div2","ve_stoptime_copy",2,2,0,"paste",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&MSCM_Paste_ve_stoptime, NULL);


	MS_Add_Content_Text("ve_ident","ve_stoptime","ve_move_spline_text",1,"Move Spline : ");
		MS_Add_Content_Text("ve_ident","ve_move_spline_text","ve_move_spline_x_text",0,"x - ");
		MS_Add_Content_Slider_Width("ve_ident","ve_move_spline_x_text","ve_move_spline_x_slider",2,0,"f",&vem_move_spline_x,-100, 100, 2);
		MS_Add_Content_Text("ve_ident","ve_move_spline_x_slider","ve_move_spline_y_text",0," y - ");
		MS_Add_Content_Slider_Width("ve_ident","ve_move_spline_y_text","ve_move_spline_y_slider",2,0,"f",&vem_move_spline_y,-100, 100, 2);
		MS_Add_Content_Text("ve_ident","ve_move_spline_y_slider","ve_move_spline_z_text",0," z - ");
		MS_Add_Content_Slider_Width("ve_ident","ve_move_spline_z_text","ve_move_spline_z_slider",2,0,"f",&vem_move_spline_z,-100, 100, 2);


	MS_Add_Content_Text("ve_ident","ve_move_spline_text","ve_divider2",1,"----------------------------------------------");

	MS_Add_Content("ve_ident", "ve_divider2", "ve_pointnumtext", 1,1,1, "not working!", NULL , NULL, NULL, NULL, NULL, 0, 0, 0, NULL, NULL, &MSCM_Update_ve_pointumtext);
		MS_Add_Content_Text("ve_ident","ve_pointnumtext","ve_pointnumtext_div1",0," : ");
		MS_Add_Content("ve_ident", "ve_pointnumtext_div1", "ve_pt_next", 2,2,0, "next", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Next_Point_ve, NULL);
		MS_Add_Content_Text("ve_ident","ve_pt_next","ve_pointnumtext_div2",0," -- ");
		MS_Add_Content("ve_ident", "ve_pointnumtext_div2", "ve_pt_prev", 2,2,0, "prev", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Prev_Point_ve, NULL);
		MS_Add_Content_Text("ve_ident","ve_pt_prev","ve_pointnumtext_div3",0," -- ");
		MS_Add_Content("ve_ident", "ve_pointnumtext_div3", "ve_pt_del", 2,2,0, "delete", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Del_Point_ve, NULL);
	MS_Add_Content_Text("ve_ident","ve_pointnumtext","ve_addpointtext",1,"Add point: ");
		MS_Add_Content("ve_ident", "ve_addpointtext", "ve_addpoint", 2,2,0, "add point", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Addpoint_ve, NULL);
		MS_Add_Content_Text("ve_ident","ve_addpoint","ve_addpointtext_div1",0," - ");
		MS_Add_Content("ve_ident", "ve_addpointtext_div1", "ve_addpoint_before_selected_point", 2,2,0, "before_selected_point", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Addpoint_before_selected_ve, NULL);
		MS_Add_Content_Text("ve_ident","ve_addpoint_before_selected_point","ve_addpointtext_div2",0," - ");
		MS_Add_Content("ve_ident", "ve_addpointtext_div2", "ve_addpoint_after_selected_point", 2,2,0, "after_selected_point", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Addpoint_after_selected_ve, NULL);
	MS_Add_Content_Text("ve_ident","ve_addpointtext","ve_setpointtext",1,"Set point: ");
		MS_Add_Content("ve_ident", "ve_setpointtext", "ve_setpoint", 2,2,0, "current_position", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Setpoint_ve, NULL);
	MS_Add_Content_Text("ve_ident","ve_setpointtext","ve_point_x_text",1,"X : ");
		MS_Add_Content_Slider_Width("ve_ident","ve_point_x_text","ve_point_x_slider",2,0,"f",&vem_point_x,-100000, 100000, 2);
		MS_Add_Content("ve_ident","ve_point_x_slider","ve_point_x_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&vem_point_x, NULL);
	MS_Add_Content_Text("ve_ident","ve_point_x_text","ve_point_y_text",1,"Y : ");
		MS_Add_Content_Slider_Width("ve_ident","ve_point_y_text","ve_point_y_slider",2,0,"f",&vem_point_y,-100000, 100000, 2);
		MS_Add_Content("ve_ident","ve_point_y_slider","ve_point_y_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&vem_point_y, NULL);
	MS_Add_Content_Text("ve_ident","ve_point_y_text","ve_point_z_text",1,"Z : ");
		MS_Add_Content_Slider_Width("ve_ident","ve_point_z_text","ve_point_z_slider",2,0,"f",&vem_point_z,-100000, 100000, 2);
		MS_Add_Content("ve_ident","ve_point_z_slider","ve_point_z_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&vem_point_z, NULL);
	MS_Add_Content_Text("ve_ident","ve_point_z_text","ve_point_time_text",1,"time : ");
		MS_Add_Content_Slider_Width("ve_ident","ve_point_time_text","ve_point_time_slider",2,0,"d",&vem_point_time,0, 1, 2);
		MS_Add_Content("ve_ident","ve_point_time_slider","ve_point_time_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"d",&vem_point_time, NULL);
	MS_Add_Content("ve_ident", "ve_point_time_text", "ve_copy_point", 2,2,1, "copy", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Copypoint_ve, NULL);
		MS_Add_Content_Text("ve_ident","ve_copy_point","ve_copy_point_div",0," - ");
		MS_Add_Content("ve_ident", "ve_copy_point_div", "ve_paste_point", 2,2,0, "paste", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Pastepoint_ve, NULL);
	MS_Add_Content("ve_ident", "ve_copy_point", "ve_even_out_speed", 2,2,1, "Even out speed", NULL , NULL, "f00", NULL, NULL, 0, 0, 0, NULL, &MSCM_SS_Even_Out_ve, NULL);	


			// Camera Draw Options
	MS_Add_Menu(10,10,200,200,"Cam Drawoptions","ve_dop", NULL);
	MS_Add_Content("ve_dop", NULL,"ve_btmm",2,1,0,"Back to the cam edit menu",NULL,"MS_Activate_Menu ve_ident\n","f00",NULL,NULL,0,0,0,NULL,NULL, NULL);
	MS_Add_Content_Text("ve_dop", "ve_btmm", "ve_resolution", 1, "Resolution: ");
		MS_Add_Content_Slider_Width("ve_dop","ve_resolution","ve_resolution_slider",2,0,"i",&vem_resolution,0,666, 3);
		MS_Add_Content("ve_dop","ve_resolution_slider","ve_resolution_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"i",&vem_resolution, NULL);
	MS_Add_Content_Text("ve_dop", "ve_resolution", "ve_color_r", 1, "Red  : ");
		MS_Add_Content_Slider_Width("ve_dop","ve_color_r","ve_color_r_slider",2,0,"f",&vem_curve_r,0,1, 2);
		MS_Add_Content("ve_dop","ve_color_r_slider","ve_color_r_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&vem_curve_r, NULL);
	MS_Add_Content_Text("ve_dop", "ve_color_r", "ve_color_g", 1, "Green: ");
		MS_Add_Content_Slider_Width("ve_dop","ve_color_g","ve_color_g_slider",2,0,"f",&vem_curve_g,0,1, 2);
		MS_Add_Content("ve_dop","ve_color_g_slider","ve_color_g_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&vem_curve_g, NULL);
	MS_Add_Content_Text("ve_dop", "ve_color_g", "ve_color_b", 1, "Blue : ");
		MS_Add_Content_Slider_Width("ve_dop","ve_color_b","ve_color_b_slider",2,0,"f",&vem_curve_b,0,1, 2);
		MS_Add_Content("ve_dop","ve_color_b_slider","ve_color_b_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&vem_curve_b, NULL);
	MS_Add_Content_Text("ve_dop", "ve_color_b", "ve_color_a", 1, "Alpha: ");
		MS_Add_Content_Slider_Width("ve_dop","ve_color_a","ve_color_a_slider",2,0,"f",&vem_curve_a,0,1, 2);
		MS_Add_Content("ve_dop","ve_color_a_slider","ve_color_a_editbox",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0,0,"f",&vem_curve_a, NULL);




	MS_Activate_Menu_Struct(cam_menu);
	CamSys_Event_Init();

	NMENU_Autosave_Init();
	NMENU_Trackentity_Init();
	NMENU_Demo_Init();
	
}




void Cam_Menu_Frame(void)
{




}
