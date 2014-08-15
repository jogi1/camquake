#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "q_shared.h"

#ifdef WIN32
#include "winquake.h"
#else
#include <dirent.h>
#include <time.h>
#endif
#include "quakedef.h"

#include "sys_lib.h"

#include "lua.h"
#include "lauxlib.h"

#include "new_menu.h"
#include "splines.h"
#include "cam_new.h"

#define FUNCTIONS_NAME "Quake"

struct lua_double
{
	double *ptr;
	double value;
};

struct lua_int
{
	int *ptr;
	int value;
};

/*
 * Dynamic loading
 */

#define qwlua_getglobal(L,s)	qwlua_getfield(L, LUA_GLOBALSINDEX, (s))
#define qwlua_tostring(L,i)	qwlua_tolstring(L, (i), NULL)
#define qwlua_open()		qwlua_newstate()
#define qwlua_dofile(L, fn) \
        (qwlua_loadfile(L, fn) || qwlua_pcall(L, 0, LUA_MULTRET, 0))
#define qwlua_pushliteral(L, s)   \
        qwlua_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)
#define qwlua_pop(L,n)            qwlua_settop(L, -(n)-1)
#define qwlua_checkstring(L,n)   (qwlua_checklstring(L, (n), NULL))
#define qwlua_getglobal(L,s)      qwlua_getfield(L, LUA_GLOBALSINDEX, (s))
#define qwlua_setglobal(L,s)      qwlua_setfield(L, LUA_GLOBALSINDEX, (s))

void		(*qwlua_close)		(lua_State *L);
void		(*qwlua_getfield)	(lua_State *L, int idx, const char *k);
void		(*qwlua_setfield)	(lua_State *L, int idx, const char *k);
void		(*qwlua_pushnumber)	(lua_State *L, lua_Number n);
int		(*qwlua_pcall)		(lua_State *L, int nargs, int nresults, int errfunc);
const char *	(*qwlua_tolstring)	(lua_State *L, int idx, size_t *len);
void		(*qwlua_pushstring)	(lua_State *L, const char *s);
void		(*qwlua_pushlstring)	(lua_State *L, const char *s, size_t l);
lua_State *	(*qwlua_newstate)	(void);
void		(*qwlua_openlibs)	(lua_State *L);
int		(*qwlua_loadfile)	(lua_State *L, const char *filename);
int		(*qwlua_newmetatable)	(lua_State *L, const char *tname);
void		(*qwlua_openlib)	(lua_State *L, const char *libname, const luaL_Reg *l, int nup);
void		(*qwlua_pushvalue)	(lua_State *L, int idx);
void		(*qwlua_rawset)		(lua_State *L, int idx);
void		(*qwlua_settop)		(lua_State *L, int idx);
const char *	(*qwlua_checklstring)	(lua_State *L, int numArg, size_t *l);
int		(*qwlua_checkint)	(lua_State *L, int narg);
void		(*qwlua_pushnil)	(lua_State *L);
void		(*qwlua_getfield)	(lua_State *L, int idx, const char *k);
int		(*qwlua_next)		(lua_State *L, int idx);
int             (*qwlua_type)		(lua_State *L, int idx);
const char *	(*qwlua_typename)	(lua_State *L, int tp);
void		(*qwlua_pushlightuserdata) (lua_State *L, void *p);
void *		(*qwlua_touserdata)	(lua_State *L, int idx);
lua_Number	(*qwlua_tonumber)	(lua_State *l, int idx);
void		(*qwlua_concat)		(lua_State *L, int n);
void		(*qwlua_settable)	(lua_State *L, int idx);

struct lib_functions lua_functions[] =
	{
	{	"lua_close",		(void *)&qwlua_close },
	{	"lua_getfield",		(void *)&qwlua_getfield },
	{	"lua_pushnumber",	(void *)&qwlua_pushnumber },
	{	"lua_pcall",		(void *)&qwlua_pcall },
	{	"lua_tolstring",	(void *)&qwlua_tolstring },
	{	"lua_pushstring",	(void *)&qwlua_pushstring },
	{	"lua_pushlstring",	(void *)&qwlua_pushlstring },
	{	"luaL_newstate",	(void *)&qwlua_newstate },
	{	"luaL_openlibs",	(void *)&qwlua_openlibs },
	{	"luaL_loadfile",	(void *)&qwlua_loadfile },
	{	"luaL_newmetatable",	(void *)&qwlua_newmetatable },
	{	"luaL_openlib",		(void *)&qwlua_openlib },
	{	"luaL_checklstring",	(void *)&qwlua_checklstring},
	{	"lua_pushvalue",	(void *)&qwlua_pushvalue},
	{	"lua_rawset",		(void *)&qwlua_rawset},
	{	"lua_settop",		(void *)&qwlua_settop},
	{	"luaL_checkinteger",	(void *)&qwlua_checkint},
	{	"lua_pushnil",		(void *)&qwlua_pushnil},
	{	"lua_getfield",		(void *)&qwlua_getfield},
	{	"lua_setfield",		(void *)&qwlua_setfield},
	{	"lua_next",		(void *)&qwlua_next},
	{	"lua_typename",		(void *)&qwlua_typename},
	{	"lua_type",		(void *)&qwlua_type},
	{	"lua_pushlightuserdata",(void *)&qwlua_pushlightuserdata},
	{	"lua_touserdata",	(void *)&qwlua_touserdata},
	{	"lua_tonumber",		(void *)&qwlua_tonumber},
	{	"lua_concat",		(void *)&qwlua_concat},
	{	"lua_settable",		(void *)&qwlua_settable},
	{NULL}
	};




