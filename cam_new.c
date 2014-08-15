// New Camera System
#include "quakedef.h"
#include "gl_model.h"
#include "gl_local.h"
#include "new_menu.h"
#include "teamplay.h"
#include "splines.h"
#include "cam_event_system.h"
#include "3dtext.h"

#define CS_STARTTIME 0
#define CS_STOPTIME 1
#define CS_COLOR_R 2
#define CS_COLOR_G 4
#define CS_COLOR_B 5
#define CS_COLOR_A 6
#define CS_SHOW 7
#define CS_ENABLE 8



#define lhrandom(MIN,MAX) ((rand() & 32767) * (((MAX)-(MIN)) * (1.0f / 32767.0f)) + (MIN))

cvar_t	camsys_enable	=	{"camsys_enable", "0"};

struct CamSys_Spline *active_view_spline;
struct CamSys_Spline *active_cam_spline;

struct CamSys_Spline
{
	struct	CamSys_Spline *next, *prev;
	void	*spline;
	double	starttime, stoptime;
	float	r,g,b,a;
	int		show, enable, resolution, number;
	int		active_point;
	struct	event_list *event_list;

};

struct CamSys_Spline_Data
{
	int draw;
	double time;
};

struct CamSys_Spline *view_splines, *cam_splines, *selected_view_spline, *selected_cam_spline, *plugin_splines;
int spline_count[2];

void CamSys_Update_Numbers(int type);




int *cem_resolution, *vem_resolution;
int dummy_int;
float dummy_float;
double dummy_double;
float *cem_point_x, *cem_point_y, *cem_point_z, *cem_curve_r, *cem_curve_g, *cem_curve_b, *cem_curve_a;
float *vem_point_x, *vem_point_y, *vem_point_z, *vem_curve_r, *vem_curve_g, *vem_curve_b, *vem_curve_a;
double *cem_starttime, *cem_stoptime, *vem_starttime, *vem_stoptime, *cem_point_time, *vem_point_time;
float *cem_move_spline_x, *cem_move_spline_y, *cem_move_spline_z;
float real_cem_move_spline_x, real_cem_move_spline_y, real_cem_move_spline_z;
float *vem_move_spline_x, *vem_move_spline_y, *vem_move_spline_z;
float real_vem_move_spline_x, real_vem_move_spline_y, real_vem_move_spline_z;


struct MS_Popup_Menu_Menu **cam_popups, **view_popups;

int qglProject (float objx, float objy, float objz, float *model, float *proj, int *view, float* winx, float* winy, float* winz);
void MS_Popup_Select_Helper(void *data);
void MS_Popup_Delete_Helper(void *data);
struct select_helper
{
	struct CamSys_Spline *spline;
	int point;

};



void CamSys_Popups_Cleanup(struct CamSys_Spline *spline, struct CamSys_Spline *last_selected, int *last_point_count, struct MS_Popup_Menu_Menu ***popups, vec3_t last_pos, vec3_t last_view)
{
	int i;
	if (*last_point_count == 0)
		return;

	if (*popups)
	{
		for (i=0;i<*last_point_count;i++)
		{

			MS_Popup_Remove_Menu((*popups)[i]);
			
		}
		free(*popups);
		*popups = NULL;

	}
	*last_point_count = 0;

}


void CamSys_Popups_Setup(struct CamSys_Spline *spline, struct CamSys_Spline *last_selected, int *last_point_count, struct MS_Popup_Menu_Menu ***popups, vec3_t last_pos, vec3_t last_view)
{
	vec3_t vpos;
	int i, view[4];
	float x,y,z, model[16], project[16], *pos;
	struct select_helper *sh;

	if (spline->show == 0)
	{
		if (*popups)
		{
			for (i=0;i<*last_point_count;i++)
			{

				MS_Popup_Remove_Menu((*popups)[i]);
				
			}
			free(*popups);
			*popups = NULL;

		}
		return;
	}


	if (spline != last_selected || Spline_Get_Point_Count(spline->spline) != *last_point_count)
	{

		if (*popups)
		{
			for (i=0;i<*last_point_count;i++)
			{

				MS_Popup_Remove_Menu((*popups)[i]);
				
			}
			free(*popups);
			*popups = NULL;

		}

		*last_point_count = Spline_Get_Point_Count(spline->spline);
		last_selected = spline;
		
		if (spline->show == 0)
			return;

		if (*last_point_count > 0)
		{
			*popups = calloc(*last_point_count, sizeof (struct MS_Popup_Menu_Menu**));
			if (*popups == NULL)
				return;
		}


		glGetFloatv(GL_MODELVIEW_MATRIX, model);
		glGetFloatv(GL_PROJECTION_MATRIX, project);
		glGetIntegerv(GL_VIEWPORT, (GLint *)view);


		for (i=0;i<*last_point_count;i++)
		{
			pos = Spline_Get_Point_Origin_By_Number(last_selected->spline, i+1);
			if (!pos)
				break;

			(*popups)[i] = MS_Popup_Add_Menu();
			sh = calloc(1, sizeof(struct select_helper));
				
			sh->point = i;
			sh->spline = last_selected;
			MS_Popup_Add_Entry((*popups)[i], NULL, "select", &MS_Popup_Select_Helper, sh, NULL);
			sh = calloc(1, sizeof(struct select_helper));
			sh->point = i;
			sh->spline = last_selected;
			MS_Popup_Add_Entry((*popups)[i], NULL, "delete", &MS_Popup_Delete_Helper, sh, NULL);

			VectorSet(vpos, pos[0], pos[1], pos[2]);
			if (R_CullSphere(vpos, 4))
			{
				MS_Popup_Menu_Set((*popups)[i], -666, -666);
			}
			else
			{
				qglProject(pos[0], pos[1], pos[2], model, project, view, &x, &y, &z);
				x =  x * vid.width / glwidth;
				y =  (glheight - y) * vid.height / glheight;
				MS_Popup_Menu_Set((*popups)[i], x, y);
			}
		}
	}
	else
	{
		if (VectorCompare(r_refdef.vieworg, last_pos) && VectorCompare(r_refdef.viewangles, last_view))
		{
		}
		else
		{
			glGetFloatv(GL_MODELVIEW_MATRIX, model);
			glGetFloatv(GL_PROJECTION_MATRIX, project);
			glGetIntegerv(GL_VIEWPORT, (GLint *)view);

			for (i=0;i<*last_point_count;i++)
			{
				pos = Spline_Get_Point_Origin_By_Number(last_selected->spline, i+1);
				if (!pos)
					break;


				VectorSet(vpos, pos[0], pos[1], pos[2]);
				if (R_CullSphere(vpos, 4))
				{
					MS_Popup_Menu_Set((*popups)[i], -666, -666);
				}
				else
				{
					qglProject(pos[0], pos[1], pos[2], model, project, view, &x, &y, &z);
					x =  x * vid.width / glwidth;
					y =  (glheight - y) * vid.height / glheight;
					MS_Popup_Menu_Set((*popups)[i], x, y);
				}
			}
		}

	}
}

