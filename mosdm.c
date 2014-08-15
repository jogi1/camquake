// on screen info,edit menu for the camera curves
// well the new version can do much more ;]
#include "quakedef.h"
#include "mvd_cam.h"


int show_me = 0;
int active = 0 ;
int sub_active = 0;
int sub_track = 0;
int sub_track_origin = 0;
int selected = 0;
int	selected_sub = 0;
int	select_sub = 0;
int	select_sub_sub;
int selected_view = 0;
int selected_cam = 0;
int wich_one = 0;
int show_info = 0;
float modifier = 1;
int draw_snd_cam = 0;
int sub_pe = 0;
int edit_plane = 0;
int sub_pe_plane_select = 0;
char sub_pe_plane[3]= "xyz";
char track_model[128];
int *num;
int draw_compass;
int selected_point = 0;
int *current_point;
int selected_view_point = 0;
int *current_view_point;

double	compass_min;
double	compass_max;
double	compass_restrict;

int *wwp;
double last_track_update;

// Variable for the new menus 
// this way is shit atm but it works
int *view_num;
int *cam_num;
c_curve_t	*current_cam;
c_curve_t	*current_view;
// camera pointer
int *cc_enable;			// enable pointer
int *cc_show;			// show pointer
int *cc_type;			// type pointer
double *cc_startt;			//starttime pointer
double *cc_stopt;			// stoptime pointer
double *cc_fovstart;		//	fov	start	pointer
double *cc_fovstop;			//	fov	stop	pointer
double *cc_dofstart;		//	dof	start	pointer
double *cc_dofstop;			//	dof stop	pointer
double *cc_dsstart;			//	demo_speed	start	pointer
double *cc_dsstop;			//	demo_speed	stop	pointer
// points
vec_t *cc_points[60];
int		cam_spnum;
int		*cam_pnum;
vec3_t	*cc_point;
vec_t	*cc_point_x,*cc_point_y,*cc_point_z;
// events
c_event_t	*cam_current_event;
int		cam_event_snum;
int		*cam_event_num;
double	*cam_event_time;
double	*cam_event_value;
int		*cam_event_type;
char	*cam_event_name;


// view pointer
int *vv_enable;			// enable pointer
int	*vv_show;
int *vv_type;
double *vv_startt;
double *vv_stopt;
double *vv_rstart;
double *vv_rstop;
// points
vec_t *vv_points[60];

int		view_spnum;
int		*view_pnum;
vec3_t	*vv_point;
vec_t	*vv_point_x,*vv_point_y,*vv_point_z;



// Stuff for the compass
double	compass_time;

// entity track stuff
double	etrack_time;
int		etrack_active;

int cam_show_all = 0;
int view_show_all = 0;

int cam_enable_all = 0;
int view_enable_all = 0;

extern c_curve_t cameras[max_cameras];
extern c_curve_t views[max_cameras];
extern int update_subwindow;

typedef struct menu_llist_s {

	struct mvd_menu_s	*menu;
	struct	menu_llist_s	*next;
	int end;
} menu_llist_t;
menu_llist_t	menus;
/*
	type:
	0 - normal menu										-- can be used from anywhere :D
	1 - modifies variable								-- internal use
	2 - toggles variable								-- internal use
	3 - sets cvar_t variable to the name				-- can be used from console
	4 - calls the command with the name as parameter	-- can be used from console
	5 - toggels cvar_t variable							-- can be used from console
	6 - modifies cvar_t variable						-- can be used from console
	7 - vector edit menu ....							-- internal use
	8 - just calls the function							-- internal use
	9 - will display name: charptr						-- internal use
*/




mvd_menu_t	mainmenu;
mvd_menu_t	*activemenu;


char	mvd_nicks[32][32];
int		mvd_nicks_count;

char *camera_info_strings[8] = {{"Camera"},{"Number"},{"Info"},{"Submenu"},{"Modifier"},{"Show"},{"Enable"},{"Close"}};
char *view_info_strings[8] = {{"View"},{"Number"},{"Info"},{"Submenu"},{"Modifier"},{"Show"},{"Enable"},{"Close"}};

cvar_t mosdm_newmenu = {"mosdm_newmenu","1"};

cvar_t mosdm_main_x = {"mosdm_main_x","0"};
cvar_t mosdm_main_y = {"mosdm_main_y","0"};
cvar_t mosdm_sub_x = {"mosdm_sub_x","0"};
cvar_t mosdm_sub_y = {"mosdm_sub_y","0"};
cvar_t mosdm_ma	= {"mosdm_ma","3"};
cvar_t mosdm_md	= {"mosdm_md","4"};

cvar_t mosdm_modifier = {"mosdm_modifier","1.0",0,MOSDM_Onchange_Modifier};

cvar_t	mosdm_new_update_track	=	{"mosdm_new_update_track","0"};
cvar_t	mosdm_new_edit_menu		=	{"mosdm_new_edit_menu","0"};

cvar_t	mosdm_new_update_ptrack=	{"mosdm_new_update_ptrack","1"};

cvar_t	mosdm_new_update_etrack=	{"mosdm_new_update_etrack","1"};

cvar_t	mosdm_new_draw			=	{"mosdm_new_draw","1"};

void Update_Num(void){
	// such an awfull way :<
	
	cam_num = &selected_cam;
	current_cam = &cameras[selected_cam];
	view_num = &selected_view;
	current_view = &views[selected_view];
	current_point = &selected_point;
	current_view_point = &selected_view_point;
	//Com_Printf("Sp %i\n",*current_point);
	//Com_Printf("Sp %i\n",selected_point);
	// camera pointers 
	cc_enable		=	&current_cam->enable;
	cc_show			=	&current_cam->showme;
	cc_startt		=	&current_cam->starttime;
	cc_stopt		=	&current_cam->stoptime;
	cc_fovstart		=	&current_cam->fov_start;		//	fov	start	pointer
	cc_fovstop		=	&current_cam->fov_stop;			//	fov	stop	pointer
	cc_dofstart		=	&current_cam->dof_modifier_start;		//	dof	start	pointer
	cc_dofstop		=	&current_cam->dof_modifier_stop;			//	dof stop	pointer
	cc_dsstart		=	&current_cam->ds_start;			//	demo_speed	start	pointer
	cc_dsstop		=	&current_cam->ds_stop;			//	demo_speed	stop	pointer
	cc_type			=	&current_cam->type;				//	type	pointer
	cc_point		=	&current_cam->ctrlpoints[*current_point];

	//cc_point = &current_cam->ctrlpoints[*current_point];
	cc_point_x = &current_cam->ctrlpoints[*current_point][0];
	cc_point_y = &current_cam->ctrlpoints[*current_point][1];
	cc_point_z = &current_cam->ctrlpoints[*current_point][2];
	//Com_Printf("%i %i %i\n",cc_point_x,cc_point_y,cc_point_z);

	vv_point_x = &current_view->ctrlpoints[*current_view_point][0];
	vv_point_y = &current_view->ctrlpoints[*current_view_point][1];
	vv_point_z = &current_view->ctrlpoints[*current_view_point][2];


	// view pointers
		vv_enable		=	&current_view->enable;
	vv_show			=	&current_view->showme;
	vv_startt		=	&current_view->starttime;
	vv_stopt		=	&current_view->stoptime;
	vv_rstart		=	&current_view->r_start;			//	rotation	start	pointer
	vv_rstop		=	&current_view->r_stop;			//	rotation	stop	pointer
	vv_type			=	&current_view->type;
	vv_point = &current_view->ctrlpoints[*current_view_point];
}

void MOSDM_TEST(void){
	int i,j;
	vec_t *test;
	test =  &current_cam->ctrlpoints[0][0];
	for (i=0,j=0;i<20;i++){/*
		Com_Printf("%i %f %f %f\n",i,current_cam->ctrlpoints[i][0],current_cam->ctrlpoints[i][1],current_cam->ctrlpoints[i][2]);
		Com_Printf("%i %f ",i,*test);
		test++;
		Com_Printf("%f ",*test);
		test++;
		Com_Printf("%f\n",*test);
		test++;*/
		Com_Printf("%i %f %f %f\n",i,*cc_points[j],*cc_points[j+1],*cc_points[j+2]);
		j +=3;
	}


}

void MOSDM_NEW_UP(void){
	if (mosdm_new_edit_menu.value){
		MOSDM_New_Move_Menu(-1);
		return;
	}
	if (activemenu->type == 0 ){
		activemenu->sub_active--;
		if (activemenu->sub_active < 0)
			activemenu->sub_active = activemenu->maxmenu;
		if (activemenu->menu[activemenu->sub_active])
			if (activemenu->menu[activemenu->sub_active]->type == 7){
				activemenu = activemenu->menu[activemenu->sub_active];
				if (activemenu->function){
					activemenu->function();
				}
				activemenu = activemenu->parent;
			}
	}else if (activemenu->type == 1){
		if (activemenu->intptr){
			**activemenu->intptr = **activemenu->intptr +1;
			if (activemenu->restrict)
				if (**activemenu->intptr > activemenu->max)
					**activemenu->intptr = activemenu->min;
		}
		if (activemenu->floatptr){
			**activemenu->floatptr = **activemenu->floatptr + activemenu->variable->value;
		}
		if (activemenu->function)
			activemenu->function();
	}else if (activemenu->type == 2){
		if (activemenu->intptr)
			**activemenu->intptr = !**activemenu->intptr;
		if (activemenu->floatptr)
			**activemenu->floatptr = !**activemenu->floatptr;
	}else if (activemenu->type == 5){
		Cvar_Set(activemenu->variable,va("%i",(int)!activemenu->variable->value));
	}else if (activemenu->type == 6){
		Cvar_Set(activemenu->variable,va("%f",activemenu->variable->value + activemenu->restrict));
	}else if (activemenu->type == 7){
		**activemenu->vecptr = **activemenu->vecptr + activemenu->variable->value;
		if(activemenu->function)
			activemenu->function();
	}

}