//void Register_Functions(char *libname, lua_State *L, luaL_reg *methods, luaL_reg *meta);

struct L_lua_states
{
	char *name;
	lua_State *L;
	int buggy;
	struct L_lua_states *next, *prev;
};

struct L_lua_states *L_lua_states;

void Clear_Lua_State(struct L_lua_states *ls)
{
	if (!ls)
		return;

	if (ls->name)
		free(ls->name);
	if (ls->L)
		qwlua_close(ls->L);
}

struct L_lua_states *Find_Lua_State(char *name)
{
	struct L_lua_states *ls;

	ls = L_lua_states;

	while (ls)
	{
		if (!strcmp(ls->name, name))
			return ls;
		ls = ls->next;
	}

	return NULL;
}

void Remove_Lua_State(char *name)
{
	struct L_lua_states *ls;
	if(!name)
		return;

	ls = Find_Lua_State(name);
	if (!ls)
		return;

	if (ls->prev && ls->next)
	{
		ls->prev->next = ls->next;
		ls->next->prev = ls->prev;
		Clear_Lua_State(ls);
		return;
	}

	if (!ls->prev)
	{
		if (ls->next)
		{
			L_lua_states = ls->next;
			ls->next->prev = NULL;
		}
		else
		{
			L_lua_states = NULL;
		}
		Clear_Lua_State(ls);
	}


}

struct L_lua_states *Add_Lua_State(void)
{
	struct L_lua_states *ls;

	if (L_lua_states == NULL)
	{
		L_lua_states = calloc(1, sizeof(struct L_lua_states));
		if (L_lua_states == NULL) 
		{
			return NULL;
		}
		ls = L_lua_states;
	}
	else
	{
		ls = L_lua_states;
		while (ls->next)
			ls = ls->next;
		ls->next = calloc(1, sizeof(struct L_lua_states));
		if (ls->next == NULL)
		{
			return NULL;
		}
		ls->next->prev = ls;
		ls = ls->next;
	}

	return ls;
}

void Run_Script_In_Lua_State(char *name, char *script)
{
}

static char *check_null(char *string)
{
	if (strcmp(string, "NULL") == 0)
		return NULL;
	else
		return string;
}

/*
 * lib functins
 */

static int LF_Com_Printf(lua_State *L)
{
	const char *string;

	string = qwlua_checkstring(L, 2);
	if (string)
		Com_Printf("%s", string);
	return 0;
}

static int LF_Get_Position(lua_State *L)
{
	
	qwlua_pushnumber(L, r_refdef.vieworg[0]);
	qwlua_pushnumber(L, r_refdef.vieworg[1]);
	qwlua_pushnumber(L, r_refdef.vieworg[2]);

	return 3;
}

/*
 * taken from an iccolus.org email
 */
void LUA_RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point,
float degrees )
{
   float q[3];
   float q3;
   float t[3];
   float t3;

   {
      float hrad;
      float s;

      hrad = DEG2RAD(degrees) / 2;
      s = sin(hrad);
      VectorScale(dir, s, q);
      q3 = cos(hrad);
   }

   CrossProduct(q, point, t);
   VectorMA(t, q3, point, t);
   t3 = DotProduct(q, point);

   CrossProduct(q, t, dst);
   VectorMA(dst, t3, q, dst);
   VectorMA(dst, q3, t, dst);
}

void RPAV_NEW( vec3_t dst, const vec3_t dir,
                              const vec3_t point, float degrees ) {
	vec_t sind, cosd, expr;
	vec3_t dxp;

	degrees = DEG2RAD( degrees );
	sind = sin( degrees );
	cosd = cos( degrees );
	expr = ( 1 - cosd ) * DotProduct( dir, point );
	CrossProduct( dir, point, dxp );

	dst[0] = expr*dir[0] + cosd*point[0] + sind*dxp[0];
	dst[1] = expr*dir[1] + cosd*point[1] + sind*dxp[1];
	dst[2] = expr*dir[2] + cosd*point[2] + sind*dxp[2];
}