void CamSys_Update_Popups(void)
{
	static struct CamSys_Spline *last_scs, *last_svs;
	static int last_cpc, last_vpc;
	static vec3_t last_pos, last_view;


	if (selected_cam_spline)
		CamSys_Popups_Setup(selected_cam_spline, last_scs, &last_cpc, &cam_popups, last_pos, last_view);
	else
		CamSys_Popups_Cleanup(selected_cam_spline, last_scs, &last_cpc, &cam_popups, last_pos, last_view);

	if (selected_view_spline)
		CamSys_Popups_Setup(selected_view_spline, last_svs, &last_vpc, &view_popups, last_pos, last_view);
	else
		CamSys_Popups_Cleanup(selected_view_spline, last_svs, &last_vpc, &view_popups, last_pos, last_view);


	if (!(VectorCompare(r_refdef.vieworg, last_pos) && VectorCompare(r_refdef.viewangles, last_view)))
	{
		VectorCopy(r_refdef.vieworg, last_pos);
		VectorCopy(r_refdef.viewangles, last_view);
	}

}

void CamSys_Move_Spline(struct CamSys_Spline *spline, float x, float y, float z);
void CamSys_Update_Menu_Pointer(void)
{
	float	*pos;
	double *point_time;
	
	if (selected_cam_spline)
	{
		
		cem_move_spline_x = &real_cem_move_spline_x;
		cem_move_spline_y = &real_cem_move_spline_y;
		cem_move_spline_z = &real_cem_move_spline_z;

		Spline_Check_Selected_Point_Time(selected_cam_spline->spline, selected_cam_spline->active_point + 1);
		cem_resolution = &selected_cam_spline->resolution;
		pos = Spline_Get_Point_Origin_By_Number(selected_cam_spline->spline, selected_cam_spline->active_point + 1);
		cem_curve_a = &selected_cam_spline->a;
		cem_curve_r = &selected_cam_spline->r;
		cem_curve_g = &selected_cam_spline->g;
		cem_curve_b = &selected_cam_spline->b;
		cem_starttime = &selected_cam_spline->starttime;
		cem_stoptime = &selected_cam_spline->stoptime;
		if (pos)
		{
			cem_point_x = &pos[0];
			cem_point_y = &pos[1];
			cem_point_z = &pos[2];
		}
		else
		{
			cem_point_x = cem_point_y = cem_point_z = &dummy_float;
		}
		point_time = Spline_Get_Point_Time_By_Number(selected_cam_spline->spline, selected_cam_spline->active_point + 1);
		if (point_time)
		{
			cem_point_time = point_time;
		}
		else
		{
			cem_point_time = &dummy_double;
		}
		if (real_cem_move_spline_x || real_cem_move_spline_y || real_cem_move_spline_z)
		{
			CamSys_Move_Spline(selected_cam_spline, real_cem_move_spline_x, real_cem_move_spline_y, real_cem_move_spline_z);
			real_cem_move_spline_x = real_cem_move_spline_y = real_cem_move_spline_z = 0;
		}
	}
	else
	{
		cem_move_spline_x = &real_cem_move_spline_x;
		cem_move_spline_y = &real_cem_move_spline_y;
		cem_move_spline_z = &real_cem_move_spline_z;
		cem_starttime = cem_stoptime = cem_point_time = &dummy_double;
		cem_resolution = &dummy_int;
		cem_point_x = cem_point_y = cem_point_z  = cem_curve_r = cem_curve_g = cem_curve_b = cem_curve_a = &dummy_float;
		real_cem_move_spline_x = real_cem_move_spline_y = real_cem_move_spline_z = 0;
	}

	if (selected_view_spline)
	{

		vem_move_spline_x = &real_vem_move_spline_x;
		vem_move_spline_y = &real_vem_move_spline_y;
		vem_move_spline_z = &real_vem_move_spline_z;

		Spline_Check_Selected_Point_Time(selected_view_spline->spline, selected_view_spline->active_point + 1);
		vem_resolution = &selected_view_spline->resolution;
		pos = Spline_Get_Point_Origin_By_Number(selected_view_spline->spline, selected_view_spline->active_point + 1);
		vem_curve_a = &selected_view_spline->a;
		vem_curve_r = &selected_view_spline->r;
		vem_curve_g = &selected_view_spline->g;
		vem_curve_b = &selected_view_spline->b;
		vem_starttime = &selected_view_spline->starttime;
		vem_stoptime = &selected_view_spline->stoptime;
		if (pos)
		{
			vem_point_x = &pos[0];
			vem_point_y = &pos[1];
			vem_point_z = &pos[2];
		}
		else
		{
			vem_point_x = vem_point_y = vem_point_z = &dummy_float;
		}
		point_time = Spline_Get_Point_Time_By_Number(selected_view_spline->spline, selected_view_spline->active_point + 1);
		if (point_time)
		{
			vem_point_time = point_time;
		}
		else
		{
			vem_point_time = &dummy_double;
		}
		if (real_vem_move_spline_x || real_vem_move_spline_y || real_vem_move_spline_z)
		{
			CamSys_Move_Spline(selected_view_spline, real_vem_move_spline_x, real_vem_move_spline_y, real_vem_move_spline_z);
			real_vem_move_spline_x = real_vem_move_spline_y = real_vem_move_spline_z = 0;
		}
	}
	else
	{
		vem_move_spline_x = &real_vem_move_spline_x;
		vem_move_spline_y = &real_vem_move_spline_y;
		vem_move_spline_z = &real_vem_move_spline_z;
		vem_starttime = vem_stoptime = vem_point_time = &dummy_double;
		vem_resolution = &dummy_int;
		vem_point_x = vem_point_y = vem_point_z  = vem_curve_r = vem_curve_g = vem_curve_b = vem_curve_a = &dummy_float;
		real_cem_move_spline_x = real_cem_move_spline_y = real_cem_move_spline_z = 0;
	}



}


