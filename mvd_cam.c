// MVD_Cam
// Camera Scripting in mvd demos
#include "quakedef.h"
#include "mvd_cam.h"
#include <time.h>
#include <string.h>
#include "gl_model.h"
#include "gl_local.h"
#include "pmove.h"

int inited = 0;
int cam_in_progress = -1;
int view_in_progress = -1;
double cam_pos1,cam_pos2,cam_pos3;
double view_pos1,view_pos2,view_pos3;
double view_oldpos1,view_oldpos2,view_oldpos3;
double testtime;
double last_tt_m =0 ;
double tt_m = 0;
double *testtime_pointer = &tt_m;
vec3_t	cam_pos,view_pos,cam_angles,snd_cam_pos,snd_cam_view;
byte *OSD_buffer;
int framenumb= -1;
int mvd_done = 0;
char *jogi_mvd_char_test = "";
mpic_t *mvd_picture;
int mvd_function_exist = 0;
int mvd_current_cam = -1;
int mvd_flip = 0;
double cam_start_time,cam_stop_time,testtime1;
double glob_rot;

double	mvd_mouse_to_view_time;
int		mvd_mouse_to_view_vnum;
int		mvd_mouse_to_view_cnum;
int		mvd_mouse_to_view;
int		mvd_mouse_to_view_cpoint;

vec3_t	lookat_pos;

c_curve_t cameras[max_cameras];
c_curve_t views[max_cameras];

tele_plane_t tele_cams[1];

c_event_t	dummy;



cvar_t	bezier_cams_enable = {"bezier_cams_enable", "0"};
cvar_t	bezier_cams_show_in_pip = {"beziers_cams_show_in_pip","0"};
cvar_t	rot_x = {"rot_x", "0"};
cvar_t	rot_y = {"rot_y", "0"};
cvar_t	rot_z = {"rot_z", "0"};
cvar_t	rot_switch = {"rot_switch", "0"};

byte* StringToRGB(char *s);

qbool ON_Change_Test_Time(cvar_t *var, char *string){
	int i;
	if (var->value == 0){
		testtime = 9999;
		for (i=0;i<max_cameras;i++)
			if (cameras[i].stoptime > 0){
				if (cameras[i].starttime < testtime)
					testtime = cameras[i].starttime;
				if (testtime == 0)
					break;
			}
	}
	Cvar_Set(var,string);
	return true;
}

qbool ON_Change_Cam (cvar_t *var, char *string){
	return true;
	strcpy(var->string,string);
	Cvar_Set(var,string);
#ifdef GLQUAKE
	mvd_picture->texnum = -1;
#endif
	return true;
}
cvar_t	jogi_mvd_track = {"jogi_mvd_track","0"};
cvar_t	jogi_mvd_test = {"jogi_mvd_test","0"};
cvar_t	jogi_mvd_test1 = {"jogi_mvd_test1","0"};
cvar_t	jogi_mvd_test_x = {"jogi_mvd_test_x","0"};
cvar_t	jogi_mvd_test_y = {"jogi_mvd_test_y","0"};
cvar_t	jogi_mvd_test_size_x = {"jogi_mvd_test_size_x","320",0 , ON_Change_Cam};
cvar_t	jogi_mvd_test_size_y = {"jogi_mvd_test_size_y","200" ,0 , ON_Change_Cam};
cvar_t	jogi_mvd_test_a = {"jogi_mvd_test_a","-1"};
cvar_t	jogi_mvd_test_b = {"jogi_mvd_test_b","1"};
cvar_t	jogi_mvd_test_scale = {"jogi_mvd_test_scale","0"};

cvar_t	jogi_mvd_entities = {"jogi_mvd_entities","0"};

cvar_t	testorito	=	{"testorito","0"};
cvar_t	testorito1	=	{"testorito1","0"};
cvar_t	testorito2	=	{"testorito2","0"};
cvar_t	testorito3	=	{"testorito3","0"};



cvar_t	beziers_mouse_move = {"beziers_mouse_move","0"};
cvar_t	beziers_mouse_move_modifier = {"beziers_mouse_move_modifier","100"};
cvar_t	beziers_mouse_move_type = {"beziers_mouse_move_type","0"};
cvar_t	beziers_mouse_move_number = {"beziers_mouse_move_number","0"};
cvar_t	beziers_mouse_move_point = {"beziers_mouse_move_point","1"};
cvar_t	beziers_mouse_move_plane = {"beziers_mouse_move_plane","xy"};

cvar_t	beziers_testtime = {"beziers_testtime","0",0,ON_Change_Test_Time};
cvar_t	beziers_testtime_modifier = {"beziers_testtime_modifier","1000"};

cvar_t	beziers_reverse = {"beziers_reverse","0"};

cvar_t	beziers_svl	=	{"beziers_svl","0"};

cvar_t	beziers_autodist	=	{"beziers_autodist","0"};

cvar_t	beziers_color_va	=	{"beziers_color_va","255 255 0"};
cvar_t	beziers_color_vd	=	{"beziers_color_vd","0 255 255"};
cvar_t	beziers_color_ca	=	{"beziers_color_ca","255 0 255"};
cvar_t	beziers_color_cd	=	{"beziers_color_cd","0 255 0"};
cvar_t	beziers_color_vl	=	{"beziers_color_vl","255 0 0"};

cvar_t	beziers_color_ep	=	{"beziers_color_ep","255 0 0"};

cvar_t	event_color			=	{"event_color","255 255 255"};

cvar_t	beziers_mouse_to_view_interval	=	{"beziers_mouse_to_view_interval","0.5"};






void MVD_Autodist(void){
	extern float distance;
	vec3_t impact, normal, dest, forward, right, up, origin;
	int i;
	entity_t	test;
	trace_t trace;

	if (!beziers_autodist.value)
		return;
			
	AngleVectors (r_refdef.viewangles,forward, right, up);
	VectorCopy(r_refdef.vieworg,origin);
			for (i=0;i<3;i++)
				dest[i] = r_refdef.vieworg[i] + forward[i] * 10000;

	//MVD_TraceLine (r_refdef.vieworg, dest, impact, normal, 0, 1, &test);
	trace = PM_TraceLine(r_refdef.vieworg, dest);
	VectorCopy(trace.endpos, impact);
	VectorCopy(trace.plane.normal, normal);
			if (!VectorCompare(dest, impact))
			{
				dest[0] = impact[0] + forward[0] * 8 + normal[0] * 4;
				dest[1] = impact[1] + forward[1] * 8 + normal[1] * 4;
				dest[2] = impact[2] + forward[2] * 8 + normal[2] * 4;
			}


	VectorCopy(dest,lookat_pos);
}

void MVD_Mouse_Move_Points(void){
	extern double mouse_x, mouse_y;
	double mm_x,mm_y,mm_z;
	int mode;
	vec3_t	vector, angles;
	c_curve_t *current;
	
	if (!beziers_mouse_move.value)
		return;

	if (beziers_mouse_move_type.value == 0)
		current = &cameras[(int)beziers_mouse_move_number.value];
	else if (beziers_mouse_move_type.value == 1)
		current = &views[(int)beziers_mouse_move_number.value];
	else{
		Com_Printf("beziers_mouse_move_type has to be 0 for cams or 1 for views\n");
		Cvar_Set(&beziers_mouse_move,"0");
		return;
	}
	
	if (!strstr(beziers_mouse_move_plane.string,"xy")){
		mm_x = mouse_x / beziers_mouse_move_modifier.value;
		mm_y = mouse_y / beziers_mouse_move_modifier.value;
		mm_z = 0;
	}else if (!strstr(beziers_mouse_move_plane.string,"xz")){
		mm_x = mouse_x / beziers_mouse_move_modifier.value;
		mm_y = 0;
		mm_z = mouse_y / beziers_mouse_move_modifier.value;
	}else if (!strstr(beziers_mouse_move_plane.string,"yz")){
		mm_x = 0;
		mm_y = mouse_x / beziers_mouse_move_modifier.value;
		mm_z = mouse_y / beziers_mouse_move_modifier.value;
	}else {
		Com_Printf("Use xy,xz or zy as values for beziers_mouse_move_plane\n");
		Cvar_Set(&beziers_mouse_move,"0");
		return;
	}
	mode = beziers_mouse_move_point.value;
	if (mode == 1 || mode == 2 || mode == 3 || mode == 4)
		mode--;
	else{
		mode = 0;
		Com_Printf("Use 1,2,3 or 4 as value for beziers_mouse_move_point\n");
		Cvar_Set(&beziers_mouse_move,"0");
		return;
	}

	current->ctrlpoints[mode][0] += mm_x;
	current->ctrlpoints[mode][1] += mm_y;
	current->ctrlpoints[mode][2] += mm_z;

	VectorSubtract(current->ctrlpoints[mode], cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin, vector);
	mvd_vectoangles(vector, angles);
	VectorCopy(angles,cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].viewangles);
	VectorCopy(angles,r_refdef.viewangles);
	r_refdef.viewangles[0] = -r_refdef.viewangles[0];
	


	//Com_Printf("%f %f %f\n",mm_x,mm_y,mm_z);
	
}