void Rotate_Point_Around_Line(vec3_t out, vec3_t point, vec3_t seg1, vec3_t seg2, double angle)
{
	vec3_t	r;
	double cosa, sina;

	angle = DEG2RAD(angle);

	r[0] = seg2[0] - seg1[0];
	r[1] = seg2[1] - seg1[1];
	r[2] = seg2[2] - seg1[2];

	point[0] -= seg1[0];
	point[1] -= seg1[1];
	point[2] -= seg1[2];

	VectorNormalize(r);

	cosa = cos(angle);
	sina = sin(angle);

	out[0] = out[1] = out[2] = 0;

	out[0] += (cosa + (1 - cosa) * r[0] * r[0]) * point[0];
	out[0] += ((1 - cosa) * r[0] * r[1] - r[2] * sina) * point[1];
	out[0] += ((1 - cosa) * r[0] * r[2] + r[1] * sina) * point[2];

	out[1] += ((1 - cosa) * r[0] * r[1] + r[2] * sina) * point[0];
	out[1] += (cosa + (1 - cosa) * r[1] * r[1]) * point[1];
	out[1] += ((1 - cosa) * r[1] * r[2] - r[0] * sina) * point[2];

	out[2] += ((1 - cosa) * r[0] * r[2] - r[1] * sina) * point[0];
	out[2] += ((1 - cosa) * r[1] * r[2] + r[0] * sina) * point[1];
	out[2] += (cosa + (1 - cosa) * r[2] * r[2]) * point[2];

	out[0] += seg1[0];
	out[1] += seg1[1];
	out[2] += seg1[2];
}

static int LF_Rotate_Point_Around_Line(lua_State *L)
{
	vec3_t rpoint, point1, point2, dst;
	float degrees;

	rpoint[0] = qwlua_tonumber(L, 2);
	rpoint[1] = qwlua_tonumber(L, 3);
	rpoint[2] = qwlua_tonumber(L, 4);

	point1[0] = qwlua_tonumber(L, 5);
	point1[1] = qwlua_tonumber(L, 6);
	point1[2] = qwlua_tonumber(L, 7);

	point2[0] = qwlua_tonumber(L, 8);
	point2[1] = qwlua_tonumber(L, 9);
	point2[2] = qwlua_tonumber(L, 10);

	degrees = qwlua_tonumber(L, 11);

	Rotate_Point_Around_Line(dst, rpoint, point1, point2, degrees);

	qwlua_pushnumber(L, dst[0]);
	qwlua_pushnumber(L, dst[1]);
	qwlua_pushnumber(L, dst[2]);

	return 3;
}

static int LF_Rotate_Point_Around_Vector(lua_State *L)
{
	vec3_t dir, point, dst;
	float degrees;

	dir[0] = qwlua_tonumber(L, 2);
	dir[1] = qwlua_tonumber(L, 3);
	dir[2] = qwlua_tonumber(L, 4);

	point[0] = qwlua_tonumber(L, 5);
	point[1] = qwlua_tonumber(L, 6);
	point[2] = qwlua_tonumber(L, 7);

	Com_Printf("%f %f %f %f %f %f\n", dir[0], dir[1], dir[2], point[0], point[1], point[2]);

	degrees = qwlua_tonumber(L, 8);

	VectorNormalize(dir);
	RPAV_NEW(dst, dir, point, degrees);

	Com_Printf("%f %f %f\n", dst[0], dst[1], dst[2]);

	qwlua_pushnumber(L, dst[0]);
	qwlua_pushnumber(L, dst[1]);
	qwlua_pushnumber(L, dst[2]);

	return 3;
}


static int LF_Menu_Add_To_Mainmenu(lua_State *L)
{
	extern struct MS_Popup_Menu_Menu MS_Popup_Menu_Root;

	const char *name, *identifier;
	char buf[512];
	int lx, ly, rx, ry;

	lx = qwlua_checkint(L, 2);
	ly = qwlua_checkint(L, 3);
	rx = qwlua_checkint(L, 4);
	ry = qwlua_checkint(L, 5);
	name = qwlua_checkstring(L, 6);
	identifier = qwlua_checkstring(L, 7);

	snprintf(buf, sizeof(buf), "MS_Activate_Menu %s", identifier);

	MS_Popup_Add_Entry(&MS_Popup_Menu_Root, NULL, (char *)name, NULL, NULL, buf); 

	if (MS_Add_Menu(lx, ly, rx, ry, (char *)name, (char *)identifier, NULL) == NULL)
	{
		Com_Printf("menu_add_to_mainmenu error creating menu!\n");
	}

	return 0;
}

void LUA_Update_Text_Function(struct menu_ct *self)
{
	char buf[1024];
	lua_State *L;

	L = MS_Get_Content_Lua_State(self);

	if (!L)
		return;

	snprintf(buf, sizeof(buf), "%s_update", MS_Get_Content_Identifier(self));
	qwlua_getglobal(L, buf);
	qwlua_pushlightuserdata(L, self);
	qwlua_pcall(L, 1, 0, 0);


}

static int LF_Menu_Add_Content_Text (lua_State *L)
{

	const char *menu_id, *parent_id, *id, *text;
	int align;
	struct menu_ct *ct;

	menu_id = qwlua_checkstring(L, 2);
	parent_id = qwlua_checkstring(L, 3);
	parent_id = (const char *)check_null((char *)parent_id);
	id = qwlua_checkstring(L, 4);
	align = qwlua_checkint(L, 5);
	text = qwlua_checkstring(L, 6);

	ct = MS_Add_Content_Text((char *)menu_id, (char *)parent_id, (char *)id, align, (char *)text);
	MS_Set_Content_Lua_State(ct, L);

	return 0;
}