void CamSys_Cleanup_Spline(struct CamSys_Spline *spline)
{
	Spline_Remove(spline->spline);

}

struct CamSys_Spline *CamSys_Alloc_Spline(void)
{
	struct CamSys_Spline		*css;
	struct event_list			*event_list;
	css = calloc(1, sizeof(struct CamSys_Spline));
	
	if (!css)
	{
		Com_Printf("CamSys_Alloc_Spline: calloc error!\n");
		return NULL;
	}

	event_list = CamSys_Event_List_Alloc();
	if (!event_list)
	{
		Com_Printf("CamSys_Alloc_Spline: calloc error event_list!\n");
		free(css);
		return NULL;
	}


	css->spline = (void *)Spline_Add(NULL);
	css->resolution = 10;
	css->r = lhrandom(1, 255)/255.0f;
	css->g = lhrandom(1, 255)/255.0f;
	css->b = lhrandom(1, 255)/255.0f;
	css->a = 1;
	css->show = 1;
	css->event_list = event_list;

	return css;
}

struct CamSys_Spline *CamSys_Add_Spline(int type)
{
	struct CamSys_Spline *css, *csss;
	css = CamSys_Alloc_Spline();
	if (!css)
	{
		Com_Printf("CamSys_Add_Spline: error look ^\n");
		return NULL;
	}
	if (type == 0)
	{
		if (cam_splines == NULL)
		{
			cam_splines = css;
			selected_cam_spline = css;
			CamSys_Update_Numbers(type);
			CamSys_Update_Menu_Pointer();
			return css;
		}
		else
		{
			csss = cam_splines;
		}
	}
	else if (type == 1)
	{
		if (view_splines == NULL)
		{
			view_splines = css;
			selected_view_spline = css;
			CamSys_Update_Numbers(type);
			CamSys_Update_Menu_Pointer();
			return css;
		}
		else
		{
			csss = view_splines;
		}
	}
	else if (type == 2)
	{
		if (plugin_splines == NULL)
		{
			plugin_splines = css;
			return css;
		}
		else
		{
			csss = plugin_splines;
		}
	}
	else
	{
		Com_Printf("CamSys_Add_Spline: error - Type has to be 0/1/2\n");
		CamSys_Cleanup_Spline(css);
		return NULL;
	}

	while (csss->next)
		csss = csss->next;

	csss->next = css;
	css->prev = csss;

	CamSys_Update_Numbers(type);
	CamSys_Update_Menu_Pointer();
	return css;
}

void CamSys_Update_Numbers(int type)
{

	int i = 0;

	struct CamSys_Spline *spline;

	if (type == 0)
		spline = cam_splines;
	else if (type == 1)
		spline = view_splines;
	else 
		return;

	if (!spline)
		return;

	while(spline)
	{
		spline->number = i++;
		spline = spline->next;
	}

	spline_count[type] = i;

	


}

void CamSys_Remove_Spline(struct CamSys_Spline *spline, int number, int type)
{
	struct CamSys_Spline *lspline, *lst;

	if ((!spline && !number) || type < 0 || type > 1)
	{
		return;
	}

	if (type == 0)
		lst = cam_splines;
	else
		lst = view_splines;

	if (!spline)
	{
		lspline = lst;
		while (lspline)
		{
			if (lspline->number == number)
				break;
			lspline = lspline->next;
		}

		if (!lspline)
			return;
	}
	else
	{
		lspline = spline;
	}

	spline_count[type]--;

	if (lspline->next && lspline->prev)
	{
		lspline->next->prev = lspline->prev;
		lspline->prev->next = lspline->next;
		CamSys_Cleanup_Spline(lspline);
	}
	else if (lspline->next && !lspline->prev)
	{
		lspline->next->prev = NULL;
		if (type == 0)
			cam_splines = lspline->next;
		if (type == 1)
			view_splines = lspline->next;
		CamSys_Cleanup_Spline(lspline);
	}
	else if (!lspline->next && lspline->prev)
	{
		lspline->prev->next = NULL;
		CamSys_Cleanup_Spline(lspline);
	}
	else if (!lspline->next && !lspline->prev)
	{
		if (type == 0)
			cam_splines = NULL;
		if (type == 1)
			view_splines = NULL;
		CamSys_Cleanup_Spline(lspline);
		return;
	}

	CamSys_Update_Numbers(type);

	


}

void CamSys_Remove_Active_Spline(int type)
{
	if (type == 0)
	{
		CamSys_Remove_Spline(selected_cam_spline, -1, type);
		selected_cam_spline = cam_splines;
	}
	if (type == 1)
	{
		CamSys_Remove_Spline(selected_view_spline, -1, type);
		selected_view_spline = view_splines;
	}
}

void CamSys_Remove_Spline_By_Number(int num, int type)
{
	CamSys_Remove_Spline(NULL, num, type);
}