float texpoints[4][2];

float	texpoints[4][2] = {{1.0,0.0},{1.0,1.0},{0.0,0.0},{0.0,1.0}};

void MVD_Set_Tele_Plane_Points(void){
	int i;
	
	if(!strcmp(Cmd_Argv(1),"p1"))
		VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,tele_cams[0].setpoints[0]);
	if(!strcmp(Cmd_Argv(1),"p2"))
		VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,tele_cams[0].setpoints[1]);
	if(!strcmp(Cmd_Argv(1),"p3"))
		VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,tele_cams[0].setpoints[2]);
	if(!strcmp(Cmd_Argv(1),"p4"))
		VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,tele_cams[0].setpoints[3]);
	if(!strcmp(Cmd_Argv(1),"sp1")){
		tele_cams[0].setpoints[0][0] = atof(Cmd_Argv(2));
		tele_cams[0].setpoints[0][1] = atof(Cmd_Argv(3));
		tele_cams[0].setpoints[0][2] = atof(Cmd_Argv(4));
	}
	if(!strcmp(Cmd_Argv(1),"sp2")){
		tele_cams[0].setpoints[1][0] = atof(Cmd_Argv(2));
		tele_cams[0].setpoints[1][1] = atof(Cmd_Argv(3));
		tele_cams[0].setpoints[1][2] = atof(Cmd_Argv(4));
	}
	if(!strcmp(Cmd_Argv(1),"sp3")){
		tele_cams[0].setpoints[2][0] = atof(Cmd_Argv(2));
		tele_cams[0].setpoints[2][1] = atof(Cmd_Argv(3));
		tele_cams[0].setpoints[2][2] = atof(Cmd_Argv(4));
	}
	if(!strcmp(Cmd_Argv(1),"sp4")){
		tele_cams[0].setpoints[3][0] = atof(Cmd_Argv(2));
		tele_cams[0].setpoints[3][1] = atof(Cmd_Argv(3));
		tele_cams[0].setpoints[3][2] = atof(Cmd_Argv(4));
	}
	if(!strcmp(Cmd_Argv(1),"enable"))
		tele_cams[0].enable =  atoi(Cmd_Argv(2));
	if(!strcmp(Cmd_Argv(1),"test"))
		tele_cams[0].test =  atoi(Cmd_Argv(2));
	if(!strcmp(Cmd_Argv(1),"test1"))
		tele_cams[0].test1 =  atoi(Cmd_Argv(2));
	if(!strcmp(Cmd_Argv(1),"show"))
		for (i=0;i<4;i++)
			Com_Printf("p%i %f %f %f\n",i,tele_cams[0].setpoints[i][0],tele_cams[0].setpoints[i][1],tele_cams[0].setpoints[i][2]);

}



MVD_Calc_Bezier_new(float *pointer,double time, vec3_t vec);
MVD_Calc_Catmull_Rom(float *pointer,double time, vec3_t vec);
void Draw_3D_Text(vec3_t position, vec3_t orientation, int x, int y,char *text, int type);
void Draw_3D_Text_Position(vec3_t position, vec3_t destination, int x, int y, char *text, int type, float scale);
void Draw_3D_Text_Pop_Matrix(void);
void Draw_3D_Text_Position_Setup_Matrix(vec3_t position, vec3_t destination, float scale);

void MVD_Cam_Draw_Bezier(c_curve_t *current,int cint, int type){
	extern int selected_view,selected_cam;
	double t;
	vec3_t	point;
	extern vec3_t	*cc_point,*vv_point;
	int i,j,z;
	extern cvar_t testorito2;
	char text[128];
	color_t	mycolor;


	for (i=0,z=0;i<20;i++)
		if (((current->ctrlpoints[i][0] + current->ctrlpoints[i][1] + current->ctrlpoints[i][2]) != 0 ))
		{
				//Draw_AlphaRectangleRGB(0, 0, 100, 40, 1, 1, RGBA_TO_COLOR(0, 0, 0, 0));
				//glDepthFunc(GL_EQUAL);
				glDepthMask(GL_FALSE);
				Draw_3D_Text_Position_Setup_Matrix(current->ctrlpoints[i], r_refdef.vieworg, 1);

				glDisable(GL_TEXTURE_2D);
				glColor4f(0,1,0,1);
				glRectf(0, 0, 130, 40);
				glColor4f(0,0,0,1);
				glRectf(1, 1, 129, 39);
				glEnable(GL_TEXTURE_2D);

				snprintf(text, 128, "point: %i", i);
				Draw_String(0, 0, text);
				snprintf(text, 128, "x: %f", current->ctrlpoints[i][0]);
				Draw_String(8, 10, text);
				snprintf(text, 128, "y: %f", current->ctrlpoints[i][1]);
				Draw_String(8, 20, text);
				snprintf(text, 128, "z: %f", current->ctrlpoints[i][2]);
				Draw_String(8, 30, text);
				Draw_3D_Text_Pop_Matrix();
				glDepthMask(GL_TRUE);
			z++;
		}


	current->catmull_points = z-3;
	glDisable(GL_TEXTURE_2D);
	if (!type){
	// Cameras
		if (selected_cam == cint)
			glColor3ubv(StringToRGB(beziers_color_ca.string));
		else
			glColor3ubv(StringToRGB(beziers_color_cd.string));
	}else{
	// Views
		if (selected_view == cint)
			glColor3ubv(StringToRGB(beziers_color_va.string));
		else
			glColor3ubv(StringToRGB(beziers_color_vd.string));
	}
	
	glBegin(GL_LINE_STRIP);
	if (current->type == 0){
		for(i=0;i<=1000;i++) {
			if (i==0)
				t=0;
			else
				t = (float)i/1000;
			MVD_Calc_Bezier_new(&current->ctrlpoints[0][0],t,point);
			glVertex3fv(point);
		}
	}
	if (current->type == 1){
		for (j = 0 ;j<=(z-4);j++){
			for(i=0;i<=100;i++) {
				if (i==0)
					t=0;
				else
					t = (float)i/100;
				MVD_Calc_Catmull_Rom(&current->ctrlpoints[j][0],t,point);
				glVertex3fv(point);
			}
		}
	}
	glEnd();
	glPointSize(5.0);
	glBegin(GL_POINTS);
	for(i=0;i<z;i++){
		if (!type){
			if (*cc_point == current->ctrlpoints[i])
				glColor3f(1,0,0);
			else
				if (selected_cam == cint)
					glColor3ubv(StringToRGB(beziers_color_ca.string));
				else
					glColor3ubv(StringToRGB(beziers_color_cd.string));

		}else{
			if (*vv_point == current->ctrlpoints[i])
				glColor3f(1,0,0);
			else
				if (selected_view == cint)
					glColor3ubv(StringToRGB(beziers_color_va.string));
				else
					glColor3ubv(StringToRGB(beziers_color_vd.string));
		}
		
		
		glVertex3fv(&current->ctrlpoints[i][0]);
	}
	glEnd();
	glFlush();
	glEnable(GL_TEXTURE_2D);
}

int qglProject (float objx, float objy, float objz, float *model, float *proj, int *view, float* winx, float* winy, float* winz);

void MVD_Cam_Draw_Number(void){
	int j,k, view[4], tracknum = -1;
	float x,y,model[16], project[16], winz, *origin;
	extern vec3_t	*cc_point,*vv_point;
	return;
	origin = *cc_point;
	
	glGetFloatv(GL_MODELVIEW_MATRIX, model);
	glGetFloatv(GL_PROJECTION_MATRIX, project);
	glGetIntegerv(GL_VIEWPORT, (GLint *)view);
	qglProject(origin[0], origin[1], origin[2] + 28, model, project, view, &x, &y, &winz);
	j =  x * vid.width / glwidth;
	k =  (glheight - y) * vid.height / glheight;
	//Com_Printf("Test %f %f %f %f %f\n",origin[0],origin[1],origin[2],x,y);
	//Draw_String(j,k,"Selected");
	//Draw_String(100,100,"TEST");
}


void MVD_Cam_SVL(void){
	

	if (!beziers_svl.value)
		return;
	glDisable(GL_TEXTURE_2D);
	glColor3ubv(StringToRGB(beziers_color_vl.string));
	glPointSize(5.0);
	glBegin(GL_POINTS);
	glVertex3fv(snd_cam_pos);
	glVertex3fv(snd_cam_view);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(snd_cam_pos[0], snd_cam_pos[1], snd_cam_pos[2]); // origin of the line
		glVertex3f(snd_cam_view[0], snd_cam_view[1], snd_cam_view[2]); // ending point of the line
	glEnd();
	glFlush();
	glEnable(GL_TEXTURE_2D);


}

void MVD_Draw_LA(void){
	if (!beziers_autodist.value)
		return;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glColor3ubv(StringToRGB(beziers_color_vl.string));
	glPointSize(5.0);
	glBegin(GL_POINTS);
	glVertex3fv(lookat_pos);
	glEnd();
	glFlush();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);


}