void MOSDM_NEW_DOWN(void){
	if (mosdm_new_edit_menu.value){
		MOSDM_New_Move_Menu(1);
		return;
	}
	if (activemenu->type == 0){
		activemenu->sub_active++;
		if (activemenu->sub_active > activemenu->maxmenu)
			activemenu->sub_active = 0;
		if (activemenu->function)
				activemenu->function();
		if (activemenu->menu[activemenu->sub_active])
			if (activemenu->menu[activemenu->sub_active]->type == 7){
				activemenu = activemenu->menu[activemenu->sub_active];
				if (activemenu->function)
					activemenu->function();
				activemenu = activemenu->parent;
			}
	}else if (activemenu->type == 1){
		if (activemenu->intptr){
			**activemenu->intptr = **activemenu->intptr - 1;
			if (activemenu->restrict)
				if (**activemenu->intptr < activemenu->min)
					**activemenu->intptr = activemenu->max;
		}
		if (activemenu->floatptr)
			**activemenu->floatptr = **activemenu->floatptr - activemenu->variable->value;

		if (activemenu->function)
					activemenu->function();
	}else if (activemenu->type == 2){
		if (activemenu->intptr)
			**activemenu->intptr = !**activemenu->intptr;
		if (activemenu->floatptr)
			**activemenu->floatptr = !**activemenu->floatptr;
	}else if (activemenu->type == 5){
		Cvar_Set(activemenu->variable,va("%i",(int)!activemenu->variable->value));
	}else if (activemenu->type == 6){
		Cvar_Set(activemenu->variable,va("%f",activemenu->variable->value - activemenu->restrict));
		Com_Printf("%f\n",activemenu->restrict);
	}else if (activemenu->type == 7){
		**activemenu->vecptr = **activemenu->vecptr - activemenu->variable->value;
		if(activemenu->function)
			activemenu->function();
	}
}

void MOSDM_NEW_RIGHT(void){
	if (mosdm_new_edit_menu.value){
			return;
	}
	if (activemenu->type == 0 && activemenu->menu[activemenu->sub_active]){
		activemenu->active = 1;
		activemenu->menu[activemenu->sub_active]->parent = activemenu;
		activemenu = activemenu->menu[activemenu->sub_active];
		if (activemenu->function){
			activemenu->function();
		}

		if (activemenu->type == 3){
			Cvar_Set(activemenu->variable,activemenu->name);
			activemenu->active = 0;
			activemenu = activemenu->parent;
		}
		if (activemenu->type == 4){
			Cbuf_AddText(va("%s %s\n",activemenu->command,activemenu->name));
			activemenu->active = 0;
			activemenu = activemenu->parent;
		}
		if (activemenu->type == 8){
			activemenu->active = 0;
			activemenu = activemenu->parent;
		}
	}else if (activemenu->type == 2){
		if (activemenu->intptr)
			**activemenu->intptr = !**activemenu->intptr;
		if (activemenu->floatptr)
			**activemenu->floatptr = !**activemenu->floatptr;
	}else if (activemenu->type == 5){
		Cvar_Set(activemenu->variable,va("%i",(int)!activemenu->variable->value));
	}
}

void MOSDM_NEW_LEFT(void){
	if (mosdm_new_edit_menu.value){
			return;
	}
	if (activemenu->type == 1)
		if (activemenu->function)
			activemenu->function();
	activemenu->active = 0;
	activemenu = activemenu->parent;
	if (activemenu->function)
		activemenu->function();
}


void MOSDM_Check_Menus(void){
	menu_llist_t *p;
	p = &menus;
	while (1){
		Com_Printf("%s %s %i\n",p->menu->name,p->menu->identifier,p->menu->type);
		if (p->next){
			p=p->next;
		}else{
			return;
		}
	}
}

void MOSDM_NEW_ADD_MENU(char *pname,char *childname,char *identifier,int type,int **intptr,double **floatptr,char **charptr,float min,float max,double restrict, void *function, cvar_t *variable, char *command, double **modifier, vec_t **vector){
	int i ;
	mvd_menu_t *child;
	mvd_menu_t *parent;
	menu_llist_t *p;
	menu_llist_t *c;
	int quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,pname)){
			parent = p->menu;
			quit = 0;
			break;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("No menu with the identifier %s\n",pname);
				return;
			}
		}
	}
	quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,identifier)){
			Com_Printf("A menu with the identifier %s already exists !\n",identifier);
			return;
		}else{
			if (p->next){
				p=p->next;
			}else{
				quit = 0;
			}
		}
	}
	child = Q_malloc(sizeof (mvd_menu_t));
	memset(child,0,sizeof(mvd_menu_t));
	for (i=0,quit = 1;i<max_submenus && quit;i++){
		if (parent->menu[i] == NULL){
			parent->menu[i] = child;
			if (intptr)
				child->intptr = intptr;
			if (floatptr)
				child->floatptr = floatptr;
			if (charptr)
				child->charptr = charptr;
			strcpy(child->name,childname);
			strcpy(child->identifier,identifier);
			if (command)
				strcpy(child->command,command);
			child->type = type;
			child->parent = parent;
			child->function = function;
			child->restrict = restrict;
			child->min	=	min;
			child->max	=	max;
			child->variable = variable;
			child->vecptr = vector;
			if (modifier)
				child->modifier = modifier;
			parent->maxmenu = i;
			child->maxmenu = 0;
			quit = 0;
		}
		if (parent->width < strlen(parent->menu[i]->name))
			parent->width = strlen(parent->menu[i]->name);
	}
	c = Q_malloc(sizeof (menu_llist_t));
	memset(c,0,sizeof(menu_llist_t));
	c->menu = child;
	while (p->next)
		p=p->next;
	p->next = c;


}

/*
	help function for the camera menus
*/
// camera functions
void Cam_SetStart(void){
	cameras[selected_cam].starttime = cls.demotime - demostarttime;
}
void Cam_SetStop(void){
	cameras[selected_cam].stoptime = cls.demotime - demostarttime;
}
void Cam_SetP(void){
	VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,cameras[selected_cam].ctrlpoints[(int) activemenu->restrict]);
}
void Cam_DelP(void){
	cameras[selected_cam].ctrlpoints[(int) activemenu->restrict -1][0] = 0;
	cameras[selected_cam].ctrlpoints[(int) activemenu->restrict -1][1] = 0;
	cameras[selected_cam].ctrlpoints[(int) activemenu->restrict -1][2] = 0;
}

void Cam_MIS(void){
	VectorJogi(cameras[selected_cam].ctrlpoints[3], cameras[selected_cam].ctrlpoints[0], cameras[selected_cam].ctrlpoints[1],1/3);
	VectorJogi(cameras[selected_cam].ctrlpoints[0], cameras[selected_cam].ctrlpoints[3], cameras[selected_cam].ctrlpoints[2],1/3);
}

/*
	SES	-	set end of current to start of next
	SSE	-	set start of current to end of previous
*/

void Cam_SES(void){
	c_curve_t *current;
	c_curve_t *next;
	int lastpoint_c,lastpoint_n,i;

	if (selected_cam + 1 > max_cameras)
		return;

	current = &cameras[selected_cam];
	next = &cameras[selected_cam + 1];
	for (i=0;i<20;i++){
		if ((current->ctrlpoints[i][0] + current->ctrlpoints[i][1] + current->ctrlpoints[i][2]) == 0)
			break;
		lastpoint_c = i;
	}
	for (i=0;i<20;i++){
		if ((next->ctrlpoints[i][0] + next->ctrlpoints[i][1] + next->ctrlpoints[i][2]) == 0)
			break;
		lastpoint_n = i;
	}
	// cubic - cubic
	if (!current->type && !next->type)
		VectorCopy(current->ctrlpoints[lastpoint_c],next->ctrlpoints[0]);
	// catmull - cubic
	if (current->type && !next->type){
		VectorCopy(current->ctrlpoints[lastpoint_c-1],next->ctrlpoints[0]);
	}
	// cubic - catmull
	if (!current->type && next->type){
		VectorCopy(current->ctrlpoints[lastpoint_c],next->ctrlpoints[0]);
		VectorCopy(current->ctrlpoints[lastpoint_c],next->ctrlpoints[1]);
		next->ctrlpoints[0][0] -= 10;
		next->ctrlpoints[0][1] -= 10;
		next->ctrlpoints[0][2] -= 10;
	}
	// catmull - catmull
	if (current->type && next->type){
		VectorCopy(current->ctrlpoints[lastpoint_c-2],next->ctrlpoints[0]);
		VectorCopy(current->ctrlpoints[lastpoint_c-1],next->ctrlpoints[1]);
		VectorCopy(current->ctrlpoints[lastpoint_c],next->ctrlpoints[2]);

	}
	next->starttime				=	current->stoptime;				// time
	next->fov_start				=	current->fov_stop;				// fov
	next->dof_modifier_start	=	current->dof_modifier_stop;		// dof
	next->ds_start				=	current->ds_stop;				// demospeed


}
void Cam_SSE(void){
	c_curve_t *current;
	c_curve_t *previous;
	if (selected_cam - 1 < 0)
		return;
	current = &cameras[selected_cam];
	previous = &cameras[selected_cam - 1];

	VectorCopy(previous->ctrlpoints[3],current->ctrlpoints[0]);
	current->starttime			= previous->stoptime;				// time
	current->fov_start			= previous->fov_stop;				// fov
	current->ds_start			= previous->ds_stop;				// demospeed
	current->dof_modifier_start	= previous->dof_modifier_stop;		// dof
}

// view functions
void View_SetStart(void){
	views[selected_view].starttime = cls.demotime - demostarttime;
}
void View_SetStop(void){
	views[selected_view].stoptime = cls.demotime - demostarttime;
}
void View_SetP(void){
	VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,views[selected_view].ctrlpoints[(int) activemenu->restrict]);
}
void View_DelP(void){
	views[selected_view].ctrlpoints[(int) activemenu->restrict -1][0] = 0;
	views[selected_view].ctrlpoints[(int) activemenu->restrict -1][1] = 0;
	views[selected_view].ctrlpoints[(int) activemenu->restrict -1][0] = 0;
}
void View_MIS(void){
	VectorJogi(views[selected_view].ctrlpoints[3], views[selected_view].ctrlpoints[0], views[selected_view].ctrlpoints[1],1/3);
	VectorJogi(views[selected_view].ctrlpoints[0], views[selected_view].ctrlpoints[3], views[selected_view].ctrlpoints[2],1/3);
}
void View_SES(void){
	c_curve_t *current;
	c_curve_t *next;
	int i,lastpoint_c,lastpoint_n;
	if (selected_view + 1 > max_cameras)
		return;
	current = &views[selected_view];
	next = &views[selected_view + 1];
	for (i=0;i<20;i++){
		if ((current->ctrlpoints[i][0] + current->ctrlpoints[i][1] + current->ctrlpoints[i][2]) == 0)
			break;
		lastpoint_c = i;
	}
	for (i=0;i<20;i++){
		if ((next->ctrlpoints[i][0] + next->ctrlpoints[i][1] + next->ctrlpoints[i][2]) == 0)
			break;
		lastpoint_n = i;
	}
	// cubic - cubic
	if (!current->type && !next->type)
		VectorCopy(current->ctrlpoints[lastpoint_c],next->ctrlpoints[0]);
	// catmull - cubic
	if (current->type && !next->type){
		VectorCopy(current->ctrlpoints[lastpoint_c-1],next->ctrlpoints[0]);
	}
	// cubic - catmull
	if (!current->type && next->type){
		VectorCopy(current->ctrlpoints[lastpoint_c],next->ctrlpoints[0]);
		VectorCopy(current->ctrlpoints[lastpoint_c],next->ctrlpoints[1]);
		next->ctrlpoints[0][0] -= 10;
		next->ctrlpoints[0][1] -= 10;
		next->ctrlpoints[0][2] -= 10;
	}
	// catmull - catmull
	if (current->type && next->type){
		VectorCopy(current->ctrlpoints[lastpoint_c-2],next->ctrlpoints[0]);
		VectorCopy(current->ctrlpoints[lastpoint_c-1],next->ctrlpoints[1]);
		VectorCopy(current->ctrlpoints[lastpoint_c],next->ctrlpoints[2]);

	}
	next->starttime				=	current->stoptime;				// time
	next->fov_start				=	current->fov_stop;				// fov
	next->dof_modifier_start	=	current->dof_modifier_stop;		// dof
	next->ds_start				=	current->ds_stop;				// demospeed


}
void View_SSE(void){
	c_curve_t *current;
	c_curve_t *previous;
	if (selected_view - 1 < 0)
		return;
	current = &views[selected_view];
	previous = &views[selected_view - 1];
	VectorCopy(previous->ctrlpoints[3],current->ctrlpoints[0]);
	current->starttime			= previous->stoptime;				// time
	current->fov_start			= previous->fov_stop;				// fov
	current->ds_start			= previous->ds_stop;				// demospeed
	current->dof_modifier_start	= previous->dof_modifier_stop;		// dof

}