struct CamSys_Spline *CamSys_Spline_Get(int type, int number)
{
	struct CamSys_Spline *spline;
	int i;

	if (number < 0)
		return NULL;

	if (type == 0)
	{
		spline = cam_splines;
	}
	else if (type == 1)
	{
		spline = view_splines;	
	}
	else
		return NULL;

	if (number == 0)
	{
		return spline;
	}
	else
	{
		i = number;
		while(spline)
		{
			if (i-- == 0)
				return spline;
			spline = spline->next;
		}
	}

	return NULL;



}

void CamSys_Frame(void)
{
	if (!camsys_enable.value)
		return;
	
	CamSys_Update_Menu_Pointer();
	CamSys_Event_Update_Pointer();

	if (plugin_splines)
	{
		//Com_Printf("%i\n", Spline_Get_Point_Count(plugin_splines->spline));
	}
}

void CamSys_Draw_Spline(struct CamSys_Spline *spline)
{
	int i, j, k;
	vec3_t	position;
	char text[128];
	double *time;

	i = Spline_Get_Point_Count(spline->spline);
	
	for (j=0;j<=i;j++)
	{
		Spline_Get_Point_Position(spline->spline, j, position);	

		glDisable(GL_TEXTURE_2D);
		glColor4f(spline->r, spline->g, spline->b, spline->a);
		glPointSize(5);
		glBegin(GL_POINTS);
		glVertex3fv((float *)&position);
		glEnd();
		if (j == spline->active_point+1 && (spline == selected_cam_spline || spline == selected_view_spline))
		{
			time = Spline_Get_Point_Time_By_Number(spline->spline, j);
			Draw_3D_Text_Position_Setup_Matrix(position, r_refdef.vieworg, 1);
			Draw_3D_Text_Setup_Drawable_Rectangle(130, 50);
			
			glColor4f(spline->r, spline->g, spline->b, spline->a);
			glRectf(0, 0, 130, 50);
			glColor4f(0,0,0,spline->a);
			glRectf(1, 1, 129, 49);
			glEnable(GL_TEXTURE_2D);
			

			snprintf(text, 128, "point: %i", j);
			Draw_String(0, 0, text);
			snprintf(text, 128, "x: %f", position[0]);
			Draw_String(8, 10, text);
			snprintf(text, 128, "y: %f", position[1]);
			Draw_String(8, 20, text);
			snprintf(text, 128, "z: %f", position[2]);
			Draw_String(8, 30, text);

			if (time)
			{
				snprintf(text, 128, "time: %f", *time * (spline->stoptime - spline->starttime));
				Draw_String(8, 40, text);
			}
			Draw_3D_Text_Pop_Matrix();
			Draw_3D_Text_Remove_Drawable_Rectangle();
		}

	}
	
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINE_STRIP);

	glColor4f(spline->r, spline->g, spline->b, spline->a);

	if (i < 4)
	{
		for (j = 0; j <=spline->resolution; j++)
		{
			Spline_Calculate_From_Point(spline->spline, (float)j/spline->resolution, position, 1);
			glVertex3fv((float *)&position);
		}
	}
	else
	{
		for (k = 0 ; k < i-3 ; k++)
		{
			for (j = 0; j <=spline->resolution; j++)
			{
				Spline_Calculate_From_Point(spline->spline, (float)j/spline->resolution, position, k + 1);
				glVertex3fv((float *)&position);
			}
		}


	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
}

void CamSys_Draw_Spline_Plugin(struct CamSys_Spline *spline)
{
	int i, j, k;
	vec3_t	position;
	double *time;

	i = Spline_Get_Point_Count(spline->spline);
	
	for (j=0;j<=i;j++)
	{
		Spline_Get_Point_Position(spline->spline, j, position);	

		glDisable(GL_TEXTURE_2D);
		glColor4f(spline->r, spline->g, spline->b, spline->a);
		glPointSize(5);
		glBegin(GL_POINTS);
		glVertex3fv((float *)&position);
		glEnd();
	}
	
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINE_STRIP);

	glColor4f(spline->r, spline->g, spline->b, spline->a);

	if (i < 4)
	{
		for (j = 0; j <=spline->resolution; j++)
		{
			Spline_Calculate_From_Point(spline->spline, (float)j/spline->resolution, position, 1);
			glVertex3fv((float *)&position);
		}
	}
	else
	{
		for (k = 0 ; k < i-3 ; k++)
		{
			for (j = 0; j <=spline->resolution; j++)
			{
				Spline_Calculate_From_Point(spline->spline, (float)j/spline->resolution, position, k + 1);
				glVertex3fv((float *)&position);
			}
		}


	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
}




void CamSys_Draw(void)
{
	struct CamSys_Spline *spline;

	if (!camsys_enable.value)
		return;

	CamSys_Update_Popups();
	NMENU_Trackentity_Frame();

	spline = plugin_splines;
	while (spline)
	{
		if (spline->show)
			CamSys_Draw_Spline(spline);
		spline = spline->next;
	}

	spline = cam_splines;
	while (spline)
	{
		if (spline->show)
			CamSys_Draw_Spline(spline);
		spline = spline->next;
	}

	spline = view_splines;
	while (spline)
	{
		if (spline->show)
			CamSys_Draw_Spline(spline);
		spline = spline->next;
	}
}


void CamSys_Print_Help(void)
{


}





void CamSys_Set_Spline(struct CamSys_Spline *spline, int type, double value)
{

	if (!spline)
		return;

	switch (type)
	{
		case CS_STARTTIME:
			spline->starttime = value;
			break;

		case CS_STOPTIME:
			spline->stoptime = value;
			break;

		case CS_COLOR_R:
			spline->r = value;
			break;

		case CS_COLOR_G:
			spline->g = value;
			break;

		case CS_COLOR_B:
			spline->b = value;
			break;

		case CS_COLOR_A:
			spline->a = value;
			break;

		case CS_SHOW:
			spline->show = value;
			break;

		case CS_ENABLE:
			spline->enable = value;
			break;
	}
}