void MVD_Cam_PEL(void){
	extern int wich_one,selected_view, selected_cam,sub_active,sub_pe,sub_pe_plane_select, select_sub,draw_compass;
	extern double compass_time;
	extern mvd_menu_t *activemenu;
	extern double	compass_min,	compass_max,	compass_restrict;
	vec3_t point[3][2];
	c_curve_t *current;
	int i;

	if (compass_time + 1 < Sys_DoubleTime())
		return;
	
	if (compass_min == 1)
		current = &cameras[selected_cam];
	else if (compass_min == 2)
		current = &views[selected_view];
	else 
		return;

	VectorCopy(current->ctrlpoints[(int)compass_max-1],point[0][1]);
	VectorCopy(current->ctrlpoints[(int)compass_max-1],point[1][1]);
	VectorCopy(current->ctrlpoints[(int)compass_max-1],point[2][1]);
	VectorCopy(current->ctrlpoints[(int)compass_max-1],point[0][0]);
	VectorCopy(current->ctrlpoints[(int)compass_max-1],point[1][0]);
	VectorCopy(current->ctrlpoints[(int)compass_max-1],point[2][0]);
	for (i=0;i<3;i++)
		point[i][1][i]+= 50;

	
	glDisable(GL_TEXTURE_2D);
	for (i=0;i<3;i++){
		if (compass_restrict == i)
			if (compass_min == 1)
				glColor3ubv(StringToRGB(beziers_color_ca.string));
			else
				glColor3ubv(StringToRGB(beziers_color_va.string));
		else
			if (compass_min == 1)
				glColor3ubv(StringToRGB(beziers_color_cd.string));
			else
				glColor3ubv(StringToRGB(beziers_color_vd.string));
	

		glBegin(GL_LINES);
			glVertex3f(point[i][0][0], point[i][0][1], point[i][0][2]); // origin of the line
			glVertex3f(point[i][1][0], point[i][1][1], point[i][1][2]); // ending point of the line
		glEnd();
		
	}
	glEnable(GL_TEXTURE_2D);
}

void MVD_Cam_ETL(void){
	extern int wich_one,selected_view, selected_cam, etrack_active;
	extern double etrack_time;
	extern mvd_menu_t *activemenu;
	vec3_t point[3][2];
	vec3_t origin;
	c_curve_t *current;
	int i;

	

	if (etrack_time + 1 < Sys_DoubleTime())
		return;

	if (etrack_active == 1)
		current = &cameras[selected_cam];
	else if (etrack_active == 2)
		current = &views[selected_view];
	else 
		return;

	

	VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,origin);

	VectorCopy(current->track_origin[1],point[0][1]);
	VectorCopy(current->track_origin[1],point[1][1]);
	VectorCopy(current->track_origin[1],point[2][1]);
	VectorCopy(current->track_origin[1],point[0][0]);
	VectorCopy(current->track_origin[1],point[1][0]);
	VectorCopy(current->track_origin[1],point[2][0]);

	for (i=0;i<3;i++){
		point[i][1][i]+= 50;
		point[i][0][i]-= 50;
	}


	
	glDisable(GL_TEXTURE_2D);
	glColor3ubv(StringToRGB(beziers_color_ep.string));

	
	for (i=0;i<3;i++){
		glBegin(GL_LINES);
		glVertex3f(point[i][0][0], point[i][0][1], point[i][0][2]); // origin of the line
		glVertex3f(point[i][1][0], point[i][1][1], point[i][1][2]); // ending point of the line
		glEnd();
	}
	glBegin(GL_LINES);
		glVertex3f(origin[0], origin[1], origin[2]);
		glVertex3f(current->track_origin[1][0],current->track_origin[1][1],current->track_origin[1][2]);
	glEnd();
	
	glEnable(GL_TEXTURE_2D);
}