void Set_Activepoint(void){
	c_curve_t *current;
	if (activemenu->min == 1)
		current = &cameras[selected_cam];
	else if (activemenu->min == 2)
		current = &views[selected_view];
	else
		return;
	
	compass_min = activemenu->min;
	compass_max = activemenu->max;
	compass_restrict = activemenu->restrict;

	compass_time = Sys_DoubleTime();
}
void Update_Events(void);
void MOSDM_NEW_INIT(void){
	if (strlen(mainmenu.name))
		return;
	mainmenu.parent = &mainmenu;
	strcpy(mainmenu.name,"Main Menu");
	strcpy(mainmenu.identifier,"Main Menu");
	menus.menu = &mainmenu;

	cam_num = &selected_cam;
	current_cam = &cameras[selected_cam];
	cc_point = &current_cam->ctrlpoints[0];
	cc_point_x = &current_cam->ctrlpoints[0][0];
	cc_point_y = &current_cam->ctrlpoints[0][1];
	cc_point_z = &current_cam->ctrlpoints[0][2];
	cam_pnum = &cam_spnum;
	cam_event_num = &cam_event_snum;

	
	view_num = &selected_view;
	current_view = &views[selected_view];
	vv_point = &current_view->ctrlpoints[0];
	vv_point_x = &current_view->ctrlpoints[0][0];
	vv_point_y = &current_view->ctrlpoints[0][1];
	vv_point_z = &current_view->ctrlpoints[0][2];
	view_pnum = &view_spnum;
	Update_Num();
	Update_Events();

	activemenu = &mainmenu;
	Com_Printf("I got called!\n");
}
/*
	Point Stuff
*/

void Update_Point_View(void){
	if (!activemenu->intptr)
		return;

	vv_point = &current_view->ctrlpoints[**activemenu->intptr];
	vv_point_x = &current_view->ctrlpoints[**activemenu->intptr][0];
	vv_point_y = &current_view->ctrlpoints[**activemenu->intptr][1];
	vv_point_z = &current_view->ctrlpoints[**activemenu->intptr][2];
}

void View_Set_Point(void){
	VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,*vv_point);
}

void View_Del_Point(void){
	*vv_point[0] = 0;
	*vv_point[1] = 0;
	*vv_point[2] = 0;
}

void Update_Point_Cam(void){
	if (!activemenu->intptr)
		return;
	cc_point = &current_cam->ctrlpoints[**activemenu->intptr];
	cc_point_x = &current_cam->ctrlpoints[**activemenu->intptr][0];
	cc_point_y = &current_cam->ctrlpoints[**activemenu->intptr][1];
	cc_point_z = &current_cam->ctrlpoints[**activemenu->intptr][2];
}

void Cam_Set_Point(void){
	VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,*cc_point);
}

void Cam_Del_Point(void){
	*cc_point[0] = 0;
	*cc_point[1] = 0;
	*cc_point[2] = 0;
}
/*
		Event stuff
*/
void Update_Events(void){
	int i;
	c_event_t *current_event;
	if (current_cam->events){
		if (*cam_event_num > current_cam->event_count)
			*cam_event_num = 0;
		if (*cam_event_num < 0)
			*cam_event_num = current_cam->event_count;
		current_event = current_cam->events;
		i=0;
		while (i<*cam_event_num && current_event->next){
			current_event = current_event->next;
			i++;
		}
		cam_current_event = current_event;
		cam_event_time = &current_event->time;
		cam_event_value = &current_event->value;
		cam_event_type = &current_event->salone;
		cam_event_name = &current_event->name[0];
	}
}

void Cam_Event_Del(void){
	/*
	it i;
	c_event_t *current_event,*previous_event;

	current_event = current_cam->events;
	i=0;
	
	if (current_event->next)
		while (i<=*cam_event_num && current_event->next){
			previous_event = current_event;
			current_event = current_event->next;
		}
	
	if (cam_current_event->next && cam_current_event->previous){
		cam_current_event->next->previous = cam_current_event->previous;
		cam_current_event->previous->next = cam_current_event->next;
	}else if (!cam_current_event->next && !cam_current_event->previous){
		cam_current_event->salone = 0;
		cam_current_event->time = 0;
		cam_current_event->value = 0;
		cam_current_event->name[0] = NULL;
		return;
		//current_cam->events = NULL;
	}else if (!cam_current_event->next && cam_current_event->previous){
		cam_current_event->previous->next = NULL;
	}else if (cam_current_event->next && !cam_current_event->previous){
		cam_current_event->next->previous = NULL;
		current_cam->events = cam_current_event->next;
	}
	/*}else if (cam_current_event == current_cam->events && cam_current_event->next){
		current_cam->events = cam_current_event->next;
		cam_current_event->next->previous = NULL;
	}*/
	
	//current_cam->event_count -= 1;
	//Com_Printf("cams_event %i del %i\n",selected_cam,cam_event_snum +1);
	Cbuf_AddText(va("cams_event %i del %i\n",selected_cam,cam_event_snum +1));
	cam_event_num -= 1;
	

	//Q_free(current_event);
	
	Update_Num();
	Update_Events();
}

// 0 = up
// 1 = down
void Cam_Event_Sort(int direction){
	c_event_t	*current_event;

	// removing the current event from the list
	cam_current_event->next->previous = cam_current_event->previous;
	cam_current_event->previous->next = cam_current_event->next;

	//down
	if (cam_current_event->previous->time > cam_current_event->time){
		//Com_Printf("We sort down\n");
		current_event = cam_current_event->previous;
		while (current_event->previous){
			if (current_event->previous->time < cam_current_event->time)
				break;
			current_event = current_event->previous;
		}
		// if we reached the start of the list
		if (current_event == current_cam->events){
			current_cam->events = cam_current_event;
			cam_current_event->next = current_event;
			current_event->previous = cam_current_event;
			cam_current_event->previous = NULL;
		}else{
			cam_current_event->next = current_event;
			cam_current_event->previous = current_event->previous;
			current_event->previous->next = current_event;
			current_event->next->previous = current_event;

		}
		

	}else if (cam_current_event->next->time < cam_current_event->time){ // up
		//Com_Printf("We sort up\n");
		current_event = cam_current_event->next;
		while (current_event->next){
			if (current_event->next->time < cam_current_event->time)
				break;
			current_event = current_event->next;
		}
		// if we reached the en of the list
		if (current_event->next == NULL){
			cam_current_event->next = NULL;
			cam_current_event->previous = current_event;
			current_event->next = cam_current_event;
		}else{
			cam_current_event->next = current_event->next;
			cam_current_event->previous = current_event;
			cam_current_event->previous->next = cam_current_event;
			cam_current_event->next->previous = cam_current_event;
		}
		

	}
}

void Cam_Event_Checktime(void){
	if (*cam_event_time < current_cam->starttime)
		*cam_event_time = current_cam->starttime;
	if (*cam_event_time > current_cam->stoptime)
		*cam_event_time = current_cam->stoptime;

	if (!cam_current_event->previous && !cam_current_event->next){		// only object
		return;
	}else if (!cam_current_event->previous && cam_current_event->next){
		if (cam_current_event->time <= cam_current_event->next->time);
			return;
			Cam_Event_Sort(1);
	}else if (cam_current_event->previous  && !cam_current_event->next){
		if (cam_current_event->time >= cam_current_event->previous->time);
			return;
			Cam_Event_Sort(0);
	}else if (cam_current_event->previous && cam_current_event->next){
		if (cam_current_event->previous->time > cam_current_event->time)
			Cam_Event_Sort(0);
		if (cam_current_event->next->time < cam_current_event->time)
			Cam_Event_Sort(1);
		if (cam_current_event->previous->time <= cam_current_event->time && cam_current_event->next->time >= cam_current_event->time)
			return;

	}
}


void Cams_Event_Add(void){
	switch(Cmd_Argc()){
		case 2:		Cbuf_AddText(va("cams_event %i add %f %s 0 0\n",*cam_num,current_cam->starttime,Cmd_Argv(1)));
					break;
		default:	Com_Printf("%s eventname\n",Cmd_Argv(0));
					break;
	}

}