void CamSys_Add(void)
{

	struct CamSys_Spline *spline;
	int i;
	vec3_t pos;

	if (Cmd_Argc() < 2)
	{
		CamSys_Print_Help();
		return;
	}

	if (!strcmp(Cmd_Argv(1), "list"))
	{
		if (!strcmp(Cmd_Argv(2), "cam"))
		{
			spline = cam_splines;
			i = 0;
			Com_Printf("Listing Cam Splines\n");
			while (spline)
			{
				Com_Printf(" %2i: point_count %i\n",i++, Spline_Get_Point_Count(spline->spline));
				Com_Printf("   - starttime  : %f\n", spline->starttime);
				Com_Printf("   - stoptime   : %f\n", spline->stoptime);
				spline = spline->next;
			}
		}
		if (!strcmp(Cmd_Argv(2), "view"))
		{
			CamSys_Add_Spline(1);
		}
	}

	if (!strcmp(Cmd_Argv(1), "add"))
	{
		if (!strcmp(Cmd_Argv(2), "cam"))
		{
			CamSys_Add_Spline(0);
		}
		if (!strcmp(Cmd_Argv(2), "view"))
		{
			CamSys_Add_Spline(1);
		}
	}

	if (!strcmp(Cmd_Argv(1), "cam"))
	{
		spline = CamSys_Spline_Get(0, atoi(Cmd_Argv(2)));
		if (!spline)
			return;

	}

	if (!strcmp(Cmd_Argv(1), "view"))
	{
		spline = CamSys_Spline_Get(1, atoi(Cmd_Argv(2)));
		if (!spline)
			return;
	}

	if (Cmd_Argc() < 3)
		return;

	if (!strcmp(Cmd_Argv(3), "set"))
	{
		if (Cmd_Argc() == 6)
		{
			if (!strcmp(Cmd_Argv(4), "starttime"))
			{
				CamSys_Set_Spline(spline, CS_STARTTIME, atof(Cmd_Argv(5)));
				return;
			}

			if (!strcmp(Cmd_Argv(4), "stoptime"))
			{
				CamSys_Set_Spline(spline, CS_STOPTIME, atof(Cmd_Argv(5)));
				return;
			}

			if (!strcmp(Cmd_Argv(4), "r"))
			{
				CamSys_Set_Spline(spline, CS_COLOR_R, atof(Cmd_Argv(5)));
				return;
			}

			if (!strcmp(Cmd_Argv(4), "g"))
			{
				CamSys_Set_Spline(spline, CS_COLOR_G, atof(Cmd_Argv(5)));
				return;
			}

			if (!strcmp(Cmd_Argv(4), "b"))
			{
				CamSys_Set_Spline(spline, CS_COLOR_B, atof(Cmd_Argv(5)));
				return;
			}


			if (!strcmp(Cmd_Argv(4), "a"))
			{
				CamSys_Set_Spline(spline, CS_COLOR_A, atof(Cmd_Argv(5)));
				return;
			}

			if (!strcmp(Cmd_Argv(4), "show"))
			{
				CamSys_Set_Spline(spline, CS_SHOW, atof(Cmd_Argv(5)));
				return;
			}

			if (!strcmp(Cmd_Argv(4), "enable"))
			{
				CamSys_Set_Spline(spline, CS_ENABLE, atof(Cmd_Argv(5)));
				return;
			}


		}

		return;
	}

	
	if (!strcmp(Cmd_Argv(3), "addpoint"))
	{
		if (Cmd_Argc() == 4 )
			Spline_Insert_Point(spline->spline, r_refdef.vieworg, NULL, 0, NULL);
		if (Cmd_Argc() == 7 )
		{
			VectorSet(pos, atoi(Cmd_Argv(4)), atoi(Cmd_Argv(5)), atoi(Cmd_Argv(6)));
			Spline_Insert_Point(spline->spline, pos, NULL, 0, NULL);
		}
		return;
	}


	if (!strcmp(Cmd_Argv(3), "setpoint"))
	{
		if (Cmd_Argc() == 6)
		{
			Spline_Set_Point_Time_By_Number(spline->spline, atoi(Cmd_Argv(4))+1, atof(Cmd_Argv(5)));	
		}

		return;
	}

	if (!strcmp(Cmd_Argv(3), "removepoint"))
	{
		Spline_Remove_Point_By_Number(spline->spline, atoi(Cmd_Argv(4)));
		return;
	}

	if (!strcmp(Cmd_Argv(3), "event"))
	{
		if (Cmd_Argc() < 4)
			return;

		if (!strcmp(Cmd_Argv(4), "list"))
		{
			CamSys_Event_Print(spline->event_list);
			return;
		}

		if (!strcmp(Cmd_Argv(4), "add"))
		{
			if (Cmd_Argc() < 5)
				return;
			CamSys_Event_Add_Entry(spline->event_list, Cmd_Argv(5));
		}

		if (!strcmp(Cmd_Argv(4), "add_value"))
		{
			if (Cmd_Argc() < 8)
				return;
			if (Cmd_Argc() == 9)
				CamSys_Event_Add_Value_By_Number(spline->event_list, atoi(Cmd_Argv(5)), atof(Cmd_Argv(6)), atof(Cmd_Argv(7)), Cmd_Argv(8));
			else if (Cmd_Argc() == 8)
				CamSys_Event_Add_Value_By_Number(spline->event_list, atoi(Cmd_Argv(5)), atof(Cmd_Argv(6)), atof(Cmd_Argv(7)), NULL);
			return;

		}

		

	}


}