void mvd_vectoangles(vec3_t vec, vec3_t ang) {
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
void MVD_Calc_Bezier4 (vec3_t p1,vec3_t p2,vec3_t p3,vec3_t p4,double mu,vec3_t vec){
	double mum1,mum13,mu3;

	mum1 = 1 - mu;
	mum13 = mum1 * mum1 * mum1;
	mu3 = mu * mu * mu;

	vec[0] = mum13*p1[0] + 3*mu*mum1*mum1*p2[0] + 3*mu*mu*mum1*p3[0] + mu3*p4[0];
	vec[1] = mum13*p1[1] + 3*mu*mum1*mum1*p2[1] + 3*mu*mu*mum1*p3[1] + mu3*p4[1];
	vec[2] = mum13*p1[2] + 3*mu*mum1*mum1*p2[2] + 3*mu*mu*mum1*p3[2] + mu3*p4[2];

}

int MVD_Calc_Catmull_Rom(float *pointer,double time, vec3_t vec){
	double t1,t2,t3,t4;
	int i,j;
	vec3_t	points[4];
	for (i=0,j=0;i<4;i++){
		if (*(pointer+(3*i)) != 0 || *(pointer+(3*i+1)) != 0 || *(pointer+(3*i+2)) != 0)
			j++;
	}
	if (j==1){
		vec[0] = *pointer;
		vec[1] = *(pointer+1);
		vec[2] = *(pointer+2);
	}
	if (j<4)
		return 0;
	for (i=0;i<4;i++){
		points[i][0] = *(pointer+(i*3));
		points[i][1] = *(pointer+(i*3+1));
		points[i][2] = *(pointer+(i*3+2));
	}

	for (i=0;i<3;i++){
		t1	=	points[1][i];
		t2	=	-0.5 * points[0][i] + 0.5 * points[2][i];
		t3	=	points[0][i] + -2.5 * points[1][i] + 2.0 * points[2][i] + -0.5 * points[3][i];
		t4	=	-0.5 * points[0][i] + 1.5 * points[1][i] + -1.5 * points[2][i] + 0.5 * points[3][i];
		vec[i] = ((t4*time + t3)*time +t2)*time + t1;

	}
	return 1;
}

int MVD_Calc_Bezier_new(float *pointer,double time, vec3_t vec){
	double t1,t2,t3,t4;
	int i,j;
	
	for (i=0,j=0;i<20;i++){
		if (*(pointer+(3*i)) != 0 || *(pointer+(3*i+1)) != 0 || *(pointer+(3*i+2)) != 0)
			j++;
	}
	if (j==1){
		vec[0] = *pointer;
		vec[1] = *(pointer+1);
		vec[2] = *(pointer+2);
	}
	t1 = 1 - time;
	t2 = pow(t1,j-1);
	t3 = pow(time,j-1);
	for (i=0;i<j;i++){
		if (i == 0){
			vec[0] = *(pointer+(3*i)) * t2;
			vec[1] = *(pointer+(3*i+1)) * t2;
			vec[2] = *(pointer+(3*i+2)) * t2;
		}else if ( i == j-1){
			vec[0] += *(pointer+(3*i)) * t3;
			vec[1] += *(pointer+(3*i+1)) * t3;
			vec[2] += *(pointer+(3*i+2)) * t3;
		} else {
			t4 = (j-1) * pow(time,i) * pow(t1,j-1-i);
			vec[0] += *(pointer+(3*i))   * t4;
			vec[1] += *(pointer+(3*i+1)) * t4;
			vec[2] += *(pointer+(3*i+2)) * t4;
		}
	}
	return 1;

}

void MVD_Calc_Length (c_curve_t *current){
	double x,mu,mum1,mum13,mu3,distance;
	vec3_t last_point, vec,p1,p2,p3,p4;

	VectorCopy(current->ctrlpoints[0],p1);
	VectorCopy(current->ctrlpoints[1],p2);
	VectorCopy(current->ctrlpoints[2],p3);
	VectorCopy(current->ctrlpoints[3],p4);

	distance = 0;
	for (x=0;x<=1;x+=0.001){
		mu = x;
		mum1 = 1 - mu;
		mum13 = mum1 * mum1 * mum1;
		mu3 = mu * mu * mu;
		vec[0] = mum13*p1[0] + 3*mu*mum1*mum1*p2[0] + 3*mu*mu*mum1*p3[0] + mu3*p4[0];
		vec[1] = mum13*p1[1] + 3*mu*mum1*mum1*p2[1] + 3*mu*mu*mum1*p3[1] + mu3*p4[1];
		vec[2] = mum13*p1[2] + 3*mu*mum1*mum1*p2[2] + 3*mu*mu*mum1*p3[2] + mu3*p4[2];
		if (x==0)
			VectorCopy(current->ctrlpoints[0],last_point);
		distance += sqrt(pow((vec[0]- last_point[0]),2) + pow((vec[1]- last_point[1]),2) + pow((vec[2]- last_point[2]),2));
		VectorCopy(vec,last_point);
	}
	current->length = distance;

}






void MVD_Show_Textures (void){
	int x;
	for (x=0;x<cl.worldmodel->numtextures;x++)
		Com_Printf("%i %s\n",x,cl.worldmodel->textures[x]->name);
}

void MVD_Replace_Textures (void){

		cl.worldmodel->textures[atoi(Cmd_Argv(1))]->gl_texturenum=atoi(Cmd_Argv(2));

}

//#define OverTime (a,b,c,d)		((a) = (b) + (((c) - (b))*d))
void MVD_Cam_Event_Check(c_curve_t *current);
void MVD_Move_Me (c_curve_t *current, int type){
	double time;
	double modifier;
	double demospeed;
	double rotation;
	double fovchange;
	double dof_change;
	char args[1024];
	vec3_t pos,view_vec;
	extern double demostarttime;
	int i,j,z;
	double t1,t2;

	time = ((testtime - demostarttime - current->real_starttime)/(current->stoptime - current->real_starttime));

	if (current->m_stop > 0 || current->m_start > 0){
		modifier = current->m_start + ((current->m_stop - current->m_start) * time);
	}else{
		modifier = 1;
	}
	time *= modifier;

	// Number of points
	for (i=0,z=0;i<20;i++)
		if (((current->ctrlpoints[i][0] + current->ctrlpoints[i][1] + current->ctrlpoints[i][2]) != 0 ))
			z++;
	z-=3;
	//time per point
	t1 = (double)1 / (double)z;
		
	for (j=0,t2=0;t2<=time && j <=z ;t2+=t1){
		j++;
	}
	j--;

	if (current->type == 0)
			MVD_Calc_Bezier_new(&current->ctrlpoints[0][0],time,pos);
	if (current->type == 1){
			MVD_Calc_Catmull_Rom(&current->ctrlpoints[j][0],(time - (t2-t1))/t1,pos);
	}
	if (type == 1){ // Cameras

		VectorCopy(pos,cam_pos);

		if (strlen(current->track) > 0){
			MVD_Find_Track_Player(current);
			MVD_Find_Track_Entity(current,1);
			VectorCopy(current->track_origin[1],cam_pos);
		}
		// do r_dof_jitter
		if (current->dof_modifier_start != 0 || current->dof_modifier_stop != 0){
			dof_change = current->dof_modifier_start + ((current->dof_modifier_stop - current->dof_modifier_start) *time);
			sprintf(args,"r_dof_jitter %f\n",dof_change);
			Cbuf_AddText(args);
		}
		// do the demospeed
		if (current->ds_stop > 0){
			if (current->ds_start == current->ds_stop){
				sprintf(args,"demo_setspeed %f\n",current->ds_start);
				Cbuf_AddText(args);
			}else{
				demospeed = current->ds_start + ((current->ds_stop - current->ds_start) * time);
				sprintf(args,"demo_setspeed %f\n",demospeed);
				Cbuf_AddText(args);
				Com_Printf(args);
			}
		}
		// do the fovchange
		if (current->fov_stop != 0 || current->fov_start != 0){
			fovchange = current->fov_start + ((current->fov_stop - current->fov_start) * time);
			//if (fovchange > 100)
			//	Com_Printf("%f\n",fovchange);
			sprintf(args,"fov %f\n",fovchange);
			Cbuf_AddText(args);
		}
		if (bezier_cams_show_in_pip.value == 0 && beziers_svl.value == 0){
			VectorCopy(cam_pos,r_refdef.vieworg);
		}else{
			VectorCopy(cam_pos,snd_cam_pos);
		}
		//Com_Printf("Test\n");
		MVD_Cam_Event_Check(current);
	}
	if (type == 2){ // Views
		VectorCopy(pos,view_pos);

		// do the rotation
		if (current->r_start != 0 || current->r_stop != 0){
			rotation = current->r_start + ((current->r_stop - current->r_start) * time);
		}else{
			rotation = 0;
		}

		if (strlen(current->track) > 0){
			MVD_Find_Track_Player(current);
			MVD_Find_Track_Entity(current,0);
			VectorSubtract(current->track_origin[1], cam_pos, view_vec);
			VectorCopy(current->track_origin[1],view_pos);
		}else{
			VectorSubtract(view_pos, cam_pos, view_vec);
		}
		mvd_vectoangles(view_vec, cam_angles);

		if (bezier_cams_show_in_pip.value == 0 && beziers_svl.value == 0){
			VectorCopy(cam_angles,r_refdef.viewangles);
			r_refdef.viewangles[ROLL]= rotation;
			r_refdef.viewangles[0] = -r_refdef.viewangles[0];
		}else if (beziers_svl.value == 1 && bezier_cams_show_in_pip.value == 0){
			VectorCopy(view_pos,snd_cam_view);
		}else{
			VectorCopy(cam_angles,snd_cam_view);
			r_refdef.viewangles[ROLL]= rotation;
			r_refdef.viewangles[0] = -r_refdef.viewangles[0];
		}
		//MVD_Cam_Event_Check(current);
		if (glob_rot){
			r_refdef.viewangles[ROLL] = glob_rot;
			//Com_Printf("%f %f\n",glob_rot,r_refdef.viewangles[ROLL]);
		}
		

	}

}





void MVD_Set (void) {
	int number;
	int call,x,i;
	c_curve_t *current;
	char *strings[2];
	char string[32];
	strings[1]="View";
	strings[0]="Camera";

	if (!strcmp(Cmd_Argv(0),"jogi_bc_setcam"))
		call = 0;
	else
		call = 1;

	switch(Cmd_Argc()){
		case 1: Com_Printf("Use %s %snumber\n",Cmd_Argv(0),strings[call]);break;
		case 2: number=atoi(Cmd_Argv(1));
				current=call ? &views[number] : &cameras[number];
				Com_Printf("%s %i\n",strings[call],number);
				Com_Printf("Point 1        : %f %f %f\n",current->ctrlpoints[0][0],current->ctrlpoints[0][1],current->ctrlpoints[0][2]);
				Com_Printf("Point 2        : %f %f %f\n",current->ctrlpoints[1][0],current->ctrlpoints[1][1],current->ctrlpoints[1][2]);
				Com_Printf("Point 3        : %f %f %f\n",current->ctrlpoints[2][0],current->ctrlpoints[2][1],current->ctrlpoints[2][2]);
				Com_Printf("Point 4        : %f %f %f\n",current->ctrlpoints[3][0],current->ctrlpoints[3][1],current->ctrlpoints[3][2]);
				Com_Printf("Resolution     : %i\n",current->resolution);
				Com_Printf("Enable         : %i\n",current->enable);
				Com_Printf("Show           : %i\n",current->showme);
				Com_Printf("Modifier start : %f\n",current->m_start);
				Com_Printf("Modifier stop  : %f\n",current->m_stop);
				Com_Printf("Demospeed start: %f\n",current->ds_start);
				Com_Printf("Demospeed stop : %f\n",current->ds_stop);
				Com_Printf("Consolecmomands: %s\n",current->consolecommands);
				Com_Printf("Rotation Start : %f\n",current->r_start);
				Com_Printf("Rotation Stop  : %f\n",current->r_stop);
				Com_Printf("Fov Start      : %f\n",current->fov_start);
				Com_Printf("Fov Stop       : %f\n",current->fov_stop);
				Com_Printf("Start          : %f\n",current->starttime);
				Com_Printf("Stop           : %f\n",current->stoptime);
				Com_Printf("Track          : %s\n",current->track);
				break;
		default :	number=atoi(Cmd_Argv(2));
					current=call ? &views[number] : &cameras[number];
					if(!strcmp(Cmd_Argv(1),"set")){
						if(!strcmp(Cmd_Argv(3),"p1"))
							VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,current->ctrlpoints[0]);
						if(!strcmp(Cmd_Argv(3),"p2"))
							VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,current->ctrlpoints[1]);
						if(!strcmp(Cmd_Argv(3),"p3"))
							VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,current->ctrlpoints[2]);
						if(!strcmp(Cmd_Argv(3),"p4"))
							VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,current->ctrlpoints[3]);
						if(!strcmp(Cmd_Argv(3),"track_origin"))
							VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[cl.playernum].origin,current->track_origin[0]);
						if(!strcmp(Cmd_Argv(3),"enable"))
							current->enable=atoi(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"show"))
							current->showme=atoi(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"type"))
							current->type=atoi(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"modifier_start"))
							current->m_start=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"modifier_stop"))
							current->m_stop=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"rotation_start"))
							current->r_start=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"rotation_stop"))
							current->r_stop=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"demospeed_start"))
							current->ds_start=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"demospeed_stop"))
							current->ds_stop=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"fov_start"))
							current->fov_start=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"fov_stop"))
							current->fov_stop=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"dof_start"))
							current->dof_modifier_start=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"dof_stop"))
							current->dof_modifier_stop=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"consolecommands"))
							strcpy(current->consolecommands,Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"track"))
							strcpy(current->track,Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"color"))
							memcpy(current->color,StringToRGB(va("%s %s %s",Cmd_Argv(4),Cmd_Argv(5),Cmd_Argv(6))),3);
						if(!strcmp(Cmd_Argv(3),"start"))
							current->starttime=(cls.demotime - demostarttime);
						if(!strcmp(Cmd_Argv(3),"stop"))
							current->stoptime=(cls.demotime - demostarttime);
						if(!strcmp(Cmd_Argv(3),"track_modelname"))
							strcpy(current->track_modelname,Cmd_Argv(4));
						break;
					}
					if(!strcmp(Cmd_Argv(1),"seta")){
						strcpy(string,Cmd_Argv(3));
						if(string[0]=='p'){
							for (i=1;i<=20;i++){
								if (atoi(string+1) == i){
									current->ctrlpoints[i-1][0]=atof(Cmd_Argv(4));
									current->ctrlpoints[i-1][1]=atof(Cmd_Argv(5));
									current->ctrlpoints[i-1][2]=atof(Cmd_Argv(6));
								}
							}
						}
						if(!strcmp(Cmd_Argv(3),"track_origin")){
							current->track_origin[0][0]=atof(Cmd_Argv(4));
							current->track_origin[0][1]=atof(Cmd_Argv(5));
							current->track_origin[0][2]=atof(Cmd_Argv(6));
						}
						if(!strcmp(Cmd_Argv(3),"start"))
							current->starttime=atof(Cmd_Argv(4));
						if(!strcmp(Cmd_Argv(3),"stop"))
							current->stoptime=atof(Cmd_Argv(4));
					break;
					}
					if(!strcmp(Cmd_Argv(1),"sets")){
						if(!strcmp(Cmd_Argv(3),"p1")){
							current->ctrlpoints[0][0]=current->ctrlpoints[0][0] + atof(Cmd_Argv(4));
							current->ctrlpoints[0][1]=current->ctrlpoints[0][1] + atof(Cmd_Argv(5));
							current->ctrlpoints[0][2]=current->ctrlpoints[0][2] + atof(Cmd_Argv(6));
						}
						if(!strcmp(Cmd_Argv(3),"p2")){
							current->ctrlpoints[1][0]=current->ctrlpoints[1][0] + atof(Cmd_Argv(4));
							current->ctrlpoints[1][1]=current->ctrlpoints[1][1] + atof(Cmd_Argv(5));
							current->ctrlpoints[1][2]=current->ctrlpoints[1][2] + atof(Cmd_Argv(6));
						}
						if(!strcmp(Cmd_Argv(3),"p3")){
							current->ctrlpoints[2][0]=current->ctrlpoints[2][0] + atof(Cmd_Argv(4));
							current->ctrlpoints[2][1]=current->ctrlpoints[2][1] + atof(Cmd_Argv(5));
							current->ctrlpoints[2][2]=current->ctrlpoints[2][2] + atof(Cmd_Argv(6));
						}
						if(!strcmp(Cmd_Argv(3),"p4")){
							current->ctrlpoints[3][0]=current->ctrlpoints[3][0] + atof(Cmd_Argv(4));
							current->ctrlpoints[3][1]=current->ctrlpoints[3][1] + atof(Cmd_Argv(5));
							current->ctrlpoints[3][2]=current->ctrlpoints[3][2] + atof(Cmd_Argv(6));
						}
					break;
					}
					if(!strcmp(Cmd_Argv(1),"setall")){
						if (!strcmp(Cmd_Argv(2),"show")){
							if (!call){
								for (x=0;x<max_cameras && cameras[x].ctrlpoints[0][0] != 0;x++)
									cameras[x].showme = atoi(Cmd_Argv(3));
							}else{
								for (x=0;x<max_cameras && views[x].ctrlpoints[0][0] != 0;x++)
									views[x].showme = atoi(Cmd_Argv(3));
							}
						}
						if (!strcmp(Cmd_Argv(2),"enable")){
							if (!call){
								for (x=0;x<max_cameras && cameras[x].ctrlpoints[0][0] != 0;x++)
									cameras[x].enable = atoi(Cmd_Argv(3));
							}else{
								for (x=0;x<max_cameras && views[x].ctrlpoints[0][0] != 0;x++)
									views[x].enable = atoi(Cmd_Argv(3));
							}
						}
					break;
					}
					if(!strcmp(Cmd_Argv(1),"sse")){
						if (!call){
							VectorCopy(cameras[atoi(Cmd_Argv(2))].ctrlpoints[3],cameras[atoi(Cmd_Argv(3))].ctrlpoints[0]);
						}else{
							VectorCopy(views[atoi(Cmd_Argv(2))].ctrlpoints[3],views[atoi(Cmd_Argv(3))].ctrlpoints[0]);
						}
					break;
					}
					if(!strcmp(Cmd_Argv(1),"ses")){
						if (!call){
							VectorCopy(cameras[atoi(Cmd_Argv(2))].ctrlpoints[0],cameras[atoi(Cmd_Argv(3))].ctrlpoints[3]);
						}else{
							VectorCopy(views[atoi(Cmd_Argv(2))].ctrlpoints[0],views[atoi(Cmd_Argv(3))].ctrlpoints[3]);
						}
					break;
					}
					if(!strcmp(Cmd_Argv(1),"ms")){
						VectorJogi(current->ctrlpoints[3], current->ctrlpoints[0], current->ctrlpoints[1],0.3);
						VectorJogi(current->ctrlpoints[0], current->ctrlpoints[3], current->ctrlpoints[2],0.3);
					}
					break;
}
}