void MOSDM_NEW_Console_Static(void){
	char	*name, *parent;
	if (Cmd_Argc() < 3){
		Com_Printf("We need the identifier of the menu and the parent to attach it too\n");
		return;
	}
	name = Cmd_Argv(1);
	parent = Cmd_Argv(2);
	Com_Printf("%s %s\n",name,parent);

	// add the cam menu
	if (!strcmp("Cam Menu",name)){
			MOSDM_NEW_ADD_MENU(parent,"Cam Menu","Cam Menu",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
			return;
	}
	
	if (!strcmp("Cams",name)){
	// cameras submenu
		MOSDM_NEW_ADD_MENU(parent,"Cams","Cams",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		return;
	}

	if (!strcmp("cnumber",name)){
		MOSDM_NEW_ADD_MENU(parent,"Number","cnumber",1,&cam_num,NULL,NULL,0,39,1,Update_Num,NULL,NULL,NULL,NULL);
		return;
	}

	if (!strcmp("cenable",name)){
	// enable
		MOSDM_NEW_ADD_MENU(parent,"enable","cenable",2,&cc_enable,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("cenable","enabled off","cenableon",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("cenable","enabled on","cenableoff",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
	}
	if (!strcmp("cshow",name)){
	// show
		MOSDM_NEW_ADD_MENU(parent,"show","cshow",2,&cc_show,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("cshow","show off","cshowon",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("cshow","show on","cshowoff",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		return;
	}
	if (!strcmp("ctype",name)){
	// type
		MOSDM_NEW_ADD_MENU(parent,"type","ctype",2,&cc_type,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("ctype","cubic spline","ctypeon",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("ctype","catmull-rom spline","ctypeoff",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
	}
	if (!strcmp("cstartts",name)){
		// starttime set
		MOSDM_NEW_ADD_MENU(parent,"set","cstartts",3,NULL,NULL,NULL,0,0,0,Cam_SetStart,NULL,NULL,NULL,NULL);
		return;
	}
	if (!strcmp("cstarttm",name)){
		// starttime modify
		MOSDM_NEW_ADD_MENU(parent,"modify","cstarttm",1,NULL,&cc_startt,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}

	if (!strcmp("cstopts",name)){
		// stoptime set
		MOSDM_NEW_ADD_MENU(parent,"set","cstopts",3,NULL,NULL,NULL,0,0,0,Cam_SetStop,NULL,NULL,NULL,NULL);
		return;
	}

	if (!strcmp("cstoptm",name)){
		// stoptime modify
			MOSDM_NEW_ADD_MENU(parent,"modify","cstoptm",1,NULL,&cc_stopt,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	
	if (!strcmp("cstoptm",name)){
		// stoptime modify
			MOSDM_NEW_ADD_MENU(parent,"modify","cstoptm",1,NULL,&cc_stopt,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	
	//	demospeed
	//	start
	if (!strcmp("cdstarttm",name)){
			MOSDM_NEW_ADD_MENU(parent,"start","cdstarttm",1,NULL,&cc_dsstart,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	//	stop
	if (!strcmp("cdstoptm",name)){
			MOSDM_NEW_ADD_MENU(parent,"stop","cdstoptm",1,NULL,&cc_dsstop,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}


	//	fov
	//	start
	if (!strcmp("cfovstart",name)){
		MOSDM_NEW_ADD_MENU(parent,"start","cfovstart",1,NULL,&cc_fovstart,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	//	stop
	if (!strcmp("cfovstart",name)){
		MOSDM_NEW_ADD_MENU(parent,"stop","cfovstop",1,NULL,&cc_fovstop,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	// dof
	//	start
	if (!strcmp("cdofstart",name)){
		MOSDM_NEW_ADD_MENU(parent,"start","cdofstart",1,NULL,&cc_dofstart,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	//	stop
	if (!strcmp("cdofstop",name)){
		MOSDM_NEW_ADD_MENU(parent,"stop","cdofstop",1,NULL,&cc_dofstop,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	
	
	// points
	// new points system
	// selecting the point
	if (!strcmp("cpointnum",name)){
		MOSDM_NEW_ADD_MENU(parent,"number","cpnumber",1,&cam_pnum,NULL,NULL,0,19,1,Update_Point_Cam,NULL,NULL,NULL,NULL);
		return;
	}
	// set
	if (!strcmp("cpointset",name)){
		MOSDM_NEW_ADD_MENU(parent,"set","cpset",3,NULL,NULL,NULL,0,0,0,Cam_Set_Point,NULL,NULL,NULL,NULL);
		return;
	}
	// edit
	if (!strcmp("cpointedit",name)){
		MOSDM_NEW_ADD_MENU(parent,"edit","cpointedit",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("cpointedit","x","cpointeditx",7,NULL,NULL,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,&cc_point_x);
		MOSDM_NEW_ADD_MENU("cpointedit","y","cpointedity",7,NULL,NULL,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,&cc_point_y);
		MOSDM_NEW_ADD_MENU("cpointedit","z","cpointeditz",7,NULL,NULL,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,&cc_point_z);
		return;
	}
	// delete
	if (!strcmp("cpointdel",name)){
		MOSDM_NEW_ADD_MENU(parent,"del","cpointdel",3,NULL,NULL,NULL,0,0,0,Cam_Del_Point,NULL,NULL,NULL,NULL);
		return;
	}

	//event system
	// selecting the event
	if (!strcmp("ce_number",name)){
		MOSDM_NEW_ADD_MENU(parent,"Number","cenumber",1,&cam_event_num,NULL,NULL,0,0,0,Update_Events,NULL,NULL,NULL,NULL);
		return;
	}

	// showing the name
	if (!strcmp("ce_name",name)){
		MOSDM_NEW_ADD_MENU(parent,"Name","cename",9,NULL,NULL,&cam_event_name,0,0,0,NULL,NULL,NULL,NULL,NULL);
		return;
	}

	// add -- done via a command

	// delete
	if (!strcmp("ce_del",name)){
		MOSDM_NEW_ADD_MENU(parent,"del","ce_del",3,NULL,NULL,NULL,0,0,0,Cam_Event_Del,NULL,NULL,NULL,NULL);
		return;
	}

	// edit stuff
	// time
	if (!strcmp("ce_time",name)){
		MOSDM_NEW_ADD_MENU(parent,"time","ce_time",1,NULL,&cam_event_time,NULL,0,0,0,Cam_Event_Checktime,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	// value
	if (!strcmp("ce_value",name)){
		MOSDM_NEW_ADD_MENU(parent,"value","ce_value",1,NULL,&cam_event_value,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	// type
	if (!strcmp("ce_type",name)){
		MOSDM_NEW_ADD_MENU(parent,"type","ce_type",2,&cam_event_type,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("ce_type","multievent","ce_e_saoff",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("ce_type","standalone","ce_e_saon",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
	}


	// other stuff

	// make it straight
	if (!strcmp("cmis",name)){
		MOSDM_NEW_ADD_MENU(parent,"make it straight","cmis",3,NULL,NULL,NULL,0,0,0,Cam_MIS,NULL,NULL,NULL,NULL);
		return;
	}
	// set start to end of previous
	if (!strcmp("csse",name)){
		MOSDM_NEW_ADD_MENU(parent,"set start to end of previous","csse",3,NULL,NULL,NULL,0,0,0,Cam_SSE,NULL,NULL,NULL,NULL);
		return;
	}
	// set end to start of next
	if (!strcmp("cses",name)){
		MOSDM_NEW_ADD_MENU(parent,"set start of next to end of current","cses",3,NULL,NULL,NULL,0,0,0,Cam_SES,NULL,NULL,NULL,NULL);
		return;
	}

	// ********************************************************************
	// views submenu
	// ********************************************************************
	if (!strcmp("vnumber",name)){
		MOSDM_NEW_ADD_MENU(parent,"Number","vnumber",1,&view_num,NULL,NULL,0,39,1,Update_Num,NULL,NULL,NULL,NULL);
		return;
	}
	// enable
	if (!strcmp("venable",name)){
		MOSDM_NEW_ADD_MENU(parent,"enable","venable",2,&vv_enable,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("venable","enabled off","venableon",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("venable","enabled on","venableoff",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		return;
	}
	// show
	if (!strcmp("vshow",name)){
		MOSDM_NEW_ADD_MENU(parent,"show","vshow",2,&vv_show,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("vshow","show off","vshowon",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("vshow","show on","vshowoff",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		return;
	}
	if (!strcmp("vtype",name)){
	// type
		MOSDM_NEW_ADD_MENU(parent,"type","vtype",2,&vv_type,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("vtype","cubic spline","vtypeon",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("vtype","catmull-rom spline","vtypeoff",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
	}
	//	time
	
	// starttime
	// set
	if (!strcmp("vstartts",name)){
		MOSDM_NEW_ADD_MENU(parent,"set","vstartts",3,NULL,NULL,NULL,0,0,0,View_SetStart,NULL,NULL,NULL,NULL);
		return;
	}
	// modify
	if (!strcmp("vstarttm",name)){
		MOSDM_NEW_ADD_MENU(parent,"modify","vstarttm",1,NULL,&vv_startt,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	
	// stoptime
	// set
		if (!strcmp("vstopts",name)){
		MOSDM_NEW_ADD_MENU(parent,"set","vstopts",3,NULL,NULL,NULL,0,0,0,View_SetStop,NULL,NULL,NULL,NULL);
		return;
	}
	// modify
	if (!strcmp("vstoptm",name)){
		MOSDM_NEW_ADD_MENU(parent,"modify","vstoptm",1,NULL,&vv_stopt,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	
	

	//	rotation
	//	start
	if (!strcmp("vrstart",name)){
		MOSDM_NEW_ADD_MENU(parent,"start","vrstart",1,NULL,&vv_rstart,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}
	//	stop
	if (!strcmp("vrstop",name)){
		MOSDM_NEW_ADD_MENU(parent,"stop","vrstop",1,NULL,&vv_rstop,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,NULL);
		return;
	}

	// points
	// new points system
	// selecting the point
	if (!strcmp("vpointnum",name)){
		MOSDM_NEW_ADD_MENU(parent,"number","vpnumber",1,&view_pnum,NULL,NULL,0,19,1,Update_Point_View,NULL,NULL,NULL,NULL);
		return;
	}
	// set
	if (!strcmp("vpointset",name)){
		MOSDM_NEW_ADD_MENU(parent,"set","vpset",3,NULL,NULL,NULL,0,0,0,View_Set_Point,NULL,NULL,NULL,NULL);
		return;
	}
	// edit
	if (!strcmp("vpointedit",name)){
		MOSDM_NEW_ADD_MENU(parent,"edit","vpointedit",0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
		MOSDM_NEW_ADD_MENU("vpointedit","x","vpointeditx",7,NULL,NULL,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,&vv_point_x);
		MOSDM_NEW_ADD_MENU("vpointedit","y","vpointedity",7,NULL,NULL,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,&vv_point_y);
		MOSDM_NEW_ADD_MENU("vpointedit","z","vpointeditz",7,NULL,NULL,NULL,0,0,0,NULL,&mosdm_modifier,NULL,NULL,&vv_point_z);
		return;
	}
	// delete
	if (!strcmp("vpointdel",name)){
		MOSDM_NEW_ADD_MENU(parent,"del","vpointdel",3,NULL,NULL,NULL,0,0,0,View_Del_Point,NULL,NULL,NULL,NULL);
		return;
	}
	// make it straight
	if (!strcmp("vmis",name)){
		MOSDM_NEW_ADD_MENU(parent,"make it straight","vmis",3,NULL,NULL,NULL,0,0,0,View_MIS,NULL,NULL,NULL,NULL);
		return;
	}
	// set start to end of previous
	if (!strcmp("vsse",name)){
		MOSDM_NEW_ADD_MENU(parent,"set start to end of previous","vsse",3,NULL,NULL,NULL,0,0,0,View_SSE,NULL,NULL,NULL,NULL);
		return;
	}
	// set end to start of next
	if (!strcmp("vses",name)){
		MOSDM_NEW_ADD_MENU(parent,"set start of next to end of current","vses",3,NULL,NULL,NULL,0,0,0,View_SES,NULL,NULL,NULL,NULL);
		return;
	}
	//}

	Com_Printf("Identifier not found\n");
	return;
}

/*
	Adds a normal submenu to the current menu
	usage:	name identifier parentmenu
*/
void MOSDM_Console_Add_Menu(void){
	menu_llist_t *p;
	int quit = 1;
	Com_Printf("%s %s %s %s\n",Cmd_Argv(0),Cmd_Argv(1),Cmd_Argv(2),Cmd_Argv(3));

	p = &menus;

	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(2))){
			Com_Printf("A menu with the name %s already exists\n",Cmd_Argv(2));
			return;
		}else{
			if (p->next){
				p=p->next;
			}else{
				quit = 0;
			}
		}
	}
	quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(3))){
			quit = 0;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("A parentmenu with the name %s does not exit\n",Cmd_Argv(3));
				return;
			}
		}
	}
	MOSDM_NEW_ADD_MENU(Cmd_Argv(3),Cmd_Argv(1),Cmd_Argv(2),0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);

}

void MOSDM_Console_Add_select_Menu(void){
	cvar_t *variable = 0;
	menu_llist_t *p;
	qbool iscmd;
	int quit = 1;
	Com_Printf("%s %s %s %s\n",Cmd_Argv(0),Cmd_Argv(1),Cmd_Argv(2),Cmd_Argv(3));
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(2))){
			Com_Printf("A menu with the name %s already exists\n",Cmd_Argv(2));
			return;
		}else{
			if (p->next){
				p=p->next;
			}else{
				quit = 0;
			}
		}
	}
	quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(3))){
			quit = 0;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("A parentmenu with the name %s does not exit\n",Cmd_Argv(3));
				return;
			}
		}
	}
	variable = Cvar_Find(Cmd_Argv(4));
	iscmd = Cmd_Exists(Cmd_Argv(4));
	if (!variable && !iscmd){
		Com_Printf("Neither a variable nor a command with the name %s does extis\n",Cmd_Argv(4));
		return;
	}

	if (variable)
		MOSDM_NEW_ADD_MENU(Cmd_Argv(3),Cmd_Argv(1),Cmd_Argv(2),3,NULL,NULL,NULL,0,0,0,NULL,variable,NULL,NULL,NULL);
	if (iscmd){
		MOSDM_NEW_ADD_MENU(Cmd_Argv(3),Cmd_Argv(1),Cmd_Argv(2),4,NULL,NULL,NULL,0,0,0,NULL,NULL,Cmd_Argv(4),NULL,NULL);
	}
	
}

void MOSDM_Console_Add_toggle_Menu(void){
	char arg[32];
	cvar_t *variable = 0;
	menu_llist_t *p;
	int quit = 1;
	Com_Printf("%s %s %s %s\n",Cmd_Argv(0),Cmd_Argv(1),Cmd_Argv(2),Cmd_Argv(3));
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(2))){
			Com_Printf("A menu with the name %s already exists\n",Cmd_Argv(2));
			return;
		}else{
			if (p->next){
				p=p->next;
			}else{
				quit = 0;
			}
		}
	}
	quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(3))){
			quit = 0;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("A parentmenu with the name %s does not exit\n",Cmd_Argv(3));
				return;
			}
		}
	}
	variable = Cvar_Find(Cmd_Argv(4));
	if (!variable){
		Com_Printf("No variable the name %s does extis\n",Cmd_Argv(4));
		return;
	}

	MOSDM_NEW_ADD_MENU(Cmd_Argv(3),Cmd_Argv(1),Cmd_Argv(2),5,NULL,NULL,NULL,0,0,0,NULL,variable,NULL,NULL,NULL);
	sprintf(arg,"%s off",Cmd_Argv(2));
	MOSDM_NEW_ADD_MENU(Cmd_Argv(2),arg,arg,0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);
	sprintf(arg,"%s on",Cmd_Argv(2));
	MOSDM_NEW_ADD_MENU(Cmd_Argv(2),arg,arg,0,NULL,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL);

}


/*

Function for adding a modifier menu from console
will automaticaly use the variable mosdm_modifier as value
usage: 
		mosdm_new_add_modify_menu	name	identifier	parent	variable	modifierfloat(ie 0.01)


*/
void MOSDM_Console_Add_modify_Menu(void){
	cvar_t *variable = 0;
	menu_llist_t *p;
	int quit = 1;
	Com_Printf("%s %s %s %s %s %s\n",Cmd_Argv(0),Cmd_Argv(1),Cmd_Argv(2),Cmd_Argv(3),Cmd_Argv(4),Cmd_Argv(5));
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(2))){
			Com_Printf("A menu with the name %s already exists\n",Cmd_Argv(2));
			return;
		}else{
			if (p->next){
				p=p->next;
			}else{
				quit = 0;
			}
		}
	}
	quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(3))){
			quit = 0;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("A parentmenu with the name %s does not exit\n",Cmd_Argv(3));
				return;
			}
		}
	}
	variable = Cvar_Find(Cmd_Argv(4));
	if (!variable){
		Com_Printf("No variable the name %s does extis\n",Cmd_Argv(4));
		return;
	}

	if (!strlen(Cmd_Argv(5))){
		Com_Printf("You did not supply a modifier\n");
		return;
	}

	MOSDM_NEW_ADD_MENU(Cmd_Argv(3),Cmd_Argv(1),Cmd_Argv(2),6,NULL,NULL,NULL,0,0,atof(Cmd_Argv(5)),NULL,variable,NULL,NULL,NULL);
}
/*
	Internal function for movung a menu up/down
*/

void MOSDM_New_Move_Menu(int dir){
	mvd_menu_t	*temp;
	int newm;
	newm = activemenu->sub_active + dir;
	if (newm < 0)
		newm = activemenu->maxmenu;
	if (newm > activemenu->maxmenu)
		newm = 0;

	temp = activemenu->menu[activemenu->sub_active];
	activemenu->menu[activemenu->sub_active] = activemenu->menu[newm];
	activemenu->menu[newm] = temp;
	activemenu->sub_active = newm;

}

/* 
	Internal function for removing a menu
*/

void MOSDM_New_Delete_Menu(char *menu){
	int i,z;
	menu_llist_t *p;
	menu_llist_t *previous;
	mvd_menu_t	*parent;
	int quit = 1;
	if (!strcmp(menu,"Main Menu")){
		Com_Printf("Fuck you for trying that :>\n");
		return;
	}
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,menu)){
			quit = 0;
		}else{
			if (p->next){
				previous = p;
				p=p->next;
			}else{
				Com_Printf("No menu with the identifier %s found\n",menu);
				return;
			}
		}
	}
	p->menu->active = -1;
	if (p->menu->parent->active != -1 && activemenu == p->menu){
		activemenu = p->menu->parent;
	}
	
	// deletes all submenus
	for (i=0;i<max_submenus;i++)
		if (p->menu->menu[i])
			MOSDM_New_Delete_Menu(p->menu->menu[i]->identifier);

	parent = p->menu->parent;

	// remove the entry from the parent
	for (i=0;i<max_submenus;i++){
		if (parent->menu[i] && !strcmp(parent->menu[i]->name,p->menu->name))
			break;
	}
	
	for(z=i+1;z<max_submenus;z++)
		if (parent->menu[z]){
			parent->menu[i++] = parent->menu[z];
			parent->menu[z] = NULL;
		}else{
			parent->menu[i] = NULL;
		}
	for (i=0;i<max_submenus;i++)
		if (parent->menu[i])
			parent->maxmenu = i;
	

	previous->next = p->next;
	Q_free(p->menu);
	Q_free(p);

	
}

/*
	Function for linking an existing menu to multible other menus
*/

void MOSDM_NEW_Console_Link(void){
	menu_llist_t *p;
	mvd_menu_t	*parent,*child;
	int i;
	int quit = 1;

	Com_Printf("Not working atm\n");
	return;
	Com_Printf("%s %s %s %s %s %s\n",Cmd_Argv(0),Cmd_Argv(1),Cmd_Argv(2),Cmd_Argv(3),Cmd_Argv(4),Cmd_Argv(5));
	p = &menus;
	if (Cmd_Argc() < 3){
		Com_Printf("%s child parent\n",Cmd_Argv(1));
		return;
	}

	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(1))){
			child = p->menu;
			quit = 0;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("A menu with the name %s does not exists\n",Cmd_Argv(1));
				return;
				
			}
		}
	}
	quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,Cmd_Argv(2))){
			parent = p->menu;
			quit = 0;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("A parentmenu with the name %s does not exit\n",Cmd_Argv(2));
				return;
			}
		}
	}
	
	for (i=0;i<max_submenus;i++){
		if (parent->menu[i] == 0){
			Com_Printf("Found !\n");
			parent->maxmenu++;
			parent->menu[i] = child;
			return;
		}
	}
	
}


/*
	Function for removing a menu from the console it works with the identifier not the name !
*/
void MOSDM_Console_Delete_Menu(void){
	MOSDM_New_Delete_Menu(Cmd_Argv(1));
}
/*
	this will remove the currently selected menu
*/
void MOSDM_Edit_Delete_Menu(void){
	MOSDM_New_Delete_Menu(activemenu->menu[activemenu->sub_active]->identifier);
	activemenu->sub_active--;
	if (activemenu->sub_active <0)
		activemenu->sub_active = 0;
}


void MOSDM_Draw_Menu(int x,int y,mvd_menu_t *current){
	int i,z,w,k,j;
	mvd_menu_t	*start;
	start = current;
	while (start != start->parent)
		start = start->parent;
	w=0;
	z=0;
	while (start != current && start->type == 0){
		for (i=0;i<max_submenus;i++){
			if (i == start->sub_active){
				if (i == start->sub_active)
					Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%s","&ca0a",start->menu[i]->name),1);
				else
					Draw_String(x+(w *8),y+(z++ * 8),start->menu[i]->name);
				start = start->menu[i];
				w++;
				break;
			}else{
					Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%s","&c707",start->menu[i]->name),1);

			}
		}
	}
	if (current->type == 0 || current->type == 3 || current->type == 4 || current->type == 8)
		for (i=0;i<max_submenus;i++){
			if (current->menu[i] != 0)
				if (i == current->sub_active){
					if (current->menu[i]->type == 9){
						Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%s: %s","&c0f0",current->menu[i]->name,*current->menu[i]->charptr),1);
					}else
						Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%s","&c0f0",current->menu[i]->name),1);
					if (current->menu[i]->type == 1){
						k=z-1;
						if (current->menu[i]->intptr){
							Draw_ColoredString(x+((2+w+current->width)*8),y+(k++ * 8),va("%s%i","&ce00",**current->menu[i]->intptr),1);
							}
						if (current->menu[i]->floatptr){
							Draw_ColoredString(x+((2+w+current->width)*8),y+(k++ * 8),va("%s%f","&ce00",**current->menu[i]->floatptr),1);
						}
					}
					for (j=0,k=z-1;j<max_submenus;j++)
						if (current->menu[i]->menu[j]){
							if (current->menu[i]->type == 0)
								Draw_ColoredString(x+((2+w+current->width)*8),y+(k++ * 8),va("%s%s","&ce00",current->menu[i]->menu[j]->name),1);
							if (current->menu[i]->type == 2){
								if (current->menu[i]->intptr){
									Draw_ColoredString(x+((2+w+current->width)*8),y+(k++ * 8),va("%s%s","&ce00",current->menu[i]->menu[**current->menu[i]->intptr]->name),1);
									break;
								}
								if (current->menu[i]->floatptr){
									Draw_ColoredString(x+((2+w+current->width)*8),y+(k++ * 8),va("%s%s","&ce00",current->menu[i]->menu[(int)**current->menu[i]->floatptr]->name),1);
									break;
								}
							}
						}
				}else{
					if (current->menu[i]->type == 9){
						Draw_String(x+(w *8),y+(z++ * 8),va("%s: %s",current->menu[i]->name,*current->menu[i]->charptr));
					}else
						Draw_String(x+(w *8),y+(z++ * 8),current->menu[i]->name);
				}
			else
				return;
		}
	if (current->type ==1){
		if (current->intptr){
			Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%i","&c0f0",**current->intptr),1);
		}else if (current->floatptr){
				Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%f","&c0f0",**current->floatptr),1);
		}else
			return;
	}
	if (current->type ==2){
			if (current->intptr)
				Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%s","&c0f0",current->menu[**current->intptr]->name),1);
			else if (current->floatptr)
				Draw_ColoredString(x+(w*8),y+(z++ * 8),va("%s%s","&c0f0",current->menu[(int)**current->floatptr]->name),1);
			else
			return;
	}
	if (current->type == 5){
		if (current->variable->value == 0 || current->variable->value == 1)
			Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%s","&c0f0",current->menu[(int)current->variable->value]->name),1);
	}
	if (current->type == 6){
		Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%f","&c0f0",current->variable->value),1);
	}
	if (current->type == 7){
		Draw_ColoredString(x+(w *8),y+(z++ * 8),va("%s%f","&c0f0",**current->vecptr),1);
	}

}

/*
	This Function automaticaly updates the track menu players get added etc
	could add teamnames as submenus for teamgames
*/
void MOSDM_New_Update_Track(void){
	int i;
	menu_llist_t *p;
	mvd_menu_t *track_menu;
	int quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,"track")){
			quit = 0;
			track_menu = p->menu;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("The menu \"track\" doesnt exist\n");
				Cvar_Set(&mosdm_new_update_track,"0");
				return;
			}
		}
	}
	for (i=0;i<32;i++){
		track_menu->menu[i] = 0;
		if (cl.players[i].name[0] && !cl.players[i].spectator){
			MOSDM_New_Delete_Menu(cl.players[i].name);
			MOSDM_NEW_ADD_MENU("track",cl.players[i].name,cl.players[i].name,4,NULL,NULL,NULL,0,0,0,NULL,NULL,"track",NULL,NULL);
			Com_Printf("%s\n",cl.players[i].name);
		}
	}
}

void MOSDM_Cam_Track_Set(void){
	c_curve_t *current;
	if (activemenu->max == 1)
		current = &cameras[selected_cam];
	else if (activemenu->max == 2)
		current = &views[selected_view];
	else 
		return;
	
	strcpy(current->track,cl.players[(int)activemenu->restrict].name);
	VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[(int)activemenu->restrict].origin,current->track_origin[1]);

}

void MOSDM_New_Update_cam_ptrack(void){
	int i,z;
	menu_llist_t *p;
	mvd_menu_t *ctrack_menu;
	mvd_menu_t *vtrack_menu;
	int quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,"vptrack")){
			quit = 0;
			vtrack_menu = p->menu;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("The menu \"vptrack\" doesnt exist\n");
				Cvar_Set(&mosdm_new_update_ptrack,"0");
				return;
			}
		}
	}
	quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,"cptrack")){
			quit = 0;
			ctrack_menu = p->menu;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("The menu \"cptrack\" doesnt exist\n");
				Cvar_Set(&mosdm_new_update_ptrack,"0");
				return;
			}
		}
	}
	for (i=max_submenus;i>=0;i--){
		if (vtrack_menu->menu[i]){
			MOSDM_New_Delete_Menu(vtrack_menu->menu[i]->identifier);
		}
		if (ctrack_menu->menu[i]){
			MOSDM_New_Delete_Menu(ctrack_menu->menu[i]->identifier);
		}
	}

	for (i=0,z=0;i<32;i++){
		if (cl.players[i].name[0] && !cl.players[i].spectator){
			MOSDM_NEW_ADD_MENU("vptrack",cl.players[i].name,va("vpt_%s",cl.players[i].name),8,NULL,NULL,NULL,0,2,i,MOSDM_Cam_Track_Set,NULL,NULL,NULL,NULL);
			MOSDM_NEW_ADD_MENU("cptrack",cl.players[i].name,va("cpt_%s",cl.players[i].name),8,NULL,NULL,NULL,0,1,i,MOSDM_Cam_Track_Set,NULL,NULL,NULL,NULL);
		}
	}
	
}

