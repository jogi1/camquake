#include "quakedef.h"
#include "new_menu.h"
#include "gl_model.h"
#include "gl_local.h"
#include "cam_new.h"
#include "splines.h"


int qglProject (float objx, float objy, float objz, float *model, float *proj, int *view, float* winx, float* winy, float* winz);
#define TE_IDENT "trackentity_ident"


int te_running;
int te_vc = 0;
int te_show;
double te_interval;
double *te_interval_p = &te_interval;

struct MS_Popup_Menu_Menu **entity_popups;
int	last_entity_count;

vec3_t	tracked_entity_origin, track_offset;
struct CamSys_Spline *record_spline;
double record_start_time;


struct te_data
{
	entity_t	*entity;
	player_state_t *player;
};

struct te_data tracked_entity;

void NMENU_Trackentity_Stop(void)
{
	int point_count;
	vec3_t	pos, pos1, pos2;
	float *ppos;

	Spline_Insert_Point(record_spline->spline, tracked_entity_origin, NULL, 0, NULL);
	record_spline->stoptime = cls.demotime - demostarttime;

	point_count = Spline_Get_Point_Count(record_spline->spline);

	if (point_count > 3)
	{
		ppos = Spline_Get_Point_Origin_By_Number(record_spline->spline, 1);
		VectorSet(pos1, ppos[0], ppos[1], ppos[2]);
		ppos = Spline_Get_Point_Origin_By_Number(record_spline->spline, 2);
		VectorSet(pos2, ppos[0], ppos[1], ppos[2]);
		VectorSubtract(pos1, pos2, pos);
		VectorAdd(pos1, pos, pos1);
		Spline_Insert_Point(record_spline->spline, pos1, NULL, 1, NULL);
		
		ppos = Spline_Get_Point_Origin_By_Number(record_spline->spline, point_count-1);
		VectorSet(pos1, ppos[0], ppos[1], ppos[2]);
		ppos = Spline_Get_Point_Origin_By_Number(record_spline->spline, point_count);
		VectorSet(pos2, ppos[0], ppos[1], ppos[2]);
		VectorSubtract(pos2, pos1, pos);
		VectorAdd(pos2, pos, pos2);
		Spline_Insert_Point(record_spline->spline, pos2, NULL, 0, NULL);
	}
	

	






	te_running = 0;
	tracked_entity.entity = NULL;
	tracked_entity.player = NULL;
	record_spline = NULL;

}

void NMENU_Trackentity_Start(void)
{
	te_running = 1;

	if (te_interval < 0.01)
	{
		te_interval = 0.01;
	}

	if (te_interval > 1)
		te_interval = 1;

	record_spline = CamSys_Add_Spline(te_vc);
	if (!record_spline)
	{
		te_running = 0;
		return;
	}

	record_start_time = record_spline->starttime = cls.demotime - demostarttime;
	Spline_Insert_Point(record_spline->spline, tracked_entity_origin, NULL, 0, NULL);

}





void MS_Popup_Track(void *data)
{
	struct te_data *te_data;

	if (!data)
		return;

	te_data = data;
	if (te_data->entity)
	{
		tracked_entity.entity = te_data->entity;
		VectorCopy(tracked_entity.entity->origin, tracked_entity_origin);
	}
	if (te_data->player)
	{
		tracked_entity.player = te_data->player;
		VectorCopy(tracked_entity.player->origin, tracked_entity_origin);
	}

	Cbuf_AddText("MS_Activate_Menu trackentity_ident\n");
}