static int LF_Menu_Add_Content_Text_Update_Function (lua_State *L)
{
	struct menu_ct *ct;
	const char *menu_id, *parent_id, *id, *text;
	int align;

	menu_id = qwlua_checkstring(L, 2);
	parent_id = qwlua_checkstring(L, 3);
	parent_id = (const char *)check_null((char *)parent_id);
	id = qwlua_checkstring(L, 4);
	align = qwlua_checkint(L, 5);
	text = qwlua_checkstring(L, 6);

	ct = MS_Add_Content((char *)menu_id, (char *)parent_id, (char *)id, 1, 1, align, (char *)text, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, NULL, NULL, &LUA_Update_Text_Function);
	if (ct == NULL)
	{
		Com_Printf("LF_Menu_Add_Content_Text_Update_Function error!\n");
	}
	else
	{
		MS_Set_Content_Lua_State(ct, L);
	}

	return 0;
}

void LUA_Button_Function(struct menu_ct *self)
{
	char buf[1024];
	lua_State *L;

	L = MS_Get_Content_Lua_State(self);

	if (!L)
		return;

	snprintf(buf, sizeof(buf), "%s_button", MS_Get_Content_Identifier(self));
	qwlua_getglobal(L, buf);
	qwlua_pushlightuserdata(L, self);
	qwlua_pcall(L, 1, 0, 0);
}

static int LF_Menu_Add_Content_Button_Color (lua_State *L)
{
	struct menu_ct *ct;
	const char *menu_id, *parent_id, *id, *text, *color;
	int align;

	menu_id = qwlua_checkstring(L, 2);
	parent_id = qwlua_checkstring(L, 3);
	parent_id = (const char *)check_null((char *)parent_id);
	id = qwlua_checkstring(L, 4);
	align = qwlua_checkint(L, 5);
	color = qwlua_checkstring(L, 6);
	text = qwlua_checkstring(L, 7);

	ct = MS_Add_Content((char *)menu_id, (char *)parent_id, (char *)id, 2, 3, align, (char *)text, NULL, NULL, (char *)color, NULL, NULL, 0, 0, 0, NULL, NULL, &LUA_Button_Function);
	if (ct == NULL)
	{
		Com_Printf("LF_Menu_Add_Content_Button_Color error!\n");
	}
	else
	{
		MS_Set_Content_Lua_State(ct, L);
	}

	return 0;
}

static int LF_Menu_Add_Content_Editbox_Double(lua_State *L)
{
	struct menu_ct *ct;
	const char *menu_id, *parent_id, *id;
	int align;
	struct lua_double *num;

	menu_id = qwlua_checkstring(L, 2);
	parent_id = qwlua_checkstring(L, 3);
	parent_id = (const char *)check_null((char *)parent_id);
	id = qwlua_checkstring(L, 4);
	align = qwlua_checkint(L, 5);
	num = (struct lua_double *)qwlua_touserdata(L, 6);

	ct = MS_Add_Content((char *)menu_id, (char *)parent_id, (char *)id, 4, 2, align, NULL, NULL, NULL, NULL, NULL, NULL, 12, 0, 0, "d", &num->ptr, NULL);
	if (ct == NULL)
	{
		Com_Printf("LF_Menu_Add_Content_Editbox_Double error!\n");
	}
	else
	{
		MS_Set_Content_Lua_State(ct, L);
	}

	return 0;
}

static int LF_Menu_Add_Content_Editbox_Int(lua_State *L)
{
	struct menu_ct *ct;
	const char *menu_id, *parent_id, *id;
	int align;
	struct lua_int *num;

	menu_id = qwlua_checkstring(L, 2);
	parent_id = qwlua_checkstring(L, 3);
	parent_id = (const char *)check_null((char *)parent_id);
	id = qwlua_checkstring(L, 4);
	align = qwlua_checkint(L, 5);
	num = (struct lua_int *)qwlua_touserdata(L, 6);

	ct = MS_Add_Content((char *)menu_id, (char *)parent_id, (char *)id, 4, 2, align, NULL, NULL, NULL, NULL, NULL, NULL, 10, 0, 0, "i", &num->ptr, NULL);
	if (ct == NULL)
	{
		Com_Printf("LF_Menu_Add_Content_Editbox_Int error!\n");
	}
	else
	{
		MS_Set_Content_Lua_State(ct, L);
	}

	return 0;
}

static int LF_Menu_Add_Content_Slider_Width_Double(lua_State *L)
{
	struct menu_ct *ct;
	const char *menu_id, *parent_id, *id;
	int align, width;
	double lower_limit, upper_limit;
	struct lua_double *num;

	menu_id = qwlua_checkstring(L, 2);
	parent_id = qwlua_checkstring(L, 3);
	parent_id = (const char *)check_null((char *)parent_id);
	id = qwlua_checkstring(L, 4);
	align = qwlua_checkint(L, 5);
	width = qwlua_checkint(L, 6);
	lower_limit = qwlua_tonumber(L, 7);
	upper_limit = qwlua_tonumber(L, 8);
	num = (struct lua_double *)qwlua_touserdata(L, 9);

	ct = MS_Add_Content_Slider_Width((char *)menu_id, (char *)parent_id, (char *)id,  2, align, "d", &num->ptr, lower_limit, upper_limit, width);
	if (ct == NULL)
	{
		Com_Printf("LF_Menu_Add_Content_Slider_Width_Double error!\n");
	}
	else
	{
		MS_Set_Content_Lua_State(ct, L);
	}

	return 0;
}