void MOSDM_Cam_ETrack_Set(void){
	c_curve_t *current;
	if (activemenu->max == 1)
		current = &cameras[selected_cam];
	else if (activemenu->max == 2)
		current = &views[selected_view];
	else 
		return;
/*	
	strcpy(current->track,"entity");
	strcpy(current->track_modelname,cl_visents.list[(int)activemenu->restrict].model->name);
	VectorCopy(cl_visents.list[(int)activemenu->restrict].origin,current->track_origin[0]);
	etrack_time = Sys_DoubleTime();
	etrack_active = (int)activemenu->max;
*/	

}
/*
void MOSDM_New_Update_cam_etrack(void){
	int i;
	menu_llist_t *p;
	mvd_menu_t *ctrack_menu;
	mvd_menu_t *vtrack_menu;
	int quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,"vetrack")){
			quit = 0;
			vtrack_menu = p->menu;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("The menu \"vetrack\" doesnt exist\n");
				Cvar_Set(&mosdm_new_update_etrack,"0");
				return;
			}
		}
	}
	quit = 1;
	p = &menus;
	while (quit){
		if (!strcmp(p->menu->identifier,"cetrack")){
			quit = 0;
			ctrack_menu = p->menu;
		}else{
			if (p->next){
				p=p->next;
			}else{
				Com_Printf("The menu \"cetrack\" doesnt exist\n");
				Cvar_Set(&mosdm_new_update_ptrack,"0");
				return;
			}
		}
	}
	for (i=0;i< cl_visents.count && i < 64;i++){
		if (vtrack_menu->menu[i])
			MOSDM_New_Delete_Menu(vtrack_menu->menu[i]->identifier);
		vtrack_menu->menu[i] = 0;
		if (ctrack_menu->menu[i])
			MOSDM_New_Delete_Menu(ctrack_menu->menu[i]->identifier);
		ctrack_menu->menu[i] = 0;
		if (!strcmp(cl_visents.list[i].model->name,"progs/grenade.mdl") | !strcmp(cl_visents.list[i].model->name,"progs/missile.mdl") | !strcmp(cl_visents.list[i].model->name,"progs/spike.mdl")){
			MOSDM_NEW_ADD_MENU("vetrack",cl_visents.list[i].model->name,va("vet %s",cl_visents.list[i].model->name),8,NULL,NULL,NULL,0,2,i,MOSDM_Cam_ETrack_Set,NULL,NULL,NULL,NULL);
			MOSDM_NEW_ADD_MENU("cetrack",cl_visents.list[i].model->name,va("cet %s",cl_visents.list[i].model->name),8,NULL,NULL,NULL,0,1,i,MOSDM_Cam_ETrack_Set,NULL,NULL,NULL,NULL);
		}
	}
	
}
*/
/*
	The Main function for the new menu
*/
void MOSDM_NEW(void){
	int z = 0;
	int sub_x,sub_y;
	if (!mosdm_newmenu.value)
		return;
	
	
	if ((last_track_update+0.5) < Sys_DoubleTime()){
		last_track_update = Sys_DoubleTime();
		if (mosdm_new_update_track.value)
			MOSDM_New_Update_Track();
/*
		if (mosdm_new_update_ptrack.value)
			MOSDM_New_Update_cam_ptrack();
		if (mosdm_new_update_etrack.value)
			MOSDM_New_Update_cam_etrack();
			*/
	}
	
	
	sub_x  = mosdm_sub_x.value;
	sub_y  = mosdm_sub_y.value;
		
	if (mosdm_new_draw.value)
		MOSDM_Draw_Menu(sub_x,sub_y,activemenu);
	

}

