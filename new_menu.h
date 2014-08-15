struct MS_Popup_Entry
{
	void *null;
};

struct MS_Popup_Menu_Menu
{
	void *null;
};

struct menu_ct 
{
	void *null;
};

struct menu_s
{
	void *null;
};

struct MS_Popup_Entry *MS_Popup_Add_Entry(struct MS_Popup_Menu_Menu *menu, struct MS_Popup_Entry *entry, char *name, void (*f)(void *data), void *data, char *command);
void MS_Popup_Menu_Set_Type(struct MS_Popup_Menu_Menu *menu, int type);
struct MS_Popup_Menu_Menu *MS_Popup_Add_Menu(void);
void MS_Popup_Menu_Set(struct MS_Popup_Menu_Menu *menu, int x, int y);
void MS_Popup_Remove_Menu(struct MS_Popup_Menu_Menu *menu);

void MS_CT_Change_Text(struct menu_ct *content, char *text);
struct menu_ct *MS_Add_Content(char *mid, char *pid, char *identifier, int type, int subtype, int align, char *text, char *tooltip, char *name, char *color, mpic_t *picture, cvar_t *variable, int width , float limmin, float limmax, char *ptype, void *vptr, void (*update_function)(struct menu_ct *self));
struct menu_s *MS_Add_Menu(int lx, int ly, int rx, int ry, char *name, char *identifier, void (*function)(struct menu_s *self));
void MS_Activate_Menu_Struct(struct menu_s *menu);
struct menu_ct *MS_Add_Content_Text(char *mid,char *pid,char *id,int align,char *text);
struct menu_ct *MS_Add_Content_Slider(char *mid, char *pid, char *id, int subtype, int align, char *ptype, void *vptr,float llim,float mlim);
struct menu_ct *MS_Add_Content_Slider_Width(char *mid, char *pid, char *id, int subtype, int align, char *ptype, void *vptr,float llim,float mlim, int width);

struct menu_ct *MS_Add_Content(char *mid, char *pid, char *identifier, int type, int subtype, int align, char *text, char *tooltip, char *name, char *color, mpic_t *picture, cvar_t *variable, int width , float limmin, float limmax, char *ptype, void *vptr, void (*update_function)(struct menu_ct *self));
struct MS_Popup_Entry *MS_Popup_Add_Entry(struct MS_Popup_Menu_Menu *menu, struct MS_Popup_Entry *entry, char *name, void (*f)(void *data), void *data, char *command);
void MS_Set_Content_Lua_State(struct menu_ct *ct, void *ls);
void *MS_Get_Content_Lua_State(struct menu_ct *ct);
char *MS_Get_Content_Identifier(struct menu_ct *ct);