void CamSys_Write_Spline(FILE *f, struct CamSys_Spline *spline, int cam_count, int type)
{
	int i;
	float *pos;
	double *time;
	char *vc;


	if (!f || !spline)
		return;

	if (type == 0)
		vc = "cam";
	else if (type == 1)
		vc = "view";
	else
		return;

	fprintf(f, "camsys add %s\n", vc);
	fprintf(f, "camsys %s %i set starttime %f\n", vc, cam_count, spline->starttime);
	fprintf(f, "camsys %s %i set stoptime %f\n", vc, cam_count, spline->stoptime);
	fprintf(f, "camsys %s %i set enable %i\n", vc, cam_count, spline->enable);
	fprintf(f, "camsys %s %i set show %i\n", vc, cam_count, spline->show);
	fprintf(f, "camsys %s %i set r %f\n", vc, cam_count, spline->r);
	fprintf(f, "camsys %s %i set g %f\n", vc, cam_count, spline->g);
	fprintf(f, "camsys %s %i set b %f\n", vc, cam_count, spline->b);
	fprintf(f, "camsys %s %i set a %f\n", vc, cam_count, spline->a);

	for (i = 1; i <= Spline_Get_Point_Count(spline->spline); i++)
	{
		pos = Spline_Get_Point_Origin_By_Number(spline->spline, i);
		fprintf(f, "camsys %s %i addpoint %f %f %f\n", vc, cam_count, pos[0], pos[1], pos[2]);

		time = Spline_Get_Point_Time_By_Number(spline->spline, i);
		fprintf(f, "camsys %s %i setpoint %i %f\n", vc, cam_count, i-1, *time);

	}
}



void CamSys_Write_Events(struct event_list *list, int camnum, int type, FILE *f);

void CamSys_Write_Config(void)
{
	FILE *f;
	char *outfile;
	struct CamSys_Spline *spline;
	int i;


	if (Cmd_Argc() < 1)
		return;

	outfile = va("%s/qw/%s.cfg", com_basedir, Cmd_Argv(1));


	f = fopen(outfile, "wb");

	if (!f)
		return;

	spline = cam_splines;

	fprintf(f, "camsys_enable 0\n");
	i = 0;
	while (spline)
	{

		CamSys_Write_Spline(f, spline, i, 0);
		CamSys_Write_Events(spline->event_list, i++, 0, f);
		spline = spline->next;
	}


	spline = view_splines;
	i = 0;
	while (spline)
	{

		CamSys_Write_Spline(f, spline, i, 1);
		CamSys_Write_Events(spline->event_list, i++, 1, f);
		spline = spline->next;
	}



	fprintf(f, "camsys_enable 1\n");


	fclose(f);
}


void CamsSys_Even_Out(int type)
{
	if (type == 0 && selected_cam_spline)
		Spline_Even_Speed(selected_cam_spline->spline);

	if (type == 1 && selected_view_spline)
		Spline_Even_Speed(selected_view_spline->spline);

}

void CamSys_Set_Starttime(int type)
{
	if (type == 0 && selected_cam_spline)
		selected_cam_spline->starttime = cls.demotime - demostarttime;

	if (type == 1 && selected_view_spline)
		selected_view_spline->starttime = cls.demotime - demostarttime;
}

void CamSys_Set_Stoptime(int type)
{
	if (type == 0 && selected_cam_spline)
		selected_cam_spline->stoptime = cls.demotime - demostarttime;

	if (type == 1 && selected_view_spline)
		selected_view_spline->stoptime= cls.demotime - demostarttime;
}


vec3_t	CamSys_Copied_Point;

void CamSys_Copy_Active_Point(int type)
{
	float *pos;

	if (type == 0 && selected_cam_spline)
	{
		pos = Spline_Get_Point_Origin_By_Number(selected_cam_spline->spline, selected_cam_spline->active_point + 1);
		VectorSet(CamSys_Copied_Point, pos[0], pos[1], pos[2]);
		return;
	}

	if (type == 1 && selected_view_spline)
	{
		pos = Spline_Get_Point_Origin_By_Number(selected_view_spline->spline, selected_view_spline->active_point + 1);
		VectorSet(CamSys_Copied_Point, pos[0], pos[1], pos[2]);
		return;
	}
}

void CamSys_Paste_Active_Point(int type)
{
	if (type == 0 && selected_cam_spline)
	{
		Spline_Set_Point_From_Number(selected_cam_spline->spline, selected_cam_spline->active_point + 1, CamSys_Copied_Point);
	}

	if (type == 1 && selected_view_spline)
	{
		Spline_Set_Point_From_Number(selected_view_spline->spline, selected_view_spline->active_point + 1, CamSys_Copied_Point);
	}

}


void CamSys_Init(void)
{
	Cvar_Register(&camsys_enable);
	Cmd_AddCommand("camsys", CamSys_Add);
	Cmd_AddCommand("camsys_write_config", CamSys_Write_Config);
	CamSys_Update_Menu_Pointer();
}



void CamSys_MenuHook(void)
{

}



// Functions for the menu

void CamSys_Add_Cam(void)
{
	CamSys_Add_Spline(0);
}

void CamSys_Add_View(void)
{
	CamSys_Add_Spline(1);
}

char *CamSys_Menu_Cam_Numbers(void)
{
	static char string[512];

	snprintf(string, 512, "Available Cameras: %i", spline_count[0]);
	return &string[0];
}

char *CamSys_Menu_View_Numbers(void)
{
	static char string[512];

	snprintf(string, 512, "Available Cameras: %i", spline_count[1]);
	return &string[0];

}

int CamSys_Get_Spline_Count(int type)
{
	return spline_count[type];
}

int CamSys_Get_Active_Spline_Show(int type)
{
	if (type == 0)
	{
		if (selected_cam_spline)
			return selected_cam_spline->show;
	}
	else if (type == 1)
	{
		if (selected_view_spline)
			return selected_view_spline->show;
	}
	
	return -666;
}

int CamSys_Get_Active_Spline_Enable(int type)
{
	if (type == 0)
	{
		if (selected_cam_spline)
			return selected_cam_spline->enable;
	}
	else if (type == 1)
	{
		if (selected_view_spline)
			return selected_view_spline->enable;
	}
	
	return -666;
}