static int LF_Menu_Add_Content_Slider_Width_Int(lua_State *L)
{
	struct menu_ct *ct;
	const char *menu_id, *parent_id, *id;
	int align, width;
	double lower_limit, upper_limit;
	struct lua_int *num;

	menu_id = qwlua_checkstring(L, 2);
	parent_id = qwlua_checkstring(L, 3);
	parent_id = (const char *)check_null((char *)parent_id);
	id = qwlua_checkstring(L, 4);
	align = qwlua_checkint(L, 5);
	width = qwlua_checkint(L, 6);
	lower_limit = qwlua_tonumber(L, 7);
	upper_limit = qwlua_tonumber(L, 8);
	num = (struct lua_int *)qwlua_touserdata(L, 9);

	ct = MS_Add_Content_Slider_Width((char *)menu_id, (char *)parent_id, (char *)id,  2, align, "i", &num->ptr, lower_limit, upper_limit, width);
	if (ct == NULL)
	{
		Com_Printf("LF_Menu_Add_Content_Slider_Width_Int error!\n");
	}
	else
	{
		MS_Set_Content_Lua_State(ct, L);
	}

	return 0;
}

static int LF_Menu_Change_Content_Text(lua_State *L)
{
	struct menu_ct *ct;
	const char *text;

	ct = (struct menu_ct *)qwlua_touserdata(L, 2);
	text = qwlua_tostring(L, 3);

	MS_CT_Change_Text(ct, (char *)text);

	return 0;
}

static int LF_Menu_Create_Double(lua_State *L)
{
	struct lua_double *f;

	f = calloc(1, sizeof(struct lua_double));
	f->ptr = &f->value;
	qwlua_pushlightuserdata(L, f);

	return 1;
}

static int LF_Menu_Get_Double(lua_State *L)
{
	struct lua_double *f;

	f = (struct lua_double *)qwlua_touserdata(L, 2);

	qwlua_pushnumber(L, f->value);

	return 1;
}

static int LF_Menu_Set_Double(lua_State *L)
{
	struct lua_double *f;

	f = (struct lua_double*) qwlua_touserdata(L, 2);
	f->value = qwlua_tonumber(L, 3);

	return 0;
}
	
static int LF_Menu_Create_Int(lua_State *L)
{
	struct lua_int *f;

	f = calloc(1, sizeof(struct lua_int));
	f->ptr = &f->value;
	qwlua_pushlightuserdata(L, f);

	return 1;
}

static int LF_Menu_Get_Int(lua_State *L)
{
	struct lua_int *f;

	f = (struct lua_int *)qwlua_touserdata(L, 2);

	qwlua_pushnumber(L, f->value);

	return 1;
}

static int LF_Menu_Set_Int(lua_State *L)
{
	struct lua_int *f;

	f = (struct lua_int *) qwlua_touserdata(L, 2);
	f->value = qwlua_tonumber(L, 3);

	return 0;
}

static int LF_Spline_Create(lua_State *L)
{
	struct CamSys_Spline *spline;

	spline = CamSys_Add_Spline(2);

	qwlua_pushlightuserdata(L, spline);

	return 1;
}

static int LF_Spline_Add_Point(lua_State *L)
{
	struct CamSys_Spline *spline;
	vec3_t	point;

	spline = (struct CamSys_Spline *)qwlua_touserdata(L, 2);
	point[0] = qwlua_tonumber(L, 3);
	point[1] = qwlua_tonumber(L, 4);
	point[2] = qwlua_tonumber(L, 5);

	Spline_Insert_Point(spline->spline, point, NULL, 0, NULL);

	return 0;
}

static int LF_Spline_Delete_All_Points(lua_State *L)
{
	struct CamSys_Spline *spline;
	int i;

	spline = (struct CamSys_Spline *)qwlua_touserdata(L, 2);

	while(Spline_Get_Point_Count(spline->spline))
	{
		Spline_Remove_Point_By_Number(spline->spline, 1);
	}

	return 0;
}

static int LF_Spline_Set_Point(lua_State *L)
{
	struct CamSys_Spline *spline;
	int point;
	double x, y, z;
	vec3_t p;

	spline = (struct CamSys_Spline *)qwlua_touserdata(L, 2);

	point = qwlua_checkint(L, 3);

	x = qwlua_tonumber(L, 4);
	y = qwlua_tonumber(L, 5);
	z = qwlua_tonumber(L, 6);

	VectorSet(p, x, y, z);

	Spline_Set_Point_From_Number(spline->spline, point,p);

	return 0;
}

static int LF_Spline_Show(lua_State *L)
{
	struct CamSys_Spline *spline;
	int show;

	spline = (struct CamSys_Spline *)qwlua_touserdata(L, 2);

	show = qwlua_tonumber(L, 3);

	spline->show = show;

	return 0;
}

static int LF_Spline_Color(lua_State *L)
{
	struct CamSys_Spline *spline;
	float r, g, b;

	spline = (struct CamSys_Spline *)qwlua_touserdata(L, 2);

	r = qwlua_tonumber(L, 3);
	g = qwlua_tonumber(L, 4);
	b = qwlua_tonumber(L, 5);

	spline->r = r;
	spline->g = g;
	spline->b = b;

	return 0;
}