void MVD_Reset_demotime (void){
	cls.demotime=0;
}

void MVD_Write_Cam_Config (void){
	FILE *f;
	char *outfile;
	int x,y;
	c_event_t *current_event;
	outfile = va("%s/qw/%s.cfg", com_basedir, Cmd_Argv(1));
	f=fopen(outfile,"wb");
	for (x=0;x<max_cameras&& cameras[x].ctrlpoints[0][0] != 0;x++){
		for(y=0;y<20;y++)
			if (cameras[x].ctrlpoints[y][0] || cameras[x].ctrlpoints[y][1] || cameras[x].ctrlpoints[y][2])
				fprintf(f,"jogi_bc_setcam seta %i p%i %f %f %f\n",x,y+1,cameras[x].ctrlpoints[y][0],cameras[x].ctrlpoints[y][1],cameras[x].ctrlpoints[y][2]);
		current_event = cameras[x].events;
		while (current_event){
			fprintf(f,"cams_event %i add %f %s %f %i\n",x,current_event->time,current_event->name,current_event->value,current_event->salone);
			current_event = current_event->next;
		}
		
		fprintf(f,"jogi_bc_setcam set %i color %i %i %i\n",x,(int)cameras[x].color[0],(int)cameras[x].color[1],(int)cameras[x].color[2]);
		fprintf(f,"jogi_bc_setcam set %i show %i\n",x,cameras[x].showme);
		fprintf(f,"jogi_bc_setcam set %i modifier_start %f\n",x,cameras[x].m_start);
		fprintf(f,"jogi_bc_setcam set %i modifier_stop %f\n",x,cameras[x].m_stop);
		fprintf(f,"jogi_bc_setcam set %i enable %i\n",x,cameras[x].enable);
		fprintf(f,"jogi_bc_setcam set %i consolecommands \"%s\"\n",x,cameras[x].consolecommands);
		fprintf(f,"jogi_bc_setcam set %i demospeed_start %f\n",x,cameras[x].ds_start);
		fprintf(f,"jogi_bc_setcam set %i demospeed_stop %f\n",x,cameras[x].ds_stop);
		fprintf(f,"jogi_bc_setcam set %i fov_start %f\n",x,cameras[x].fov_start);
		fprintf(f,"jogi_bc_setcam set %i fov_stop %f\n",x,cameras[x].fov_stop);
		fprintf(f,"jogi_bc_setcam set %i track %s\n",x,cameras[x].track);
		fprintf(f,"jogi_bc_setcam set %i type %i\n",x,cameras[x].type);
		fprintf(f,"jogi_bc_setcam seta %i track_origin %f %f %f\n",x,cameras[x].track_origin[0][0],cameras[x].track_origin[0][1],cameras[x].track_origin[0][2]);
		fprintf(f,"jogi_bc_setcam seta %i start %f\n",x,cameras[x].starttime);
		fprintf(f,"jogi_bc_setcam seta %i stop %f\n",x,cameras[x].stoptime);
		fprintf(f,"jogi_bc_setcam set %i track_modelname %s\n",x,cameras[x].track_modelname);
		fprintf(f,"jogi_bc_setcam set %i dof_start %f\n",x,cameras[x].dof_modifier_start);
		fprintf(f,"jogi_bc_setcam set %i dof_stop %f\n",x,cameras[x].dof_modifier_stop);
	}
	for (x=0;x<max_cameras && views[x].ctrlpoints[0][0] != 0;x++){
		for(y=0;y<20;y++)
			if (views[x].ctrlpoints[y][0] || views[x].ctrlpoints[y][1] || views[x].ctrlpoints[y][2])
				fprintf(f,"jogi_bc_setview seta %i p%i %f %f %f\n",x,y+1,views[x].ctrlpoints[y][0],views[x].ctrlpoints[y][1],views[x].ctrlpoints[y][2]);
		/*current_event = views[x].events;
		while (current_event){
			fprintf(f,"view_event %i add %f %s %f %i\n",x,current_event->time,current_event->name,current_event->value,current_event->salone);
			current_event = current_event->next;
		}
		*/
		fprintf(f,"jogi_bc_setview set %i color %i %i %i\n",x,(int)views[x].color[0],(int)views[x].color[1],(int)views[x].color[2]);
		fprintf(f,"jogi_bc_setview set %i show %i\n",x,views[x].showme);
		fprintf(f,"jogi_bc_setview set %i enable %i\n",x,views[x].enable);
		fprintf(f,"jogi_bc_setview set %i rotation_start %f\n",x,views[x].r_start);
		fprintf(f,"jogi_bc_setview set %i rotation_stop %f\n",x,views[x].r_stop);
		fprintf(f,"jogi_bc_setview set %i track %s\n",x,views[x].track);
		fprintf(f,"jogi_bc_setview set %i type %i\n",x,views[x].type);
		fprintf(f,"jogi_bc_setview seta %i track_origin %f %f %f\n",x,views[x].track_origin[0][0],views[x].track_origin[0][1],views[x].track_origin[0][2]);
		fprintf(f,"jogi_bc_setview seta %i start %f\n",x,views[x].starttime);
		fprintf(f,"jogi_bc_setview seta %i stop %f\n",x,views[x].stoptime);
		fprintf(f,"jogi_bc_setview set %i track_modelname %s\n",x,views[x].track_modelname);
	}
	fclose(f);
	

}