/*
*****************************************************
*	The old menu starts here \m/					*
*	the ole one get replaced because adding new 	*
*	variables to it was a pain in the ass			*
*****************************************************
*/

// Main loop for what so ever
void MOSDM_Main(void){
	c_curve_t *current;
	int main_x,main_y,sub_x,sub_y;
	int i =0;
	if (mosdm_newmenu.value)
		return;
	if (!show_me)
		return;
	main_x = mosdm_main_x.value;
	main_y = mosdm_main_y.value;
	sub_x  = mosdm_sub_x.value;
	sub_y  = mosdm_sub_y.value;
	if (!selected)
		Draw_Fill(main_x,main_y+(active * 8),120,8,mosdm_md.value);
	else
		Draw_Fill(main_x,main_y+(active * 8),120,8,mosdm_ma.value);

	if (selected_sub && show_info){
		if (!select_sub){
			Draw_Fill(sub_x,sub_y+(sub_active * 8),140,8,mosdm_md.value);
		}else{
			Draw_Fill(sub_x,sub_y+(sub_active * 8),140,8,mosdm_ma.value);
			if (!select_sub_sub){
			Draw_Fill(sub_x + 158,sub_y+(sub_active * 8),12,8,mosdm_md.value);
			}else{
			Draw_Fill(sub_x + 158,sub_y+(sub_active * 8),12,8,mosdm_ma.value);
				if (!edit_plane){
					Draw_Fill(sub_x + 182,sub_y+(sub_active * 8),12,8,mosdm_md.value);
				}else{
					Draw_Fill(sub_x + 182,sub_y+(sub_active * 8),12,8,mosdm_ma.value);
				}
			}

		}

	}

	if (!wich_one){
		for (i = 0; i<8;i++)
			if (i == 1)
				Draw_String(main_x,main_y+(i * 8),va("%2i. %s: %i",i+1,camera_info_strings[i],selected_cam));
			else if (i == 4)
				Draw_String(main_x,main_y+(i * 8),va("%2i. %s: %f",i+1,camera_info_strings[i],modifier));
			else if (i ==5)
				if (cam_show_all)
					Draw_String(main_x,main_y+(i*8),va("%2i. Show all",i+1));
				else
					Draw_String(main_x,main_y+(i*8),va("%2i. Hide all",i+1));
			else if (i == 6)
				if (cam_enable_all)
					Draw_String(main_x,main_y+(i*8),va("%2i. Enable all",i+1));
				else
					Draw_String(main_x,main_y+(i*8),va("%2i. Disable all",i+1));
			else
				Draw_String(main_x,main_y+(i * 8),va("%2i. %s",i+1,camera_info_strings[i]));
		current=&cameras[selected_cam];
		if (show_info){
			i=0;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10i","Enable",current->enable));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10i","Show",current->showme));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Start",current->starttime));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Stop",current->stoptime));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Modifier Start",current->m_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Modifier Stop",current->m_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Fov Start",current->fov_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Fov Stop",current->fov_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Demospeed Start",current->ds_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Demospeed Stop",current->ds_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Rotation Start",current->r_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Rotation Stop",current->r_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","DOF Jitter Start",current->dof_modifier_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","DOF Jitter Stop",current->dof_modifier_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%s","Console Commands",current->consolecommands));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%5.5f %5.5f %5.5f","Point 1",current->ctrlpoints[0][0],current->ctrlpoints[0][1],current->ctrlpoints[0][2]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%5.5f %5.5f %5.5f","Point 2",current->ctrlpoints[1][0],current->ctrlpoints[1][1],current->ctrlpoints[1][2]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%5.5f %5.5f %5.5f","Point 3",current->ctrlpoints[2][0],current->ctrlpoints[2][1],current->ctrlpoints[2][2]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%5.5f %5.5f %5.5f","Point 4",current->ctrlpoints[3][0],current->ctrlpoints[3][1],current->ctrlpoints[3][2]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s : %i  %c","Edit Point",sub_pe+1,sub_pe_plane[sub_pe_plane_select]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%s","Track",current->track));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%f %f %f %s","Track Origin",current->track_origin[0][0],current->track_origin[0][1],current->track_origin[0][2],current->track_modelname));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s : %f","Length",current->length));
			i++;
			Draw_String(sub_x,sub_y+(i*8),"Set Start To end of previous");
			i++;
			Draw_String(sub_x,sub_y+(i*8),"Set End To start of next");
			i++;
			Draw_String(sub_x,sub_y+(i*8),"Make it straight");
			i++;
			Draw_String(sub_x,sub_y+(i*8),"Back");
		}
	}else{
		for (i = 0; i<8;i++)
			if (i == 1)
				Draw_String(main_x,main_y+(i * 8),va("%2i. %s: %i",i+1,view_info_strings[i],selected_view));
			else if (i == 4)
				Draw_String(main_x,main_y+(i * 8),va("%2i. %s: %f",i+1,view_info_strings[i],modifier));
					else if (i ==5)
				if (view_show_all)
					Draw_String(main_x,main_y+(i*8),va("%2i. Show all",i+1));
				else
					Draw_String(main_x,main_y+(i*8),va("%2i. Hide all",i+1));
			else if (i == 6)
				if (view_enable_all)
					Draw_String(main_x,main_y+(i*8),va("%2i. Disable all",i+1));
				else
					Draw_String(main_x,main_y+(i*8),va("%2i. Enable all",i+1));
			else
				Draw_String(main_x,main_y+(i * 8),va("%2i. %s",i+1,view_info_strings[i]));
		if (show_info){
			current=&views[selected_view];
			i=0;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10i","Enable",current->enable));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10i","Show",current->showme));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Start",current->starttime));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Stop",current->stoptime));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Modifier Start",current->m_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Modifier Stop",current->m_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Fov Start",current->fov_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Fov Stop",current->fov_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Demospeed Start",current->ds_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Demospeed Stop",current->ds_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Rotation Start",current->r_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","Rotation Stop",current->r_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","DOF Jitter Start",current->dof_modifier_start));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%10f","DOF Jitter Stop",current->dof_modifier_stop));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%s","Console Commands",current->consolecommands));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%5.5f %5.5f %5.5f","Point 1",current->ctrlpoints[0][0],current->ctrlpoints[0][1],current->ctrlpoints[0][2]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%5.5f %5.5f %5.5f","Point 2",current->ctrlpoints[1][0],current->ctrlpoints[1][1],current->ctrlpoints[1][2]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%5.5f %5.5f %5.5f","Point 3",current->ctrlpoints[2][0],current->ctrlpoints[2][1],current->ctrlpoints[2][2]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%5.5f %5.5f %5.5f","Point 4",current->ctrlpoints[3][0],current->ctrlpoints[3][1],current->ctrlpoints[3][2]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s : %i  %c","Edit Point",sub_pe+1,sub_pe_plane[sub_pe_plane_select]));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%s","Track",current->track));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s :%f %f %f %s","Track Origin",current->track_origin[0][0],current->track_origin[0][1],current->track_origin[0][2],current->track_modelname));
			i++;
			Draw_String(sub_x,sub_y+(i*8),va("%17s : %f","Length",current->length));
			i++;
			Draw_String(sub_x,sub_y+(i*8),"Set Start To end of previous");
			i++;
			Draw_String(sub_x,sub_y+(i*8),"Set End To start of next");
			i++;
			Draw_String(sub_x,sub_y+(i*8),"Make it straight");
			i++;
			Draw_String(sub_x,sub_y+(i*8),"Back");
		}
	}
}