void CamSys_Duplicate_Spline(struct CamSys_Spline *original, struct CamSys_Spline *copy);
static int LF_Spline_To_Cam(lua_State *L)
{
	struct CamSys_Spline *spline;
	struct CamSys_Spline *new;

	spline = (struct CamSys_Spline *)qwlua_touserdata(L, 2);

	new = CamSys_Add_Spline(0);
	CamSys_Duplicate_Spline(spline, new);
	return 0;
}

static int LF_Spline_To_View(lua_State *L)
{
	struct CamSys_Spline *spline;
	struct CamSys_Spline *new;

	spline = (struct CamSys_Spline *)qwlua_touserdata(L, 2);

	new = CamSys_Add_Spline(1);
	CamSys_Duplicate_Spline(spline, new);
	return 1;
}


static luaL_reg Spline_Functions_Methods[] = {
	{"create", LF_Spline_Create},
	{"add_point", LF_Spline_Add_Point},
	{"set_point", LF_Spline_Set_Point},
	{"delete_all_points", LF_Spline_Delete_All_Points},
	{"show", LF_Spline_Show},	
	{"color", LF_Spline_Color},	
	{"to_view", LF_Spline_To_View},
	{"to_cam", LF_Spline_To_Cam},
	{0, 0}
	};

static luaL_reg Functions_Methods[] = {
	{"com_printf",	LF_Com_Printf},
	{"get_position", LF_Get_Position},
	{"rotate_point_around_vector", LF_Rotate_Point_Around_Vector},
	{"rotate_point_around_line", LF_Rotate_Point_Around_Line},
	{0, 0}
	};

static luaL_reg Menu_Functions_Methods[] = {
	{"add_to_mainmenu", LF_Menu_Add_To_Mainmenu},
	{"add_content_text", LF_Menu_Add_Content_Text},
	{"add_content_text_update", LF_Menu_Add_Content_Text_Update_Function},
	{"add_content_button_color", LF_Menu_Add_Content_Button_Color},
	{"add_content_editbox_double", LF_Menu_Add_Content_Editbox_Double},
	{"add_content_slider_width_double", LF_Menu_Add_Content_Slider_Width_Double},
	{"add_content_editbox_int", LF_Menu_Add_Content_Editbox_Int},
	{"add_content_slider_width_int", LF_Menu_Add_Content_Slider_Width_Int},
	{"change_content_text", LF_Menu_Change_Content_Text},
	{"double_create", LF_Menu_Create_Double},
	{"double_get", LF_Menu_Get_Double},
	{"double_set", LF_Menu_Set_Double},
	{"int_create", LF_Menu_Create_Int},
	{"int_get", LF_Menu_Get_Int},
	{"int_set", LF_Menu_Set_Int},
	{0,0}
};

static luaL_reg Functions_Meta[] = {
	{0,0}
};



static void Register_Functions(char *libname, lua_State *L, luaL_reg *methods, luaL_reg *meta)
{
	qwlua_openlib(L, libname, methods, 0);

	qwlua_newmetatable(L, libname);          /* create metatable for Foo,
					 and add it to the Lua registry */
	qwlua_openlib(L, 0, meta, 0);    /* fill metatable */
	qwlua_pushliteral(L, "__index");
	qwlua_pushvalue(L, -3);               /* dup methods table*/
	qwlua_rawset(L, -3);                  /* metatable.__index = methods */
	qwlua_pushliteral(L, "__metatable");
	qwlua_pushvalue(L, -3);               /* dup methods table*/
	qwlua_rawset(L, -3);                  /* hide metatable:
					 metatable.__metatable = methods */
	qwlua_pop(L, 1);                      /* drop metatable */
}




/*
 * lib functions
 */