// Event stuff



void MVD_Cam_Event_Exec(c_event_t *current){
	cvar_t	*variable;
	if (!strcmp("rotation",current->name)){
		Com_Printf("Static rotation\n");
		r_refdef.viewangles[ROLL] = current->value;
	}else{
		if (Cmd_Exists(current->name)){
			Cbuf_AddText(va("%s %f\n",current->name,current->value));
		} else if (variable = Cvar_Find(current->name)){
			Cvar_Set(variable,va("%f\n",current->value));
		}else {
			Com_Printf("Sorry %s isnt valid\n",current->name);
		}
	}
}

void MVD_Cam_Event_Check(c_curve_t *current){
	cvar_t		*variable;
	c_event_t	*current_event;
	c_event_t	*next_event;
	double time;
	double time2;
	double value;

	if (beziers_testtime.value)
		time = testtime;
	else
		time = cls.demotime - demostarttime;
	
	current_event = current->events;
	while (current_event){
		if (!current_event->done){
			if (((current_event->time == time || current_event->time < time) && (time - cls.frametime) < current_event->time) && current_event->salone){
				MVD_Cam_Event_Exec(current_event);
				current_event->done = 1;
			}
			if (current_event->time <= time && !current_event->salone){
				next_event = current_event->next;
				while (next_event){
					if (!strcmp(next_event->name,current_event->name)){
						if (next_event->time <= time){
							break;
						}else if (next_event->time >= time){
							time2 = (time - current_event->time)/(next_event->time - current_event->time);
							value = current_event->value + time2 *(next_event->value - current_event->value);
							//Com_Printf("%f %f %f %f %f\n",time,current_event->time,next_event->time,time2,value);
							if (!strcmp("rotation",current_event->name)){
								glob_rot = value;
							} else if (Cmd_Exists(current_event->name)){
								Cbuf_AddText(va("%s %f\n",current_event->name,value));
							} else if (variable = Cvar_Find(current_event->name)){
								Cvar_Set(variable,va("%f",value));
							}else {
								Com_Printf("Sorry %s isnt valid\n",current_event->name);
							}
							break;
						}
					}
					next_event = next_event->next;
				}
			}

		}
		current_event = current_event->next;
	}
	
}

void Update_Events(void);
void MVD_Cam_Event(void){
	int number,count,i;
	c_event_t	*current_event;
	c_event_t	*last_event;
	c_event_t	*new_event;
	c_curve_t	*current;
	float		time;
	switch(Cmd_Argc()){
		case 1: Com_Printf("Use %s number\n",Cmd_Argv(0));break;
		case 2: number=atoi(Cmd_Argv(1));
				if (!strcmp(Cmd_Argv(0),"cams_event"))
					current = &cameras[number];
				else
					current = &views[number];
				current_event = current->events;
				count = 1;

				while (current_event){
					Com_Printf("Event %i:\n",count);
					Com_Printf("Time    : %f\n",current_event->time);
					Com_Printf("Type    : %s\n",current_event->name);
					Com_Printf("value   : %f\n",current_event->value);
					count++;
					current_event = current_event->next;
				}
				break;
		case 4: number = atoi(Cmd_Argv(1));
				count = atoi(Cmd_Argv(3));
				if (!strcmp(Cmd_Argv(0),"cams_event"))
					current = &cameras[number];
				else
					current = &views[number];
				if (current->events == NULL)
					return;
				else
					current_event = current->events;

				if (count < 1)
					return;
				i = 0;
				while (current_event->next && count-1 != i++ ){
					last_event = current_event;
					current_event = current_event->next;
				}
				
				if (!strcmp("del",Cmd_Argv(2))){
					if (current_event == current->events && current_event->next){
						current->events = current_event->next;
						current_event->next->previous = NULL;
					}else if (current_event == current->events && !current_event->next){
						current->events = NULL;
					}else{
						last_event->next = current_event->next;
						current_event->previous = last_event;
					}

				Q_free(current_event);
				
				current->event_count--;
				if (current->event_count<0)
					current->event_count = 0;
				}
				break;
		case 7: number=atoi(Cmd_Argv(1));
				time = atof(Cmd_Argv(3));
				if (!strcmp(Cmd_Argv(0),"cams_event"))
					current = &cameras[number];
				else
					current = &views[number];
				current_event = current->events;
				i = 0;
				if (current->events)
					while (current_event->next){
						if ((current_event->time <= time && current_event->next->time >= time)){
							i=2;
							break;
						} 
						current_event = current_event->next;
					}
				if (current->events)
					if (( time <= current->events->time)){
						current_event = current->events;
						i = 1;
					}
				
				if (!strcmp("add",Cmd_Argv(2))){
					if (current->events == NULL){
						new_event = Q_malloc(sizeof(c_event_t));
						current->events = new_event;
					}else if (current_event == current->events && (strlen(current_event->name) == 0) && !i){
						new_event = Q_malloc(sizeof(c_event_t));
						current->events = new_event;
					}else if (current_event == current->events && i == 1) { 
						new_event = Q_malloc(sizeof(c_event_t));
						new_event->next = current_event;
						current->events = new_event;
					}else{
						new_event = Q_malloc(sizeof(c_event_t));
						if (i)
							new_event->next = current_event->next;
						current_event->next = new_event;
					}
					if (current_event)
						if (current_event->next)
								current_event->next->previous = current_event;
					new_event->time = time;
					strcpy(new_event->name,Cmd_Argv(4));
					new_event->value = atof(Cmd_Argv(5));
					new_event->salone = atof(Cmd_Argv(6));
					current->event_count++;
				}
				Update_Events();
				break;
		default :	number=atoi(Cmd_Argv(2));
	}

}


void MVD_Start_Mouse_To_View(void){
	if (!strlen(Cmd_Argv(1)) || !strlen(Cmd_Argv(2))){
		Com_Printf("We need view and cam number !\n");
		return;
	}
	mvd_mouse_to_view_cpoint = 0;
	mvd_mouse_to_view = 1;
	mvd_mouse_to_view_vnum = atoi(Cmd_Argv(1));
	mvd_mouse_to_view_cnum = atoi(Cmd_Argv(2));

}

void MVD_Cam_Events_Check_pointer(void){
	int x;
	for (x=0;x<max_cameras;x++)
		Com_Printf("%i %i\n",cameras[x].events,views[x].events);

}

void MVD_Cam_Set_Testtime(void){
	if (Cmd_Argc() != 2){
		Com_Printf("%s time\n",Cmd_Argv(0));
		return;
	}
	testtime = atoi(Cmd_Argv(1));

}