void MOSDM_Draw_Info(int sx,int sy){
	
	//Draw_String(sx,sy,


}

void MOSDM_Toggle_Draw (void){
	c_curve_t *current,*next,*previous;
	int i;
	int cd_sse,cd_ses;

	//update_subwindow = 1;

	if(show_me ==0){
		show_me=1;
		return;
	}
	
	if (!wich_one){
		current=&cameras[selected_cam];
		if ((selected_cam + 1 ) > max_cameras){
			cd_ses=1;
		}else{
			next=&cameras[selected_cam+1];
		}
		if ((selected_cam -1) < 0){
			cd_sse=1;
		}else{
			previous=&cameras[selected_cam-1];
		}
	}else{
		current=&views[selected_view];
		if ((selected_view + 1 ) > max_cameras){
			cd_ses=1;
		}else{
			next=&views[selected_view+1];
		}
		if ((selected_view -1) < 0){
			cd_sse=1;
		}else{
			previous=&views[selected_view-1];
		}
	}
	



	if (active == 0){
		wich_one = !wich_one;
	}

	if (active == 1){
		selected = !selected;
	}

	if (active == 2){
		show_info = !show_info;
	}

	if (active == 3){

		show_info= 1;
		if (selected_sub){
			if (sub_active == 0)
				current->enable = !current->enable;
			if (sub_active == 1)
				current->showme = !current->showme;
			if (sub_active > 1 && sub_active < 14)
				select_sub = !select_sub;
			
			
			if (sub_active == 15)
				VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,current->ctrlpoints[0]);
			if (sub_active == 16)
				VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,current->ctrlpoints[1]);
			if (sub_active == 17)
				VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,current->ctrlpoints[2]);
			if (sub_active == 18)
				VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,current->ctrlpoints[3]);

			if (sub_active == 19){
				if (select_sub){
					if (select_sub_sub)
						edit_plane = !edit_plane;
					else
					select_sub_sub = !select_sub_sub;
				}else{
					select_sub = !select_sub;
				}
			}

			if (sub_active == 20){
				if (select_sub == 0){
					MOSDM_Get_Player_List();
				}
				select_sub = !select_sub;
			}

			if (sub_active == 21){
				if (select_sub)
					draw_snd_cam = 0;
				else
					draw_snd_cam = 1;
				select_sub = !select_sub;
			}

			if (sub_active == 22){
				MVD_Calc_Length(current);
			}
			if (sub_active == 23 && !cd_sse){
				VectorCopy(previous->ctrlpoints[3],current->ctrlpoints[0]);
				current->starttime			= previous->stoptime;				// time
				current->fov_start			= previous->fov_stop;				// fov
				current->r_start			= previous->r_stop;					// rotation
				current->ds_start			= previous->ds_stop;				// demospeed
				current->dof_modifier_start	= previous->dof_modifier_stop;		// dof
			}

			if (sub_active == 24 && !cd_ses){
				VectorCopy(next->ctrlpoints[0],current->ctrlpoints[3]);
				current->stoptime = next->starttime;
			}

			if (sub_active == 25){
				VectorJogi(current->ctrlpoints[3], current->ctrlpoints[0], current->ctrlpoints[1],1/3);
				VectorJogi(current->ctrlpoints[0], current->ctrlpoints[3], current->ctrlpoints[2],1/3);

			}

			if (sub_active == 26){
				selected_sub = 0;
				selected = 0;
				sub_active = 0;
			}
		}else{
			if (!selected_sub){
				selected = !selected;
				selected_sub = 1;
			}
		}

	}

	if (active == 4){
		selected = !selected;
	}

	if (active == 5){
		if (!wich_one){
			if (cam_show_all)
				for (i = 0 ; i < max_cameras && cameras[i].ctrlpoints[0][0];i++)
					cameras[i].showme = 1;
			else
				for (i = 0 ; i < max_cameras && cameras[i].ctrlpoints[0][0];i++)
					cameras[i].showme = 0;
			cam_show_all = !cam_show_all;
		}else{
			if (view_show_all)
				for (i = 0 ; i < max_cameras && views[i].ctrlpoints[0][0];i++)
					views[i].showme = 1;
			else
				for (i = 0 ; i < max_cameras && views[i].ctrlpoints[0][0];i++)
					views[i].showme = 0;
			view_show_all = !view_show_all;
		}
	}

	if (active == 6){
		if (!wich_one){
			if (cam_enable_all)
				for (i = 0 ; i < max_cameras && cameras[i].ctrlpoints[0][0];i++)
					cameras[i].enable = 1;
			else
				for (i = 0 ; i < max_cameras && cameras[i].ctrlpoints[0][0];i++)
					cameras[i].enable = 0;
			cam_enable_all = !cam_enable_all;
		}else{
			if (view_enable_all)
				for (i = 0 ; i < max_cameras && views[i].ctrlpoints[0][0];i++)
					views[i].enable = 1;
			else
				for (i = 0 ; i < max_cameras && views[i].ctrlpoints[0][0];i++)
					views[i].enable = 0;
			view_enable_all = !view_enable_all;
		}
	}
	

	if (active == 7){
		show_me=0;
	}
}