/*


static int LF_Com_Printf(lua_State *L)
{
	char *string;

	string = luaL_checkstring(L, 2);
	if (string)
		Com_Printf("%s\n", string);
	return 0;
}

void HUD_Get_Position(int pointer, int *x, int *y);

static int LF_Hud_Get_Position(lua_State *L)
{
	int pointer, x, y;

	pointer = luaL_checkint(L, 2);
	if (pointer)
	{
		HUD_Get_Position(pointer, &x, &y);
		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		return 2;
	}
	return 0;
}

void HUD_Draw_Picture(int x, int y, char *picture);
static int LF_Hud_Draw_Picture(lua_State *L)
{
	int x, y;
	char *string;

	x = luaL_checkint(L, 2);
	y = luaL_checkint(L, 3);
	string = luaL_checkstring(L, 4);
	if (!string)
		return 0;
	
	HUD_Draw_Picture(x, y, string);

	return 0;
}

static int LF_Hud_Draw_String(lua_State *L)
{
	int x, y;
	char *string;

	x = luaL_checkint(L, 2);
	y = luaL_checkint(L, 3);
	string = luaL_checkstring(L, 4);
	if (!string)
		return 0;
	
	Draw_ColoredString(x, y, string, 0);

	return 0;
}

static const luaL_reg Functions_Methods[] = {
	{"com_printf",	LF_Com_Printf},
	{"hud_get_position", LF_Hud_Get_Position},
	{"hud_draw_picture", LF_Hud_Draw_Picture},
	{"hud_draw_string", LF_Hud_Draw_String},
	{0,0}
};

static const luaL_reg Functions_Meta[] = {
	{0,0}
};

// returns 1 if the table exists and leaves it on the stack
int Lua_Has_Table(lua_State *L, char *table)
{
	lua_getglobal(L, table);
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 0;
	}
	else
	{
		return 1;
	}
}

void Lua_Table_Set(lua_State *L, char *field, int value)
{
		lua_pushstring(L, field);
		lua_pushnumber(L, value);
		lua_settable(L, -3);
}

void Lua_STAT_ITEMS_Table_Helper(lua_State *L, char *field, int value, int IT)
{
	if (value & IT)
		Lua_Table_Set(L, field, 1);
	else
		Lua_Table_Set(L, field, 0);
}

void Lua_Set_Stats(lua_State *L, int stat, int value)
{
	if(!L)
		return;

	if (stat == STAT_HEALTH)
	{
		Lua_Table_Set(L, "health", value);
		return;
	}

	if (stat == STAT_ITEMS)
	{
		Lua_STAT_ITEMS_Table_Helper(L, "axe", value, IT_AXE);
		Lua_STAT_ITEMS_Table_Helper(L, "shotgun", value, IT_SHOTGUN);
		Lua_STAT_ITEMS_Table_Helper(L, "super_shotgun", value, IT_SUPER_SHOTGUN);
		Lua_STAT_ITEMS_Table_Helper(L, "nailgun", value, IT_NAILGUN);
		Lua_STAT_ITEMS_Table_Helper(L, "super_nailgun", value, IT_SUPER_NAILGUN);
		Lua_STAT_ITEMS_Table_Helper(L, "rocket_launcher", value, IT_ROCKET_LAUNCHER);
		Lua_STAT_ITEMS_Table_Helper(L, "grenade_launcher", value, IT_GRENADE_LAUNCHER);
		Lua_STAT_ITEMS_Table_Helper(L, "lightning", value, IT_LIGHTNING);
		Lua_STAT_ITEMS_Table_Helper(L, "quad", value, IT_QUAD);
		Lua_STAT_ITEMS_Table_Helper(L, "invulnarability", value, IT_INVULNERABILITY);
		Lua_STAT_ITEMS_Table_Helper(L, "invisibility", value, IT_INVISIBILITY);
		return;
	}

	if (stat == STAT_SHELLS)
		Lua_Table_Set(L, "shells", value);

	if (stat == STAT_NAILS)
		Lua_Table_Set(L, "nails", value);

	if (stat == STAT_CELLS)
		Lua_Table_Set(L, "cells", value);

	if (stat == STAT_ROCKETS)
		Lua_Table_Set(L, "rockets", value);
}

void Lua_Stat_Changed(int stat, int value)
{
	struct L_lua_states *ls;
	char buf[1024];

	ls = L_lua_states;

	while(ls)
	{
		if (Lua_Has_Table(ls->L, "player_stat"))
			Lua_Set_Stats(ls->L, stat, value);
		ls = ls->next;
	}
}
*/

static void LUA_Print_Globals(struct L_lua_states *ls)
{
	qwlua_getglobal(ls->L, "_G");
	qwlua_pushnil(ls->L);

	while (qwlua_next(ls->L, -2))
	{
		Com_Printf("%s - %s\n", qwlua_tostring(ls->L, -2), qwlua_typename(ls->L, qwlua_type(ls->L, -1)));

		qwlua_pop(ls->L, 1);
	}
}

static struct L_lua_states *New_Lua_State(char *name, char *script, int *i)
{
	struct L_lua_states *ls;

	*i = 0;

	if (!name)
	{
		*i = -1;
		return NULL;
	}

	ls = Find_Lua_State(name);

	if (ls)
	{
		*i = -2;
		return NULL;
	}

	ls = Add_Lua_State();

	if (!ls)
	{
		*i = -3;
		return NULL;
	}

	ls->name = strdup(name);

	ls->L = qwlua_open();
	if (!ls->L)
	{
		Remove_Lua_State(name);
		*i = -4;
		return NULL;
	}
	qwlua_openlibs(ls->L);

/*
	qwlua_pushstring(ls->L, "lua/helpers/?.lua");
	qwlua_setglobal(ls->L, "LUA_PATH");
*/
	qwlua_getglobal(ls->L, "package");
	qwlua_pushstring(ls->L, "path");
	qwlua_pushstring(ls->L, "lua/helpers/?.lua");
	qwlua_settable(ls->L, -3);

	Register_Functions("Menu", ls->L, Menu_Functions_Methods, Functions_Meta); 
	Register_Functions("Quake", ls->L, Functions_Methods, Functions_Meta); 
	Register_Functions("Spline", ls->L, Spline_Functions_Methods, Functions_Meta); 


	if (script)
	{
		if (qwlua_loadfile(ls->L, script) != 0)
			Com_Printf("error %s\n", qwlua_tostring(ls->L, -1));

		if (qwlua_pcall(ls->L, 0, LUA_MULTRET, 0) != 0)
			Com_Printf("error %s\n", qwlua_tostring(ls->L, -1));

	}