void MVD_Cam_Init(void){
	int x;
	for (x=0;x<max_cameras;x++){
		cameras[x].resolution = 40;
		cameras[x].events = Q_malloc(sizeof(c_event_t));
		views[x].events = Q_malloc(sizeof(c_event_t));
		Com_Printf("%i %i\n",cameras[x].events,views[x].events);
		views[x].resolution = 40;
		views[x].color[0] = 255;
		//cameras[x].events = NULL;
		//views[x].events = NULL;
		cameras[x].color[0] = 255;
	}
	

#ifdef GLQUAKE
	Cvar_SetCurrentGroup(CVAR_GROUP_OPENGL);
#endif
	Cvar_Register(&bezier_cams_enable);
	Cvar_Register(&bezier_cams_show_in_pip);

	Cvar_Register(&beziers_mouse_move);
	Cvar_Register(&beziers_mouse_move_modifier);
	Cvar_Register(&beziers_mouse_move_type);
	Cvar_Register(&beziers_mouse_move_number);
	Cvar_Register(&beziers_mouse_move_point);
	Cvar_Register(&beziers_mouse_move_plane);

	Cvar_Register(&beziers_testtime);

	Cvar_Register(&beziers_testtime_modifier);

	Cvar_Register(&beziers_reverse);

	Cvar_Register(&beziers_svl);

	Cvar_Register(&beziers_autodist);

	Cvar_Register(&testorito);
	Cvar_Register(&testorito1);
	Cvar_Register(&testorito2);
	Cvar_Register(&testorito3);

	Cvar_Register(&jogi_mvd_entities);

	Cvar_Register(&jogi_mvd_test1);
	Cvar_Register(&jogi_mvd_test);
	Cvar_Register(&jogi_mvd_test_x);
	Cvar_Register(&jogi_mvd_test_y);
//	Cvar_Register(&jogi_mvd_test_size_x);
//	Cvar_Register(&jogi_mvd_test_size_y);
	Cvar_Register(&jogi_mvd_test_a);
	Cvar_Register(&jogi_mvd_test_b);
	Cvar_Register(&jogi_mvd_test_scale);
	Cvar_Register(&jogi_mvd_track);

	Cvar_Register(&beziers_color_ca);
	Cvar_Register(&beziers_color_cd);
	Cvar_Register(&beziers_color_va);
	Cvar_Register(&beziers_color_vd);
	Cvar_Register(&beziers_color_vl);

	Cvar_Register(&event_color);

	Cvar_Register(&beziers_mouse_to_view_interval);

	Cvar_Register(&rot_x);
	Cvar_Register(&rot_y);
	Cvar_Register(&rot_z);
	Cvar_Register(&rot_switch);

	Cmd_AddCommand("jogi_bc_setview",MVD_Set);
	Cmd_AddCommand("jogi_bc_setcam",MVD_Set);
	Cmd_AddCommand("jogi_bc_writeconfig",MVD_Write_Cam_Config);
	Cmd_AddCommand("beziers_add_static_cam",MVD_Create_Static_Cam);
	Cmd_AddCommand("cams_event",MVD_Cam_Event);
	Cmd_AddCommand("view_event",MVD_Cam_Event);
	Cmd_AddCommand("beziers_start_movement_to_curve",MVD_Start_Mouse_To_View);
	Cmd_AddCommand("jogi_test_pointer",MVD_Cam_Events_Check_pointer);
	Cmd_AddCommand("beziers_testtime_set",MVD_Cam_Set_Testtime);

	inited = 1;
	Com_Printf("MVD Camera System Loaded\n");


}

void MVD_Testtime_Hack(void){
	if (tt_m != last_tt_m){
		last_tt_m = tt_m;
		testtime = cls.demotime + tt_m;
	}else{
		tt_m = testtime - cls.demotime;
	}
}

void MVD_Mouse_To_View_Frame(void);
void MVD_Cam_Main_Hook (void){
	MVD_Testtime_Hack();
	MVD_Autodist();
	MVD_Mouse_To_View_Frame();
}

//GLfloat knots[8] = {

void MVD_Cam_Test(c_curve_t *current){
	if (testorito.value == 0 )
		return;
	if (framenumb == -1 ){
		framenumb = GL_LoadTextureImage("textures/flat/lg_test3.png","lg_test_3",0,0,1);
		Com_Printf("%i\n",framenumb);
	}

	GL_Bind(framenumb);

}

//float ctrlpoints[8][3] = {{0,0,0},{100,50,10},{200,0,100},{400,200,100}
//,{450,350,400},{450,500,550},{550,650,600},{700,700,700}};

void MVD_Curve_Test(void){
//	int i;
	/*
	glMap1f(GL_MAP1_VERTEX_3,0.0,1.0,5,8,&ctrlpoints[0][0]);
	glEnable(GL_MAP1_VERTEX_3);
	glColor3f(1,0,0);
	glLineWidth(3);
	glBegin(GL_LINE_STRIP);
	for(i=0;i<=100;i++)
		glEvalCoord1f((GLfloat)i/(GLfloat)100);
	glEnd();

	glPointSize(5.0);
	glBegin(GL_POINTS);
	for(i=0;i<8;i++)
		glVertex3fv(&ctrlpoints[i][0]);
	glEnd();
	glFlush();
	
	if (framenumb == -1)
		framenumb = GL_LoadTextureImage("textures/flat/bz_ca.png","bz_ca",0,0,1);

	glMap1f(GL_MAP1_VERTEX_3,0.0,1.0,5,8,&ctrlpoints[0][0]);
	glEnable(GL_MAP1_VERTEX_3);
	glMap1f(GL_MAP1_TEXTURE_COORD_2,0.0,1.0,5,8,&ctrlpoints[0][0]);
	glEnable(GL_MAP1_TEXTURE_COORD_2);
	glMapGrid1f(40, 0.0, 1.0);
	glEvalMesh1(GL_LINE, 0, 40);


glMap2f(GL_MAP2_VERTEX_3, 0.0, 1.0, 3, 2, 0.0, 1.0, 6, 2,tele_cams[0].setpoints[0]);

	glMap2f(GL_MAP2_TEXTURE_COORD_2, 0.0, 1.0, 2, 2, 0.0, 1.0, tele_cams[0].test, tele_cams[0].test1, texpoints[0]);
	glEnable(GL_MAP2_TEXTURE_COORD_2);
	GL_Bind(framenumb);
	glEnable(GL_MAP2_VERTEX_3);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glMapGrid2f(40, 0.0, 1.0, 40, 0.0, 1.0);
	glEvalMesh2(GL_FILL, 0, 40, 0, 40);
	*/
}

void avi_draw_plane(void);
void 	MVD_Cam_Render_Beziers(void){
	int x;
	MVD_Curve_Test();
	MVD_Draw_LA();
	MVD_Cam_PEL();
	MVD_Cam_ETL();
			for (x=0;x<max_cameras;x++){
				if (cameras[x].showme){
					MVD_Cam_Draw_Bezier(&cameras[x],x,0);
				}
			}
			for (x=0;x<max_cameras;x++){
				if (views[x].showme){
					MVD_Cam_Draw_Bezier(&views[x],x,1);
				}
			}
	MVD_Cam_SVL();
	MVD_Show_Active_Event();
}


void MVD_Cam_2D_Hook(void){
	MVD_Cam_Draw_Number();
}

void MVD_Cam_Render_Hook(void){
	int x;
	double blah, blah2;
	extern double demostarttime;
	char args [1024];
	int end;
	extern double mouse_x, mouse_y;
	extern int selected_view,selected_cam;
	//Com_Printf("%f %f\n",mouse_x,mouse_y);
	MVD_Mouse_Move_Points();
	if (beziers_testtime.value){
		testtime += mouse_x/beziers_testtime_modifier.value;
		if (testtime < demostarttime)
			testtime = demostarttime;
	}else{
		testtime = cls.demotime;
	}
	if (beziers_reverse.value){
		if (!cam_stop_time){
			cam_start_time = 1000;
			for (x=0;x<max_cameras && cameras[x].enable;x++){
				if (cam_stop_time < cameras[x].stoptime)
					cam_stop_time = cameras[x].stoptime;
				if (cam_start_time > cameras[x].starttime)
					cam_start_time = cameras[x].starttime;
			}
			testtime = demostarttime+ cam_stop_time;
			//Com_Printf("%f %f\n",cam_start_time,cam_stop_time);
			testtime1 = cam_stop_time;
		}
		if ((cls.demotime -demostarttime) >= cam_start_time && (cls.demotime -demostarttime) <= cam_stop_time){
			testtime1 -= cls.frametime;
			testtime = demostarttime + testtime1 ;
		}

	}
	if (bezier_cams_enable.value){
		end = 0;
		for (x=0;x<max_cameras && end == 0;x++){
			if (cameras[x].enable){
			blah  = (testtime - demostarttime) - cameras[x].starttime;
			blah2 = (testtime - demostarttime) - cameras[x].stoptime;
			//Com_Printf("Testtime: %f\nDemostarttime: %f\nblah: %f\nblah2: %f\n", testtime, demostarttime, blah, blah2);
				if ( blah >= 0 && blah2 <=0){
					if (x != cam_in_progress){
						if (!beziers_testtime.value){
							sprintf(args,"%s\n",cameras[x].consolecommands);
							Cbuf_AddText(args);
							cameras[x].real_starttime = testtime - demostarttime;
							cam_in_progress=x;
						}else{
							cameras[cam_in_progress].showme=0;
							cameras[x].showme=1;
							cameras[x].real_starttime = cameras[x].starttime;
							cam_in_progress=x;
							selected_cam=x;
						}
						if (beziers_reverse.value)
							cameras[x].real_starttime = cameras[x].starttime;
						
					}
				MVD_Move_Me(&cameras[x],1);
				end = 1;
				}
			}
		}
		end = 0;
		for (x=0;x<max_cameras && end == 0;x++){
			if (views[x].enable){
			blah  = (testtime - demostarttime) - views[x].starttime;
			blah2 = (testtime - demostarttime) - views[x].stoptime;
				if ( blah >= 0 && blah2 <=0){
					if (x != view_in_progress){
						if (!beziers_testtime.value){
							views[x].real_starttime = testtime - demostarttime;
							view_in_progress=x;
						}else{
							views[view_in_progress].showme=0;
							views[x].showme =1;
							views[x].real_starttime = views[x].starttime;
							view_in_progress=x;
							selected_view=x;
						}
						if (beziers_reverse.value)
							views[x].real_starttime = views[x].starttime;
					}
				MVD_Move_Me(&views[x],2);
				end = 1;
				}
			}
		}
	}
	
}