void MOSDM_Change_Active (int max,int low, int type){
	c_curve_t *current;
	vec3_t	view;
	extern vec3_t	snd_cam_pos;

	if (!wich_one){
		current=&cameras[selected_cam];
	}else{
		current=&views[selected_view];
	}

	if (!selected){
		if (type){
			active++;
		}else {
			active--;
		}
		if ( active > max ){
			active=0;
		}
		if ( active < low ){
			active=max;
		}
	}else{
		if (active == 1){
			if (!wich_one){
				if (!type){
					selected_cam++;
				}else {
					selected_cam--;
				}
				if ( selected_cam >= max_cameras ){
					selected_cam=0;
				}
				if ( selected_cam < 0 ){
					selected_cam=max_cameras-1;
				}
			}else{
				if (!type){
					selected_view++;
				}else {
					selected_view--;
				}
				if ( selected_view > max_cameras ){
					selected_view=0;
				}
				if ( selected_view < 0 ){
					selected_view=max_cameras;
				}
			}
		}
		if (active == 3){
			if (selected_sub){
				if (!select_sub){
					if (!type){
						sub_active--;
					}else {
						sub_active++;
					}
					if ( sub_active > 26 ){
						sub_active=0;
					}
					if ( sub_active < 0 ){
						sub_active=26;
					}
				}else{
					if (sub_active == 2){
						/*if (current->starttime == 0){
							current->starttime = (cls.demotime - demostarttime);
							return;*/
						//}
						if (!type)
							current->starttime += modifier;
						else
							current->starttime -= modifier;
						
						if (current->starttime < 0)
							current->starttime = 0;

						if (current->stoptime < current->starttime)
							current->stoptime = current->starttime;
					}
					if (sub_active == 3){
						if (!type)
							current->stoptime += modifier;
						else
							current->stoptime -= modifier;
						
						if (current->stoptime < 0)
							current->stoptime = 0;

						if (current->stoptime < current->starttime)
							current->starttime = current->stoptime;
					}
					if (sub_active == 4)
						if (!type)
							current->m_start += modifier;
						else
							current->m_start -= modifier;
					if (sub_active == 5)
						if (!type)
							current->m_stop += modifier;
						else
							current->m_stop -= modifier;
					if (sub_active == 6)
						if (!type)
							current->fov_start += modifier;
						else
							current->fov_start -= modifier;
					if (sub_active == 7)
						if (!type)
							current->fov_stop += modifier;
						else
							current->fov_stop -= modifier;
					if (sub_active == 8)
						if (!type)
							current->ds_start += modifier;
						else
							current->ds_start -= modifier;
					if (sub_active == 9)
						if (!type)
							current->ds_stop += modifier;
						else
							current->ds_stop -= modifier;
					if (sub_active == 10)
						if (!type)
							current->r_start += modifier;
						else
							current->r_start -= modifier;
					if (sub_active == 11)
						if (!type)
							current->r_stop += modifier;
						else
							current->r_stop -= modifier;
					if (sub_active == 12)
						if (!type)
							current->dof_modifier_start += modifier;
						else
							current->dof_modifier_start -= modifier;
					if (sub_active == 13)
						if (!type)
							current->dof_modifier_stop += modifier;
						else
							current->dof_modifier_stop -= modifier;

					if (sub_active == 19){
						if (!select_sub_sub){
							if (!type)
								sub_pe++;
							else
								sub_pe--;
							if (sub_pe<0)
								sub_pe = 3;
							if (sub_pe>3)
								sub_pe = 0;
						}else if (select_sub_sub && !edit_plane){
							if (!type)
								sub_pe_plane_select++;
							else
								sub_pe_plane_select--;
							if (sub_pe_plane_select<0)
								sub_pe_plane_select = 2;
							if (sub_pe_plane_select>2)
								sub_pe_plane_select = 0;
						}else {
							if (!type)
								current->ctrlpoints[sub_pe][sub_pe_plane_select]+= modifier;
							else
								current->ctrlpoints[sub_pe][sub_pe_plane_select]-= modifier;

						}

					}


					if (sub_active == 20){
						if(!type)
							sub_track++;
						else
							sub_track--;
					
						if (sub_track > mvd_nicks_count )
							sub_track = 0;
						if (sub_track < 0 )
							sub_track = mvd_nicks_count;
						strcpy(current->track,mvd_nicks[sub_track]);

					}

					if (sub_active == 21){
						if(!type)
							sub_track_origin++;
						else
							sub_track_origin--;
					
						if (sub_track_origin > cl_visents.count )
							sub_track_origin = 0;
						if (sub_track_origin < 0 )
							sub_track_origin = cl_visents.count -1;
						VectorCopy(cl_visents.list[sub_track_origin].origin,current->track_origin[0]);
						VectorSubtract(current->track_origin[0],r_refdef.vieworg,view);
//						sprintf(current->track_modelname,"%s",cl_visents.list[sub_track_origin].model->name);
						mvd_vectoangles(view,snd_cam_pos);
					}
				}
			}
		}
		if (active == 4){
			if (!type){
				modifier+=0.5;
			}else {
				modifier-=0.5;
			}
			if ( modifier < 0 ){
				modifier=0;
			}
		}
	}
}



void MOSDM_Up (void) {
	//update_subwindow = 1;
	MOSDM_Change_Active(7,0,0);
}

void MOSDM_Down (void) {
	//update_subwindow = 1;
	MOSDM_Change_Active(7,0,1);
}

void MOSDM_Back (void) {
	if (!selected){
		show_me = 0;
	
	}
	if (selected){

		if (sub_active == 19){
			if (select_sub_sub && edit_plane){
					edit_plane = 0;
					return;
				}
			if (select_sub_sub && !edit_plane){
				select_sub_sub = 0;
				return;
			}
			if (!select_sub_sub && !edit_plane){
				select_sub = 0;
				return;
			}
		}else{
		selected_sub = 0;
		selected = 0;
		return;
		}
	}
}

void MOSDM_Toggle_Info (void) {

	show_info = !show_info;

}



void MOSDM_Init(void){

	Cmd_AddCommand("mosdm_toggle",MOSDM_Toggle_Draw);
	Cmd_AddCommand("mosdm_up",MOSDM_Up);
	Cmd_AddCommand("mosdm_down",MOSDM_Down);
	Cmd_AddCommand("mosdm_back",MOSDM_Back);
	Cmd_AddCommand("mosdm_info",MOSDM_Toggle_Info);
	Cmd_AddCommand("mosdm_new_left",MOSDM_NEW_LEFT);
	Cmd_AddCommand("mosdm_new_right",MOSDM_NEW_RIGHT);
	Cmd_AddCommand("mosdm_new_up",MOSDM_NEW_UP);
	Cmd_AddCommand("mosdm_new_down",MOSDM_NEW_DOWN);
	Cmd_AddCommand("mosdm_new_check_menus",MOSDM_Check_Menus);
	Cmd_AddCommand("mosdm_new_add_select_menu",MOSDM_Console_Add_select_Menu);
	Cmd_AddCommand("mosdm_new_add_menu",MOSDM_Console_Add_Menu);
	Cmd_AddCommand("mosdm_new_add_toggle_menu",MOSDM_Console_Add_toggle_Menu);
	Cmd_AddCommand("mosdm_new_add_modify_menu",MOSDM_Console_Add_modify_Menu);
	Cmd_AddCommand("mosdm_new_del_menu",MOSDM_Console_Delete_Menu);

	Cmd_AddCommand("mosdm_new_edit_del",MOSDM_Edit_Delete_Menu);

	Cmd_AddCommand("mosdm_new_add_internal",MOSDM_NEW_Console_Static);

	Cmd_AddCommand("mosdm_new_link",MOSDM_NEW_Console_Link);

	Cmd_AddCommand("mosdm_test",MOSDM_TEST);

	Cmd_AddCommand("cams_event_add",Cams_Event_Add);

	Cvar_Register(&mosdm_main_x);
	Cvar_Register(&mosdm_main_y);
	Cvar_Register(&mosdm_sub_x);
	Cvar_Register(&mosdm_sub_y);
	Cvar_Register(&mosdm_ma);
	Cvar_Register(&mosdm_md);
	Cvar_Register(&mosdm_modifier);
	Cvar_Register(&mosdm_newmenu);
	Cvar_Register(&mosdm_new_update_track);
	Cvar_Register(&mosdm_new_edit_menu);

	Cvar_Register(&mosdm_new_update_ptrack);
	Cvar_Register(&mosdm_new_update_etrack);

	Cvar_Register(&mosdm_new_draw);
	MOSDM_NEW_INIT();
}

void MOSDM_Screen_Hook (void){
	MOSDM_Main();
	MOSDM_NEW();
}

void MOSDM_Get_Player_List(void){
	int i;
	int z = 1;

	mvd_nicks[0][0] = '\0';


	for (i=0;i<MAX_CLIENTS;i++)
		if (!cl.players[i].spectator && cl.players[i].name[0])
			strcpy(mvd_nicks[z++],cl.players[i].name);
	strcpy(mvd_nicks[z++],"entity");
	mvd_nicks_count = z;

}

void MOSDM_Change_View(void){
	extern vec3_t snd_cam_pos;
	if (draw_snd_cam){
		VectorCopy(snd_cam_pos,r_refdef.viewangles);
		r_refdef.viewangles[0] = -r_refdef.viewangles[0];
	}
}


qbool MOSDM_Onchange_Modifier(cvar_t *current,char *string){
	modifier = atof(string);
	Cvar_Set(current,string);
	return true;
}


