	//LUA_Print_Globals(ls);

	return ls;
}

void Lua_Run_Function(char *state, char *function)
{
	struct L_lua_states *ls;
	int i;

	if(!state || !function)
		return;

	ls = Find_Lua_State(state);
	if (!ls)
		return;

	
	qwlua_getglobal(ls->L, function);
	if ((i = qwlua_pcall(ls->L, 0, 0, 0)) != 0)
	{
		Com_Printf("lua-error %s\n", qwlua_tostring(ls->L, -1));
	}
}

void Lua_Run_Function_Arguments(char *state, char *function, char *args, ... )
{
	struct L_lua_states *ls;
	int i, narg;
	va_list vl;

	if(!state || !function || !args)
		return;

	ls = Find_Lua_State(state);
	if (!ls)
		return;
	
	qwlua_getglobal(ls->L, function);

	narg = 0;
	va_start(vl, args);

	while (*args)
	{
		switch (*args++)
		{
			case 'd':
				qwlua_pushnumber(ls->L, va_arg(vl, double));
				break;
			case 'i':
				qwlua_pushnumber(ls->L, va_arg(vl, int));
				break;
			case 's':
				qwlua_pushstring(ls->L, va_arg(vl, char *));
				break;
			default:
				return;
		}
		narg++;
	}

	if ((i = qwlua_pcall(ls->L, narg, 0, 0)) != 0)
	{
		Com_Printf("error %s\n", qwlua_tostring(ls->L, -1));
	}
}

void LUA_Delete_f(void)
{
	if (Cmd_Argc() != 2)
		return;

	Remove_Lua_State(Cmd_Argv(1));
}

void LUA_Init_f(void)
{
	int i;

	if (Cmd_Argc() != 3)
		return;

	New_Lua_State(Cmd_Argv(1), Cmd_Argv(2), &i);
}

int LUA_Load_Libs(void)
{
#ifdef WIN32
	return (LIB_Load("lua.dll", lua_functions, true));
#else
	return (LIB_Load("liblua.so", lua_functions, true));
#endif
}

void LUA_Load_Scripts(void)
{
	char name[256];
	int i;
	struct L_lua_states *ls;
#ifdef WIN32
	char *ptr;
	int len;

	WIN32_FIND_DATA findFileData;

	HANDLE hFind = FindFirstFile((LPCWSTR)"*", &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		Com_Printf("error opening lua dir.\n");
		return;
	}


	while(FindNextFile(hFind, &findFileData))
	{
		len = strlen(findFileData.cFileName);
		if (len > 4)
		{
			len -= 4;
			ptr = findFileData.cFileName + len;
			if (strcmp(ptr, ".lua") == 0)
			{
				Com_Printf("loading %s\n", findFileData.cFileName);
				snprintf(name, sizeof(name), "lua/%s", findFileData.cFileName);
				ls = New_Lua_State(name, name, &i);
				if (i < 0)
				{
					Com_Printf("error loading %s, %i\n", findFileData.cFileName, i);
				}
				else
				{

				}
			}
		}
	}

	HANDLE hFind = FindFirstFile((LPCWSTR)"*", &findFileData);
#else
	DIR *dir;
	struct dirent *dp;
	char *ptr;
	int len;

	dir = opendir("lua");

	if (dir == NULL)
	{
		Com_Printf("error opening lua dir.\n");
		return;
	}

	while ((dp = readdir(dir)) != NULL)
	{
		len = strlen(dp->d_name);
		if (len > 4)
		{
			len -= 4;
			ptr = dp->d_name + len;
			if (strcmp(ptr, ".lua") == 0)
			{
				Com_Printf("loading %s\n", dp->d_name);
				snprintf(name, sizeof(name), "lua/%s", dp->d_name);
				ls = New_Lua_State(name, name, &i);
				if (i < 0)
				{
					Com_Printf("error loading %s, %i\n", name, i);
				}
				else
				{
					qwlua_getglobal(ls->L, "init");
					qwlua_pushnumber(ls->L, cls.realtime);
					if (qwlua_pcall(ls->L, 1, 0, 0) != 0)
					{
						Com_Printf("error: %s, init function not found.\n", qwlua_tostring(ls->L, -1));
					}
				}
			}
		}
	}

	closedir(dir);
#endif
}

void LUA_Init(void)
{

	if (LUA_Load_Libs())
	{
		Com_Printf("could not load lua.\n");
		return;
	}
	else
	{
		Com_Printf("lua loaded.\n");
	}

	Cmd_AddCommand("lua_delete", LUA_Delete_f);
	Cmd_AddCommand("lua_init", LUA_Init_f);

	LUA_Load_Scripts();
}

void LUA_Frame(void)
{
	struct L_lua_states *ls;
	int i;

	ls = L_lua_states;

	while (ls)
	{
		if (ls->buggy == 0)
		{
			qwlua_getglobal(ls->L, "main");

			qwlua_pushnumber(ls->L, cls.realtime);

			if ((i = qwlua_pcall(ls->L, 1, 0, 0)) != 0)
			{
				Com_Printf("error: %s\n", qwlua_tostring(ls->L, -1));
				ls->buggy = 1;
			}
		}
		
		ls = ls->next;
	}
}