void NMENU_Trackentity_Frame(void)
{
	extern visentlist_t cl_visents;
	int i, j, player_count;
	int view[4];
	float x,y,z, model[16], project[16];
	struct te_data *te_data;
	float distance, newdistance ,founddistance;
	founddistance = 9999999;

	if (entity_popups)
	{
		for (i=0;i<last_entity_count;i++)
			MS_Popup_Remove_Menu(entity_popups[i]);
		free(entity_popups);
		entity_popups = NULL;
	}

	if (tracked_entity.player || tracked_entity.entity)
	{

		distance = -1;
		if (tracked_entity.entity)
		{
			for (i=0;i<cl_visents.count;i++)
			{
				newdistance = sqrt(pow((cl_visents.list[i].origin[0] - tracked_entity_origin[0]),2) + pow((cl_visents.list[i].origin[1] - tracked_entity_origin[1]),2) + pow((cl_visents.list[i].origin[2] - tracked_entity_origin[2]),2));
				if (distance > newdistance || distance == -1)
				{
					founddistance = distance = newdistance ;
					tracked_entity.entity = &cl_visents.list[i];				
				}
			}
		}

		if (tracked_entity.player)
		{
			for (i=0;i<32;i++)
			{
				if (cl.players[i].spectator == 1 || cl.players[i].name[0] == '\0')
					continue;

				newdistance = sqrt(pow((cl.frames[cl.validsequence & UPDATE_MASK].playerstate[i].origin[0] - tracked_entity_origin[0]),2) + pow((cl.frames[cl.validsequence & UPDATE_MASK].playerstate[i].origin[1] - tracked_entity_origin[1]),2) + pow((cl.frames[cl.validsequence & UPDATE_MASK].playerstate[i].origin[2] - tracked_entity_origin[2]),2));
				if (distance > newdistance || distance == -1)
				{
					founddistance = distance = newdistance ;
					tracked_entity.player = &cl.frames[cl.validsequence & UPDATE_MASK].playerstate[i];
				}

			}
		}
	
		if (founddistance < 10)
		{
			if (tracked_entity.entity)
				VectorCopy(tracked_entity.entity->origin, tracked_entity_origin);
			if (tracked_entity.player)
				if (tracked_entity.player->origin)
					VectorCopy(tracked_entity.player->origin, tracked_entity_origin);
			if (record_spline && (cls.demotime - record_start_time) >= te_interval)
			{
				record_start_time = cls.demotime;
				Spline_Insert_Point(record_spline->spline, tracked_entity_origin, NULL, 0, NULL);

			}
		}
		else
		{
			if (te_running)
				NMENU_Trackentity_Stop();
			tracked_entity.entity = NULL;
			tracked_entity.player = NULL;
		}
	}



	if (te_running || !te_show)
		return;

	for (i=0,player_count=0;i<32;i++)
		if (cl.players[i].spectator == 0 && cl.players[i].name[0] != '\0')
			player_count++;

	if (cl_visents.count == 0 && player_count == 0)
		return;

	last_entity_count = cl_visents.count + player_count;
	entity_popups = calloc(last_entity_count, sizeof(struct MS_Popup_Menu_Menu));

	glGetFloatv(GL_MODELVIEW_MATRIX, model);
	glGetFloatv(GL_PROJECTION_MATRIX, project);
	glGetIntegerv(GL_VIEWPORT, (GLint *)view);

	for (i=0;i<cl_visents.count;i++)
	{
		entity_popups[i] = MS_Popup_Add_Menu();
		MS_Popup_Menu_Set_Type(entity_popups[i], 1);
		te_data = calloc(1, sizeof(struct te_data));
		te_data->entity = &cl_visents.list[i];
		MS_Popup_Add_Entry(entity_popups[i], NULL, "track", &MS_Popup_Track, te_data, NULL);

		if (R_CullSphere(cl_visents.list[i].origin, 4))
		{
			MS_Popup_Menu_Set(entity_popups[i], -666, -666);
		}
		else
		{
			qglProject(cl_visents.list[i].origin[0], cl_visents.list[i].origin[1], cl_visents.list[i].origin[2], model, project, view, &x, &y, &z);
			x =  x * vid.width / glwidth;
			y =  (glheight - y) * vid.height / glheight;
			MS_Popup_Menu_Set(entity_popups[i], x, y);
		}
	}

	for (i=0, j=cl_visents.count; i<32; i++)
	{
		if (cl.players[i].spectator == 1 || cl.players[i].name[0] == '\0')
			continue;

		entity_popups[j] = MS_Popup_Add_Menu();
		MS_Popup_Menu_Set_Type(entity_popups[j], 1);
		te_data = calloc(1, sizeof(struct te_data));
		te_data->player = &cl.frames[cl.validsequence & UPDATE_MASK].playerstate[i];
		MS_Popup_Add_Entry(entity_popups[j], NULL, "track", &MS_Popup_Track, te_data, NULL);

		if (R_CullSphere(te_data->player->origin, 4))
		{
			MS_Popup_Menu_Set(entity_popups[j], -666, -666);
		}
		else
		{
			qglProject(te_data->player->origin[0], te_data->player->origin[1], te_data->player->origin[2], model, project, view, &x, &y, &z);
			x =  x * vid.width / glwidth;
			y =  (glheight - y) * vid.height / glheight;
			MS_Popup_Menu_Set(entity_popups[j], x, y);
		}
		j++;
	}
}