void CamSys_Toggle_Active_Spline(int type, int var)
{
	struct CamSys_Spline *spline = NULL;

	if (type == 0)
	{
		if (selected_cam_spline)
			spline = selected_cam_spline;
		else
			return;
	}
	else if (type == 1)
	{
		if (selected_view_spline)
			spline = selected_view_spline;
		else 
			return;
	}

	if (var == 0)
	{
		spline->enable = !spline->enable;
		
	}
	else if (var == 1)
	{
		spline->show = !spline->show;
	}
}

int CamSys_Get_Active_Spline_Number(int type)
{
	struct CamSys_Spline *spline = NULL;

	if (type == 0)
	{
		if (selected_cam_spline)
			spline = selected_cam_spline;
		else
			return 0;
	}
	else if (type == 1)
	{
		if (selected_view_spline)
			spline = selected_view_spline;
		else 
			return 0;
	}


	return spline->number;

}

void CamSys_Active_Spline_Change(int type, int dir)
{
	struct CamSys_Spline *spline;

	if (type == 0)
		spline = selected_cam_spline;
	else if (type == 1)
		spline = selected_view_spline;
	else
		return;

	if (!spline)
	{
		return;
	}

	if (dir == 1)
	{
		if (spline->next)
			spline = spline->next;
	}
	else if (dir == 0)
	{
		if (spline->prev)
			spline = spline->prev;
	}

	if (type == 0)
		selected_cam_spline = spline;
	else if (type == 1)
		selected_view_spline = spline;

	if (type == 0)
	{
		cem_resolution = &selected_cam_spline->resolution;
	}
	else if (type == 1)
	{


	}

}

void CamSys_Active_Spline_Change_Active_Point(int type, int dir)
{
	struct CamSys_Spline *spline;

	if (type == 0)
		spline = selected_cam_spline;
	else if (type == 1)
		spline = selected_view_spline;
	else
		return;

	if (!spline)
	{
		return;
	}

	if (dir == 1)
	{
		spline->active_point++;
		if (spline->active_point >= Spline_Get_Point_Count(spline->spline))
			spline->active_point = Spline_Get_Point_Count(spline->spline) -1;
	}
	else if (dir == 0)
	{
		spline->active_point--;
		if (spline->active_point < 1)
			spline->active_point = 0;
	}

}


int Spline_Get_Point_Count(struct spline *spline);
int CamSys_Get_Active_Spline_Point_Count(int type)
{
		struct CamSys_Spline *spline;

	if (type == 0)
		spline = selected_cam_spline;
	else if (type == 1)
		spline = selected_view_spline;
	else
		return -1;

	if (!spline)
	{
		return -1;
	}

	return Spline_Get_Point_Count(spline->spline);


}


int CamSys_Get_Active_Spline_Active_Point(int type)
{
	struct CamSys_Spline *spline;

	if (type == 0)
		spline = selected_cam_spline;
	else if (type == 1)
		spline = selected_view_spline;
	else
		return -1;

	if (!spline)
	{
		return -1;
	}

	return spline->active_point;
}

void CamSys_Add_Point_Active_Spline(int type)
{
	struct CamSys_Spline *spline;

	if (type == 0)
		spline = selected_cam_spline;
	else if (type == 1)
		spline = selected_view_spline;
	else
		return;

	if (!spline)
	{
		return;
	}

	Spline_Insert_Point(spline->spline, r_refdef.vieworg, NULL, 0, NULL);
}
struct spline_point *Spline_Get_Point_From_Number(struct spline *spline, int point_number);
void CamSys_Add_Point_Active_Spline_BA(int type, int ba)
{
	struct CamSys_Spline *spline;
	struct spline_point *spp;

	if (type > 1 || type < 0 )
		return;

	if (type == 0)
		spline = selected_cam_spline;
	else if (type == 1)
		spline = selected_view_spline;
	else
		return;

	if (!spline)
	{
		return;
	}
	spp = Spline_Get_Point_From_Number(spline->spline, spline->active_point + 1);

	Spline_Insert_Point(spline->spline, r_refdef.vieworg, spp, ba, NULL);
}

void CamSys_Remove_Point_Active_Spline(int type)
{
	if (type == 0 && selected_cam_spline)
	{
		Spline_Remove_Point_By_Number(selected_cam_spline->spline, selected_cam_spline->active_point + 1);
	}
	else if (type == 1 && selected_view_spline)
	{
		Spline_Remove_Point_By_Number(selected_view_spline->spline, selected_view_spline->active_point + 1);
	}
}

void CamsSys_Set_Active_Point(int type)
{
	if (type == 0 && selected_cam_spline)
	{
		Spline_Set_Point_From_Number(selected_cam_spline->spline, selected_cam_spline->active_point + 1, r_refdef.vieworg);
	}
	else if (type == 1 && selected_view_spline)
	{
		Spline_Set_Point_From_Number(selected_view_spline->spline, selected_view_spline->active_point + 1, r_refdef.vieworg);
	}
	
	
}








struct CamSys_Spline *CamSys_Check_Active_Spline(struct CamSys_Spline *spline, double time)
{
	struct CamSys_Spline *active_spline = NULL;

	if (!spline)
		return NULL;

	active_spline = spline;

	if (time >= active_spline->starttime && time <= active_spline->stoptime && active_spline->enable)
		return active_spline;
	

	return NULL;
}

struct CamSys_Spline *CamSys_Find_Active_Spline(struct CamSys_Spline *spline, double time)
{
	struct CamSys_Spline *active_spline = NULL;

	active_spline = spline;

	while (active_spline)
	{
		if (active_spline->enable)
		{
			if (time >= active_spline->starttime && time <= active_spline->stoptime)
				return active_spline;
		}

		active_spline = active_spline->next;
	}

	return NULL;
}


void CamSys_Get_Spline_Point_Time(struct CamSys_Spline *spline, double in_time, double *out_time, int *out_point)
{


}

