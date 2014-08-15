

#define max_cameras 40
#define max_mvd_cams 4
#define max_mvd_resolutions 4
#define max_submenus	64

#define VectorJogi(a,b,c,d)	((c)[0] = ((a)[0] - (b)[0])*d+(b)[0], (c)[1] = ((a)[1] - (b)[1])*d+(b)[1], (c)[2] = ((a)[2] - (b)[2])*d+(b)[2])



// Curve stuff

typedef struct c_event_s {
	struct	c_event_s *next;
	struct	c_event_s *previous;
	double	value;
	char	name[32];
	double	time;
	int		done;
	int		salone;
} c_event_t ;

typedef struct c_curve_s {
	int showme ;
	int enable;
	vec3_t ctrlpoints[20];
	struct c_event_s *events;
	double real_starttime;
	double starttime;
	double stoptime;
	double m_start;
	double m_stop;
	double ds_start;
	double ds_stop;
	double r_start;
	double r_stop;
	double fov_start;
	double fov_stop;
	double dof_modifier_start;
	double dof_modifier_stop;
	char track[32];
	char track_modelname[64];
	vec3_t track_origin[3];
	char consolecommands[1024];
	int resolution;
	byte color[3];
	double length;
	int		type;
	int		event_count;
	int		catmull_points;
} c_curve_t;
// type
// 0 = normal spline
// 1 = catmull-rom spline 

// Teleporter planes
typedef struct tele_plane_s {
	vec3_t		setpoints[4];
	vec3_t		meshpoints[17];
	int			texnum;
	int			enable;
	int			test;
	int			test1;
} tele_plane_t;





typedef struct mvd_menu_s {
	char					name[64];
	char					identifier[32];
	char					command[32];
	int						type;
	int						sub_active;
	int						active;
	int						changepname;
	int						maxmenu;
	double						max;
	double						min;
	double						**modifier;
	double						restrict;
	cvar_t					*variable;
	int						**intptr;
	double					**floatptr;
	char					**charptr;
	vec_t					**vecptr;
	struct		mvd_menu_s	*menu[max_submenus];
	void			(*function)(void);
	struct		mvd_menu_s	*parent;
	int			width;
} mvd_menu_t;






void MVD_Cam_Main_Hook (void);
void MVD_Display_Frames(void);
void MVD_Display_Draw(void);
/*
void MVD_Create_Framebuffers(mvd_cams_t *current);
void MVD_Delete_Framebuffers(mvd_cams_t *current);
void MVD_Enable_Framebuffers (mvd_cams_t *current);
void MVD_Disable_Framebuffers(mvd_cams_t *current);
void MVD_Init_mvd_picture(mvd_cams_t *current);
*/
void MVD_Load_Framebuffer_Functions (void);
int MVD_Track_f(char *string);
void mvd_vectoangles(vec3_t vec, vec3_t ang);
void MOSDM_Main(void);
void MOSDM_Init(void);
void MOSDM_Screen_Hook (void);
void MVD_Find_Track_Player (c_curve_t *current);
void MVD_Find_Track_Entity (c_curve_t *current,int dontshow);
void MOSDM_Get_Player_List(void);
void MOSDM_Change_View(void);
void MVD_Cam_Test(c_curve_t *current);
qbool MOSDM_Onchange_Modifier(cvar_t *current, char *string );
void MVD_Cam_Render_Beziers(void);
void MVD_Calc_Length (c_curve_t *current);
float MVD_TraceLine (const vec3_t start, const vec3_t end, vec3_t impact, vec3_t normal, int contents, int hitbmodels, entity_t *hitent);
void MOSDM_New_Move_Menu(int dir);
void MVD_Create_Static_Cam(void);
void MVD_Show_Active_Event(void);