void NMENU_VC_Togglebutton_Update(struct menu_ct *self)
{
	if (te_vc)
		MS_CT_Change_Text(self, "View");
	else
		MS_CT_Change_Text(self, "Cam");

}

void NMENU_VC_Togglebutton(void)
{
	if (!te_vc)
		te_vc = 1;
	else if (te_vc)
		te_vc = 0;
}

void NMENU_VC_Togglebutton1(void)
{
	if (te_running == 0)
		NMENU_Trackentity_Start();
	else
		NMENU_Trackentity_Stop();
}

void NMENU_VC_Togglebutton_Update1(struct menu_ct *self)
{
	if (te_running)
		MS_CT_Change_Text(self, "Stop");
	else
		MS_CT_Change_Text(self, "Start");

}

void NMENU_VC_Togglebutton2(void)
{
	if (te_show == 0)
		te_show = 1;
	else
		te_show = 0;
}

void NMENU_VC_Togglebutton_Update2(struct menu_ct *self)
{
	if (te_show)
		MS_CT_Change_Text(self, "Hide");
	else
		MS_CT_Change_Text(self, "Show");

}



char *NMENU_Position_Text(void)
{
	static char string[512];

	snprintf(string, 512, "Position: %f %f %f", tracked_entity_origin[0], tracked_entity_origin[1], tracked_entity_origin[2]);
	return &string[0];
}

struct menu_s *trackentity_menu;
void NMENU_Trackentity_Init(void)
{
	extern struct MS_Popup_Menu_Menu MS_Popup_Menu_Root;
	struct MS_Popup_Entry *popup;
	te_running = te_vc = 0;
	

	popup = MS_Popup_Add_Entry(&MS_Popup_Menu_Root, NULL, "Track Entity Menu", NULL, NULL, "MS_Activate_Menu trackentity_ident\n");

	if (!popup)
		return;


	trackentity_menu = MS_Add_Menu(0, 0, 200, 100, "Track Entity Menu", TE_IDENT, NULL);

	if (!trackentity_menu)
		return;

	MS_Add_Content_Text(TE_IDENT, NULL,"button_track_as_text",1,"Track as :  ");
		MS_Add_Content(TE_IDENT, "button_track_as_text","togglebutton",2,2,0,"test",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_VC_Togglebutton, &NMENU_VC_Togglebutton_Update);
	MS_Add_Content_Text(TE_IDENT, "button_track_as_text","slider_interval_text",1,"Interval :  ");
		MS_Add_Content_Slider_Width(TE_IDENT,"slider_interval_text","slider_interval",2,0,"d",&te_interval_p,0,1, 2);
			MS_Add_Content(TE_IDENT,"slider_interval","editbox_interval",4,2,0,NULL,NULL,NULL,NULL,NULL,NULL,10,0.001,1,"d",&te_interval_p, NULL);
	MS_Add_Content(TE_IDENT, "slider_interval_text", "position_text", 1,2,1, NULL, NULL , NULL, NULL, NULL, NULL, 0, 0, 0, NULL, &NMENU_Position_Text, NULL);

	MS_Add_Content(TE_IDENT, "position_text","button_track_startstop",2,2,1,"test",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_VC_Togglebutton1, &NMENU_VC_Togglebutton_Update1);
		MS_Add_Content_Text(TE_IDENT, "button_track_startstop","button_track_startstop_text",0," recording");


	MS_Add_Content(TE_IDENT, "button_track_startstop","button_track_showhide",2,2,1,"test",NULL,NULL,"f00",NULL,NULL,0,0,0,NULL,&NMENU_VC_Togglebutton2, &NMENU_VC_Togglebutton_Update2);
}