void MVD_Find_Track_Entity (c_curve_t *current,int dontshow) {
	float	distance;
	float	newdistance;
	extern visentlist_t cl_visents;
	int i;
	int x = -1;

	if (strcmp(current->track,"entity"))
		return;
	VectorCopy(current->track_origin[0],current->track_origin[1]);
	distance = -1;
	for (i=0;i<cl_visents.count;i++){
		
		if (!strcmp(current->track_modelname,cl_visents.list[i].model->name)){
			newdistance = sqrt(pow((cl_visents.list[i].origin[0] - current->track_origin[1][0]),2) + pow((cl_visents.list[i].origin[1] - current->track_origin[1][1]),2) + pow((cl_visents.list[i].origin[2] - current->track_origin[1][2]),2));
			if (distance > newdistance || distance == -1){
				distance = newdistance ;
				x=i;
				VectorCopy(cl_visents.list[i].origin,current->track_origin[2]);
			}
		}
	}
	if (dontshow && x != -1)
		cl_visents.list[x].dontrender = 1;
	VectorCopy(current->track_origin[2],current->track_origin[1]);
	
}

void MVD_Find_Track_Player (c_curve_t *current) {
	int i;

	if (!strcmp(current->track,"entity"))
		return;

	for (i=0;i<MAX_CLIENTS;i++)
		if (!cl.players[i].spectator && cl.players[i].name[0])
			if (!strcmp(current->track,cl.players[i].name)){
				VectorCopy(cl.frames[cl.validsequence & UPDATE_MASK].playerstate[i].origin,current->track_origin[1]);
			}

}

/*
float MVD_TraceLine (const vec3_t start, const vec3_t end, vec3_t impact, vec3_t normal, int contents, int hitbmodels, entity_t *hitent)
{
	float maxfrac;
	int n;
	entity_t *ent;
	float tracemins[3], tracemaxs[3];
	ctrace_t trace;
	

	if (hitent)
		hitent = NULL;
	//Mod_CheckLoaded(cl.worldmodel);
	Collision_ClipTrace(&trace, NULL, cl.worldmodel, vec3_origin, vec3_origin, vec3_origin, vec3_origin, start, vec3_origin, vec3_origin, end);
	
	if (impact)
		VectorCopy (trace.endpos, impact);
	if (normal)
		VectorCopy (trace.plane.normal, normal);
	//cl_traceline_endcontents = trace.endcontents;
	maxfrac = trace.fraction;
	if (hitent && trace.fraction < 1)
		hitent = &cl_visents.list[0];

	if (hitbmodels) {
		// look for embedded bmodels
		tracemins[0] = min(start[0], end[0]);
		tracemaxs[0] = max(start[0], end[0]);
		tracemins[1] = min(start[1], end[1]);
		tracemaxs[1] = max(start[1], end[1]);
		tracemins[2] = min(start[2], end[2]);
		tracemaxs[2] = max(start[2], end[2]);

		for (n = 0;n < cl_visents.count;n++) {
			//Com_Printf("%s %i\n",cl_visents.list[n].model->name,cl_visents.list[n].model->type);
			if (cl_visents.list[n].model->type != mod_brush)
				continue;
			ent = &cl_visents.list[n];
			Collision_ClipTrace(&trace, ent, ent->model, ent->origin, ent->angles, ent->model->mins, ent->model->maxs, start, vec3_origin, vec3_origin, end);

			if (trace.allsolid || trace.startsolid || trace.fraction < maxfrac) {
				maxfrac = trace.fraction;
				if (impact)
					VectorCopy(trace.endpos, impact);
				if (normal)
					VectorCopy(trace.plane.normal, normal);
				//cl_traceline_endcontents = trace.endcontents;
				if (hitent){
					hitent = ent;
					Com_Printf("We hit smth !\n");
				}
			}
		}
	}
	if (maxfrac < 0 || maxfrac > 1) Com_Printf("fraction out of bounds %f %s:%d\n", maxfrac, __LINE__, __FILE__);
		return maxfrac;
}
*/

void MVD_Create_Static_Cam(void){
	extern float distance;
	vec3_t impact, normal, dest, forward, right, up, origin;
	int i;
	entity_t	test;
	trace_t		trace;

	if (!strlen(Cmd_Argv(1)) || !strlen(Cmd_Argv(2)) || !strlen(Cmd_Argv(3))){
		Com_Printf("We need cameranumber starttime stoptime\n");
		return;
	}
			
	AngleVectors (r_refdef.viewangles,forward, right, up);
	VectorCopy(r_refdef.vieworg,origin);
			for (i=0;i<3;i++)
				dest[i] = r_refdef.vieworg[i] + forward[i] * 10000;
	
	trace = PM_TraceLine(r_refdef.vieworg, dest);
	//MVD_TraceLine (r_refdef.vieworg, dest, impact, normal, 0, 1, &test);
	
	for (i=0;i<4;i++)
		VectorCopy(r_refdef.vieworg,cameras[atoi(Cmd_Argv(1))].ctrlpoints[i]);
	for (i=0;i<4;i++)
		VectorCopy(trace.endpos,views[atoi(Cmd_Argv(1))].ctrlpoints[i]);

	cameras[atoi(Cmd_Argv(1))].starttime = atof(Cmd_Argv(2));
	views[atoi(Cmd_Argv(1))].starttime = atof(Cmd_Argv(2));
	cameras[atoi(Cmd_Argv(1))].stoptime = atof(Cmd_Argv(3));
	views[atoi(Cmd_Argv(1))].stoptime = atof(Cmd_Argv(3));


	//VectorCopy(dest,lookat_pos);
}

void MVD_Mouse_To_View_Frame(void){
	vec3_t impact, normal, dest, forward, right, up, origin;
	entity_t	test;
	int i;
	
	if (!mvd_mouse_to_view)
		return;
	if ((mvd_mouse_to_view_time + beziers_mouse_to_view_interval.value) < Sys_DoubleTime())
		mvd_mouse_to_view_time = Sys_DoubleTime();
	else 
		return;

	
	AngleVectors (r_refdef.viewangles,forward, right, up);

	for (i=0;i<4;i++)
		views[mvd_mouse_to_view_vnum].ctrlpoints[mvd_mouse_to_view_cpoint][i] = r_refdef.vieworg[i] + forward[i] * 8;
	VectorCopy(r_refdef.vieworg,cameras[mvd_mouse_to_view_cnum].ctrlpoints[mvd_mouse_to_view_cpoint]);
	mvd_mouse_to_view_cpoint++;
	//VectorCopy(dest,views[mvd_mouse_to_view_vnum].ctrlpoints[mvd_mouse_to_view_cpoint++]);
	if (mvd_mouse_to_view_cpoint >= 20){
		mvd_mouse_to_view = 0;
		Com_Printf("Recording ended\n");
		views[mvd_mouse_to_view_vnum].type = 1;
		cameras[mvd_mouse_to_view_vnum].type = 1;
	}


}

void MVD_Show_Active_Event(void){
	extern c_curve_t *current_cam;
	vec3_t	point;
	int j;
	double t,t1,t2;
	extern int *cam_event_num;
	extern double *cam_event_time;
	if (current_cam == NULL)
		return;
	if (!current_cam->showme)
		return;
	//Com_Printf("%f %f\n",*cam_event_time,cam_event_time);
	if (cam_event_time)
		t = (*cam_event_time - current_cam->starttime) / (current_cam->stoptime - current_cam->starttime);
	else
		t = (0 - current_cam->starttime) / (current_cam->stoptime - current_cam->starttime);
	if (current_cam->type == 0)
		MVD_Calc_Bezier_new(&current_cam->ctrlpoints[0][0],t,point);
	
	if (current_cam->type == 1){
		t1 = (double) 1 / (double)current_cam->catmull_points;
		for (j=0,t2=0;t2<=t && j <=current_cam->catmull_points ;t2+=t1){
			j++;
		}
		j--;
			MVD_Calc_Catmull_Rom(&current_cam->ctrlpoints[j][0],(t - (t2-t1))/t1,point);
	}
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_POINTS);
		glColor3ubv(StringToRGB(event_color.string));
		glVertex3fv(point);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}