int CamSys_Get_Position(struct CamSys_Spline *spline, double time, vec3_t position)
{
	double time1, time2;
	int point_count;

	if (!spline)
		return 0;

	point_count = Spline_Get_Point_Count(spline->spline);
	
	time1 = time - spline->starttime;
	time2 = spline->stoptime - spline->starttime;
	if (time2 == 0)
		return 0;

	if (point_count < 4)
	{
		Spline_Calculate_From_Point(spline->spline, time1/time2, position, 1);
	}
	else
	{
		Spline_Calculate_From_Point(spline->spline, time1/time2, position, 0);
	}
	return 1;

}

void CamSys_Vector_To_Angles(vec3_t vec, vec3_t ang) 
{
	float forward, yaw, pitch;
	

		yaw = (atan2(vec[1], vec[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (vec[0]*vec[0] + vec[1]*vec[1]);
		pitch =(atan2(vec[2], forward) * 180 / M_PI);

	ang[0] = pitch;
	ang[1] = yaw;
	ang[2] = 0;
}

void CamSys_Handler(void)
{
	vec3_t cam_pos, view_pos, temp;
	double time, time1;

	if (!camsys_enable.value)
		return;

	time = cls.demotime - demostarttime;

	active_view_spline = CamSys_Check_Active_Spline(active_view_spline, time);
	active_cam_spline = CamSys_Check_Active_Spline(active_cam_spline, time);

	if (active_view_spline == NULL)
	{
		active_view_spline = CamSys_Find_Active_Spline(view_splines, time);
	}

	if (active_cam_spline == NULL)
	{
		active_cam_spline = CamSys_Find_Active_Spline(cam_splines, time);
	}

	
	
	if (active_cam_spline)
	{
		if (CamSys_Get_Position(active_cam_spline, time, cam_pos))
			VectorCopy(cam_pos, r_refdef.vieworg);
		
	}

	if (active_view_spline)
	{
		if (CamSys_Get_Position(active_view_spline, time, view_pos))
		{
			VectorSubtract(view_pos, cam_pos, temp);
			CamSys_Vector_To_Angles(temp, temp);
			VectorCopy(temp,r_refdef.viewangles);
			r_refdef.viewangles[ROLL]= 0;
			r_refdef.viewangles[0] = -r_refdef.viewangles[0];

		}
	}
	if (active_cam_spline)
	{
		time1 = (time - active_cam_spline->starttime)/(active_cam_spline->stoptime - active_cam_spline->starttime);
		CamSys_Event_Handle(active_cam_spline->event_list, time1);
	}
	if (active_view_spline)
	{
		time1 = (time - active_view_spline->starttime)/(active_view_spline->stoptime - active_view_spline->starttime);
		CamSys_Event_Handle(active_view_spline->event_list, time1);
	}
}

void MS_Popup_Select_Helper(void *data)
{
	struct select_helper *sh;

	if (!data)
		return;

	sh = (struct select_helper *)data;

	sh->spline->active_point = sh->point;


}

void MS_Popup_Delete_Helper(void *data)
{
	struct select_helper *sh;

	if (!data)
		return;

	sh = (struct select_helper *)data;

	Spline_Remove_Point_By_Number(sh->spline->spline, sh->point + 1);
}



double copied_time;

void CamSys_Time_CP(int type, int ss, int cp)
{
	struct CamSys_Spline *spline = NULL;

	if (type == 0)
		spline = selected_cam_spline;
	if (type == 1)
		spline = selected_view_spline;

	if (!spline)
		return;

	if (cp == 0)
	{
		if (ss == 0)
			copied_time = spline->starttime;
		if (ss == 1)
			copied_time = spline->stoptime;
	}

	if (cp == 1)
	{
		if (ss == 0)
			spline->starttime = copied_time;
		if (ss == 1)
			spline->stoptime = copied_time;
	}
}

struct event_list *CamSys_Get_Active_Event_List(int type)
{
	if (type == 0 && selected_cam_spline)
		return selected_cam_spline->event_list;

	if (type == 1 && selected_view_spline)
		return selected_view_spline->event_list;

	return NULL;

}


void CamSys_Duplicate_Spline(struct CamSys_Spline *original, struct CamSys_Spline *copy)
{
	int i, point_count;
	float *pos;
	double *time;
	vec3_t	pos_vec;

	if (!copy || !original)
		return;

	copy->enable = original->enable;
	copy->show = original->show;
	copy->starttime = original->starttime;
	copy->stoptime = original->stoptime;

	point_count = Spline_Get_Point_Count(original->spline);
	for (i = 0; i<point_count; i++)
	{
		pos = Spline_Get_Point_Origin_By_Number(original->spline, i+1);
		if (!pos)
			continue;
		VectorSet(pos_vec, pos[0], pos[1], pos[2]);
		Spline_Insert_Point(copy->spline, pos_vec, NULL, 0, NULL);
	}

	for (i = 0; i<point_count; i++)
	{
		time = Spline_Get_Point_Time_By_Number(original->spline, i+1);
		if (!time)
			continue;
		Spline_Set_Point_Time_By_Number(copy->spline, i+1, *time);
	}


}


void CamSys_Duplicate_Selected_Spline(int type)
{
	struct CamSys_Spline *copy, *original;

	if (type == 0)
		original = selected_cam_spline;
	else if (type == 1)
		original = selected_view_spline;
	else
		return;

	if (type == 0)
		copy = CamSys_Add_Spline(1);
	if (type == 1)
		copy = CamSys_Add_Spline(0);

	if (!copy)
		return;
	CamSys_Duplicate_Spline(original, copy);


}

void CamSys_Move_Spline(struct CamSys_Spline *spline, float x, float y, float z)
{
	int point_count, i;
	float *pos;

	if (!spline)
		return;

	point_count = Spline_Get_Point_Count(spline->spline);

	for (i=0;i<point_count;i++)
	{
		pos = Spline_Get_Point_Origin_By_Number(spline->spline, i+1);
		if (!pos)
			continue;
		pos[0] += x;
		pos[1] += y;
		pos[2] += z;

	}


}
