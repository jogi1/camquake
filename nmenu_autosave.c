#include "quakedef.h"
#include "timer.h"
#include "new_menu.h"

struct timer *running_timer;

#define AS_IDENT "auto_save_ident"

qbool On_Change_man(cvar_t *var, char string, qbool *cancel)
{


}

cvar_t	menu_autosave_name	=	{"menu_autosave_name", "none"};
cvar_t	menu_autosave_interval	=	{"menu_autosave_interval", "30"};


void NMENU_Start_Button_Function(void)
{


	if (!running_timer)
	{
		running_timer = Timer_Add(menu_autosave_interval.value , va("camsys_write_config %s_$dates", menu_autosave_name.string), NULL, 0);
	}
	else
	{
		Timer_Delete_Struct(running_timer);
		running_timer = NULL;
	}




}

void NMENU_Start_Button_Update_Function(struct menu_ct *self)
{

	if (!running_timer)
	{
		MS_CT_Change_Text(self, "start");
	}
	else
	{
		MS_CT_Change_Text(self, "stop");
	}




}



struct menu_s *auto_save_menu;

void NMENU_Autosave_Init(void)
{

	extern struct MS_Popup_Menu_Menu MS_Popup_Menu_Root;
	struct MS_Popup_Menu_Menu *popup;
	

	popup = MS_Popup_Add_Entry(&MS_Popup_Menu_Root, NULL, "Autosave Menu", NULL, NULL, "MS_Activate_Menu auto_save_ident\n");

	if (!popup)
		return;

	auto_save_menu = MS_Add_Menu(0, 0, 200, 100, "Autosave Menu", AS_IDENT, NULL);

	if (!auto_save_menu)
		return;

	MS_Add_Content_Text(AS_IDENT, NULL,"editbox_name_text",1,"Config : ");

	MS_Add_Content(AS_IDENT , "editbox_name_text", "editbox_name" ,4,1,0,NULL,"enter config name here without .cfg",NULL,NULL,NULL,&menu_autosave_name,10,0,0,NULL,NULL,NULL);

	MS_Add_Content_Text(AS_IDENT, "editbox_name_text" ,"editbox_interval_text",1,"Interval : ");

	MS_Add_Content(AS_IDENT , "editbox_interval_text", "editbox_interval" ,4,1,0,NULL,"enter config name here",NULL,NULL,NULL,&menu_autosave_interval,10,0,0,NULL,NULL,NULL);
	

	MS_Add_Content(AS_IDENT, "editbox_interval_text","togglebutton",2,2,1,"test",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_Start_Button_Function, &NMENU_Start_Button_Update_Function);

	Cvar_Register(&menu_autosave_name);
	Cvar_Register(&menu_autosave_interval);

}
