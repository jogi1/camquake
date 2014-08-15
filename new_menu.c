/*

	A good menu system :>

*/


#include "quakedef.h"
#include "keys.h"

void Draw_AlphaString (int x, int y, char *str, float alpha);
void Draw_AlphaRectangleRealRGB (int x, int y, int w, int h, float thickness, qbool fill, float r, float g, float b, float a);
void Update_Num(void);
void CamSys_MenuHook(void);

int slider_active = 0;

/* types 
	1 :	text
		- no subtypes
	2 :	button
		1 : Cbuf_AddText(name)
		2 : call the normal function
		3 : call the update function (added for lua)
	3 :	slider
		1 : slide a cvar :D
		2 : slide a pointer
	4 :	textbox
		1 : set the cvar
		2 : set a pointer
	5 :	picture
	6 :	dropdownbox
	7 :	
*/

/* location
	
*/

/* align
	1 :	left/top
	2 :	right/bottom
	3 :	center
*/

struct menu_ct {
	int		type;
	int		subtype;
	int		lx;
	int		ly;
	int		rx;
	int		ry;
	int		pixel_width, pixel_height;
	int		align;

	void	*lua_state;
	void	(*function)(void);
	void	(*update_function)(struct menu_ct *self);
	char	*(*text_function)(void);
	char	*identifier;
	char	*name;
	int		drawn;
	char	*tooltip;
	char	*text;
	char	*color;
	int		width;
	double	hovertime;
	double	pressed_time;
	int		pressed;
	cvar_t	*variable;
	float		limit[2];
	struct	menu_ct	*next;
	struct	menu_ct	*parent;
	mpic_t	*picture;
	int		**iptr;
	float	**fptr;
	double	**dptr;
};


struct menu_s {
	int		show;
	struct	menu_s *next;
	struct	menu_ct *content;
	char	*name;
	char	*identifier;
	int		lx;
	int		ly;
	int		rx;
	int		ry;
	int		minimized;
	int		minx;
	int		miny;
	int		content_width;
	int		content_height;
	int	slider_offset_x;
	int	slider_offset_y;
	void	(*update_function)(struct menu_s *self);

};

int		mscx	=	0;
int		mscy	=	0;
int		resizing	=	0;
int		moving		=	0;
int		scrolling	=	0;
int	popup_active = 0;

qbool ON_Change_pp(cvar_t *var, char *string);

void On_Change_JMS(cvar_t *var, char *string, qbool *cancel)
{
	int i;
	i = atoi(string);

	if (i == 1)
	{
		key_dest = key_new_menu;
	}
	else
		key_dest = key_game;
		
}

cvar_t	jogi_menu_system		=	{"new_menu_system","0", 0, On_Change_JMS};
cvar_t	jogi_menu_system_debug	=	{"jogi_menu_system_debug","1"};
cvar_t	jogi_menu_system_border_alpha	=	{"jogi_menu_system_border_alpha","1"};
cvar_t	jogi_menu_system_background_alpha	=	{"jogi_menu_system_background_alpha","1"};
cvar_t	jogi_menu_system_red	=	{"jogi_menu_system_red","0"};
cvar_t	jogi_menu_system_green	=	{"jogi_menu_system_green","1"};
cvar_t	jogi_menu_system_blue	=	{"jogi_menu_system_blue","0"};
cvar_t	new_menu_sensitivity	=	{"jogi_menu_system_sensitivity", "0.1"};


struct menu_s	*menu = NULL;
struct menu_s	*menu_active = NULL;

int		kdmb1;
int		kdmb2;

int		ssc = 0;
//int		selected_editbox =0 ;
struct menu_ct	*selected_editbox = NULL;
struct menu_ct	*selected_slider = NULL;
struct menu_ct	*hoverend_content = NULL;
struct menu_ct	*last_hovered = NULL;
char	tmpstring[128];
float	editbox_lasttime;
double menu_mouse_x = 0, menu_mouse_y = 0;

void MS_Handle_Content(struct menu_ct *content);
void Activate_Popup_Menu(void);
void Deactivate_Popup_Menu(void);

int MS_Get_Key(int key, int down)
{
	int l;

	if (key == K_MOUSE2 && popup_active == 0 && down)
	{
		Activate_Popup_Menu();
		return 0;
	}

	if (key == K_MOUSE2 && popup_active == 1 && !down)
	{
		Deactivate_Popup_Menu();
		return 0;
	}
	
	if (key == K_MWHEELUP && down)
	{
		MS_Handle_Content(hoverend_content);
		return 0;
	}

	if (key == K_MWHEELDOWN && down)
	{
		MS_Handle_Content(hoverend_content);
		return 0;
	}


	if (selected_editbox && down)
	{

		switch (key) {
			case K_ESCAPE:
					memset(&tmpstring,0,sizeof(tmpstring));
					selected_editbox = NULL;
					ssc = 0;
					return 1;
			case K_ENTER:
					if (selected_editbox->subtype == 1)
						Cvar_Set(selected_editbox->variable,tmpstring);
					if (selected_editbox->subtype == 2){
						if (selected_editbox->iptr != NULL){
							**selected_editbox->iptr = atoi(tmpstring);
						}
						if (selected_editbox->fptr != NULL){
							**selected_editbox->fptr = atof(tmpstring);
						}
						if (selected_editbox->dptr != NULL){
							**selected_editbox->dptr = (double)atof(tmpstring);
						}
					}
					memset(&tmpstring,0,sizeof(tmpstring));
					selected_editbox = NULL;
					ssc = 0;
					return 1;
			case K_BACKSPACE:
					if (strlen(tmpstring))
							tmpstring[strlen(tmpstring)-1] = 0;
					return 1;
			default:
					if (key < 32 || key > 127)
						break;
					l = strlen(tmpstring);
					if (l < 128 && l< selected_editbox->width) {
						tmpstring[l+1] = 0;
						tmpstring[l] = key;
						return 1;
					}
					break;
			}
	}
	return 0;
}

static void MS_Draw_Tooltip(int x, int y, char *string, float r, float g, float b,float alpha) 
{
	int len = strlen(string) * 8;

	// Make sure we're drawing within the screen.
	if (x + len > vid.width)
	{
		x -= len;
	}

	if (y + 9 > vid.height)
	{
		y -= 9;
	}

	Draw_AlphaRectangleRealRGB(x, y, len, 8, 1, 1, r, g, b,alpha);
	Draw_String(x, y, string);
}

void MS_Mouse(void)
{
	
	struct menu_s	*mm = NULL;

	static double remaining_x;
	static double remaining_y;
	

	int type = 0;
	int lx,ly,rx,ry;
	int x,y;

	menu_mouse_x *= new_menu_sensitivity.value;
	menu_mouse_y *= new_menu_sensitivity.value;

	menu_mouse_x += remaining_x;
	menu_mouse_y += remaining_y;

	remaining_x = menu_mouse_x - floor(menu_mouse_x);
	remaining_y = menu_mouse_y - floor(menu_mouse_y);
	menu_mouse_x = floor(menu_mouse_x);
	menu_mouse_y = floor(menu_mouse_y);


	/*
	1- moving
	2- resize
	3- minimize
	4- close
	*/

	if (!menu_active)
	{
		lx = -1;
		ly = -1;
		rx = -1;
		ry = -1;
		type = -1;
	}
	else
	{
		type = 0;

		mm = menu_active;
		
		lx = mm->lx;
		ly = mm->ly;
		rx = mm->rx;
		ry = mm->ry;

		if (mm->minimized)
		{
			rx = strlen(mm->name)*8+22 +lx;
			ry = ly + 22;
		}
	}


	// we always draw the cursor
	Draw_String(mscx-4, mscy-4, "x");
	if (popup_active)
	{
		if (mscx + menu_mouse_x > 0 && mscx + menu_mouse_x < vid.width)
			mscx	+=	menu_mouse_x;
		if (mscy + menu_mouse_y > 0 && mscy + menu_mouse_y < vid.height)
			mscy	+=	menu_mouse_y;
		return;
	}
	
	// check if we are in the activemenu
	if ( mscx >= lx && mscx <= rx && mscy >= ly && mscy <= ry)
	{
	
	}
	else
	{
	}

	if (mm == NULL)
		return;

	// check if we are in the moving area
	if (mscx >= lx && mscx <= rx - 18 && mscy >= ly && mscy <= ly + 10)
	{
		MS_Draw_Tooltip(mscx,mscy,va("%s moving",mm->name,lx,ly,rx,ry),0.5,0.5,0.5,0.9);
		type = 1;

	}
	// check if we are in the resize area
	if (mscx >= rx - 10 && mscx <= rx && mscy >= ry - 10 && mscy <= ry)
	{
		MS_Draw_Tooltip(mscx,mscy,va("%s resizing",mm->name),.5,0.5,0.5,0.9);
		type = 2;
	}
	// minimize
	if (mscx >= rx - 17 && mscx <= rx - 8 && mscy >= ly + 1  && mscy <= ly + 10)
	{
		MS_Draw_Tooltip(mscx,mscy,va("%s minimizing",mm->name),.5,0.5,0.5,0.9);
		type = 3;
	}

	// close
	if (mscx >= rx - 9 && mscx <= rx && mscy >= ly + 1 && mscy <= ly + 9)
	{
		MS_Draw_Tooltip(mscx,mscy,va("close %s",mm->name),.5,0.5,0.5,0.9);
		type = 4;
	}

	// scroll x
	if (mm->content_width > rx - lx - 10)
	if (mscx >= lx  && mscx <= rx - 9 && mscy >= ry - 8 && mscy <= ry )
	{
		MS_Draw_Tooltip(mscx,mscy,va("scroll %s", mm->name),.5,0.5,0.5,0.9);
		type = 5;
	}
	// scroll y
	if (mm->content_height > ry - ly - 22)
	if (mscx >= rx -9  && mscx <= rx && mscy >= ly - 8 && mscy <= ry - 9 )
	{
		MS_Draw_Tooltip(mscx,mscy,va("scroll %s", mm->name),.5,0.5,0.5,0.9);
		type = 6;
	}



	if (keydown[K_MOUSE1] && type == 4)
	{
		menu_active	= NULL;
	}


	if(!slider_active)
	{
		if (mscx + menu_mouse_x > 0 && mscx + menu_mouse_x < vid.width)
			mscx	+=	menu_mouse_x;
		if (mscy + menu_mouse_y > 0 && mscy + menu_mouse_y < vid.height)
			mscy	+=	menu_mouse_y;
	}
	


	

	if (keydown[K_MOUSE1] && 	!kdmb1)
			kdmb1 = 1;

	if (!keydown[K_MOUSE1] && 	kdmb1)
	{
			slider_active = 0;
			kdmb1 = 0;
	}

	if (!keydown[K_MOUSE1])
	{
			resizing = 0;
			moving = 0;
			scrolling = 0;
	}

	if (keydown[K_MOUSE1])
	{
		if (type == 1)
			moving = 1;
		if (type == 2)
			resizing = 1;
		if (type == 5 || type == 6)
			scrolling = 1;
	}

	if (type != -1)
	{
		if (mm->rx < strlen(mm->name)*8)
			mm->rx = strlen(mm->name)*8 + 40;
		if (mm->ry < mm->ly + 20)
			mm->ry = mm->ly + 20;
	}

	if (keydown[K_MOUSE1] && type != -1){
		if (type == 1){
			if (mm->rx + menu_mouse_x > vid.width || mm->ry + menu_mouse_y > vid.height || mm->lx + menu_mouse_x < 0 || mm->ly + menu_mouse_y < 0)
				return;
			moving = 1;
			mm->lx += menu_mouse_x;
			mm->rx += menu_mouse_x;
			mm->ly += menu_mouse_y;
			mm->ry += menu_mouse_y;
			mscx = mm->lx + 2;
			mscy = mm->ly + 2;
		}
		if (type == 2){

			if (mm->rx + menu_mouse_x > vid.width || mm->ry + menu_mouse_y > vid.height || mm->ry +menu_mouse_y< mm->ly + 20 || mm->rx +menu_mouse_x <= mm->lx + strlen(mm->name)*8 + 22)
				return;
			mm->rx += menu_mouse_x;
			mm->ry += menu_mouse_y;
			resizing = 1;
			mscx = mm->rx - 5;
			mscy = mm->ry - 5;
		}
		if (type == 3)
			mm->minimized = !mm->minimized;

		if (type == 5)
		{
			mm->slider_offset_x += menu_mouse_x;
			if (mm->slider_offset_x < 0)
				mm->slider_offset_x = 0;
			if (mm->slider_offset_x > 100)
				mm->slider_offset_x = 100;

			x = mm->rx - mm->lx - 11; //100
			x = x *  (float)(x/(float)(mm->content_width));
			
			y = mm->rx - mm->lx - 11;
			y = y - x;
			y = y * (float)(mm->slider_offset_x/100.0f);

			y = y + x/2;
			mscx = mm->lx + y;
			mscy = mm->ry - 5;




		}

		if (type == 6)
		{
			mm->slider_offset_y += menu_mouse_y;
			if (mm->slider_offset_y < 0)
				mm->slider_offset_y = 0;
			if (mm->slider_offset_y > 100)
				mm->slider_offset_y = 100;

			y = mm->ry - mm->ly - 22;
			y = y * (float)(y/(float)(mm->content_height));

			x = mm->ry - mm->ly - 22;
			x = x -y;
			x = x * (float)(mm->slider_offset_y/100.0f);

			x = x + y/2;


			mscy = mm->ly + 11 + x;
			mscx = mm->rx - 5;
		}

	}
}

void MS_Handle_Content_Slider(struct menu_ct *content){
	double	value = 0;
	float	check = 0;
	float	draw;
	float mp;


	// asign the check variable
	if (content->subtype == 1)
	{
		check = content->variable->value;
	}
	else if (content->subtype == 2)
	{
		if (content->iptr != NULL)
			check = **content->iptr;
		if (content->fptr != NULL)
			check = **content->fptr;
		if (content->dptr != NULL)
			check = **content->dptr;
	}

	if (menu_mouse_x || keydown[K_MWHEELUP] || keydown[K_MWHEELDOWN])
	{
		if (keydown[K_CTRL] && !keydown[K_SHIFT])
			mp = 1;
		else if (!keydown[K_CTRL] && keydown[K_SHIFT])
			mp = 0.01;
		else if (keydown[K_CTRL] && keydown[K_SHIFT])
			mp = 0.001;
		else
			mp = 0.1;

		if (menu_mouse_x != 0)
			value = menu_mouse_x * mp;
		else if (keydown[K_MWHEELUP])
			value = 1 *mp;
		else if (keydown[K_MWHEELDOWN])
			value = -1 *mp;

		if (content->subtype == 1){
			check += value;
		}
		
		if (content->subtype == 2 && content->fptr != NULL)
		{
			check += value;
		}

		if (content->subtype == 2 && content->dptr != NULL)
		{
			check += value;
		}

		if (content->subtype == 2 && content->iptr != NULL)
		{
			if (menu_mouse_x < 0 || keydown[K_MWHEELDOWN])
				check -= 1;
			if (menu_mouse_x > 0 || keydown[K_MWHEELUP])
				check += 1;
		}
	}

	
	// check if we are in limits
	if (check < content->limit[0])
		check = content->limit[0];

	if (check > content->limit[1])
		check = content->limit[1];


	draw = (check - content->limit[0])/(content->limit[1] - content->limit[0]);

	if (keydown[K_MOUSE1] || keydown[K_MWHEELUP] || keydown[K_MWHEELDOWN])
	{
		if (keydown[K_MOUSE1])	
				slider_active = 1;
		if (content->subtype == 1){
			Cvar_Set(content->variable,va("%f",check));
		}
		
		if (content->subtype == 2 && content->fptr != NULL){
			**content->fptr = check;
		}

		if (content->subtype == 2 && content->dptr != NULL){
			**content->dptr = (double)check;
		}

		if (content->subtype == 2 && content->iptr != NULL){
			**content->iptr = (int) check;
		}

		// position the cursor correctly
		if (keydown[K_MOUSE1])
		{
			mscx = content->lx + (content->width-1)*8 * draw + 12;
			mscy = content->ly + 4;
		}
	}
}

void MS_Handle_Content_Button(struct menu_ct *content){

	if (!keydown[K_MOUSE1])
	{
		content->pressed = 0;
		content->pressed_time = 0;
		return;
	}
	
	if (keydown[K_MOUSE1] && content->pressed)
		return;

	if (content->subtype == 1)
		Cbuf_AddText(content->name);
	if (content->subtype == 2)
		if (content->function)
			content->function();
	if (content->subtype == 3)
		if (content->update_function)
			content->update_function(content);
	content->pressed =1;
	content->pressed_time = cls.realrealtime;

}

void MS_Handle_Content_Editbox(struct menu_ct *content){

	if (!ssc && keydown[K_MOUSE1]){
		selected_editbox = content;
		memset(tmpstring,0,(sizeof(char)* 128));
		if (content->subtype == 1)
			strcpy(tmpstring,content->variable->string);
		if (content->subtype == 2){
			if (content->iptr != NULL){
				strcpy(tmpstring,va("%i",**content->iptr));
			}
			if (content->fptr != NULL){
				strcpy(tmpstring,va("%f",**content->fptr));
			}
			if (content->dptr != NULL){
				strcpy(tmpstring,va("%f",**content->dptr));
			}
		}

		ssc=1;
	}

}

void MS_Handle_Content(struct menu_ct *content)
{

	if (resizing || moving || popup_active || scrolling)
		return;

	if (!content)
	{
		return;
	}

	if (content->type != 2 && content->subtype != 3)
	{
		if (content->update_function)
		{
			content->update_function(content);
		}
	}

	if (content->type == 2)
		MS_Handle_Content_Button(content);
	if (content->type == 3)
		MS_Handle_Content_Slider(content);
	if (content->type == 4)
		MS_Handle_Content_Editbox(content);

	if (!kdmb1 && keydown[K_MOUSE1])
		S_LocalSound ("misc/menu1.wav");

}

void MS_Pressed_Animation(struct menu_ct *content)
{

	if (content->type == 2)
	{
		Draw_AlphaRectangleRealRGB(content->lx, content->ly, strlen(content->text)* 8 , 8, 1, 1, 0, 0, 1, 1);
	}
}


void MS_Hovertime_Animation(struct menu_ct *content)
{
	double a;

	a = cls.realrealtime - content->hovertime;
	a = 1-a;
	if (a <= 0.01)
	{
			content->hovertime = 0;
			return;
	}

	if (content->type == 4)
	{
		Draw_AlphaRectangleRealRGB(content->lx, content->ly, content->width * 8, 8, 1, 1, 0, 0, 1, a);
	}

	if (content->type == 3)
	{
		Draw_AlphaRectangleRealRGB(content->lx, content->ly, content->width * 8 + 16, 8, 1, 1, 0, 0, 1, a);
	}

	if (content->type == 2)
	{
		Draw_AlphaRectangleRealRGB(content->lx, content->ly, strlen(content->text)* 8 , 8, 1, 1, 0, 0, a, a);
	}
}

void MS_Selected_Animation(struct menu_ct *content)
{
	double a, r, g, b;

	a = 0.75;
	r = 0;
	g = 0;
	b = 1;

	if (content->type > 1)
		Draw_AlphaRectangleRealRGB(	content->lx , content->ly , content->pixel_width , content->pixel_height ,
									1, 0, r, g, b, a);

}

void Test_Draw(int x, int y, char *string);

void MS_Draw_Content_Text(struct menu_ct *content){

	if (content->subtype == 1)
	{
		Draw_String(content->lx,content->ly,content->text);
		//Test_Draw(content->lx, content->ly, content->text);
		content->rx = content->lx + strlen(content->text)*8;
	}
	else if (content->subtype == 2)
	{
		if (content->text_function)
		{
			Draw_String(content->lx, content->ly, content->text_function());
			//Test_Draw(content->lx, content->ly, content->text_function());
			content->rx = content->lx + strlen(content->text_function());

		}
	}
	content->ry = content->ly + content->pixel_height;
}


void MS_Draw_Content_Button(struct menu_ct *content){
	int py = 8;

	content->rx = content->lx + strlen(content->text) * 8;
	if ((content->color))
	{
		Draw_ColoredString(content->lx,content->ly,va("&c%s%s",content->color,content->text),0);
	}
	else
	{
		Draw_String(content->lx,content->ly,content->text);
	}
	content->rx = content->lx + strlen(content->text)*8;
	content->ry = content->ly + py;
}

void MS_Draw_Content_Picture (struct menu_ct *content){
	int px,py;
	if (content->picture)
	{
		px = content->picture->width;

		py = content->picture->height;

		content->rx = content->lx + px;
		content->ry = content->ly + py;
		content->rx = content->rx;
		content->ry = content->ry;

		//Draw_SAlphaSubPic(lx,ly,content->picture,content->picture->width,content->picture->height,px,py,1,1);
		Draw_SubPic(content->lx,content->ly,content->picture,0,0,px,py);
	}
}


void MS_Draw_Content_Slider (struct menu_ct	*content){
	float	check = 0;
	float	draw;
	int		px;
	int		py = 8;
	int		i;
	
	px = content->width * 8 + 16;

	if (content->subtype == 1)
		check = content->variable->value;
	if (content->subtype == 2)
	{
		if (content->iptr != NULL)
			check = **content->iptr;
		if (content->fptr != NULL)
			check = **content->fptr;
		if (content->dptr != NULL)
			check = **content->dptr;
	}


	//if (content->lx + px < content->rx)
	//{
		Draw_Character(content->lx,content->ly,128);
		for (i=1;i<=content->width ;i++)
			Draw_Character(content->lx + i *8,content->ly,129);
		Draw_Character (content->lx +i *8, content->ly, 130);

		if (check < content->limit[0])
			draw = 0;
		if (check > content->limit[1])
			draw = 1;
		draw = (check - content->limit[0])/(content->limit[1] - content->limit[0]);
		Draw_Character (content->lx + (content->width-1)*8 * draw + 8, content->ly, 131);
		
		content->rx = content->lx + px;
		content->ry = content->ly + py;
	//}
}

void MS_Draw_Content_Editbox (struct menu_ct *content){
	int		py = 8;
	int		i;
	char	buf[512];
	
	
	i = (content->rx - content->lx) / 8;

	// if we are the selected editbox draw a black background

	if (selected_editbox == content)
		Draw_AlphaRectangleRealRGB(content->lx, content->ly, content->width * 8, 8, 1, 1, 0.2, 0.2, 0.2, 1);
		

	if (content->subtype == 1)
		strcpy(buf,content->variable->string);

	if (content->subtype == 2)
	{
		if (content->iptr != NULL)
			strcpy(buf,va("%i",**content->iptr));
		if (content->fptr != NULL)
			strcpy(buf,va("%f",**content->fptr));
		if (content->dptr != NULL)
			strcpy(buf,va("%f",**content->dptr));
	}

	if (ssc){
		if (selected_editbox == content)
		{
			Draw_String(content->lx,content->ly,tmpstring);
		}
		else 
		{
			Draw_String(content->lx,content->ly,buf);
		}
	}else{
		Draw_String(content->lx,content->ly,buf);
	}


	content->ry = content->ly + py;
	content->rx = content->lx + 8 * content->width;

}


void MS_Draw_Content (struct menu_ct *content, int selected)
{
	
	if (selected)
		MS_Selected_Animation(content);

	if (content->hovertime)
		MS_Hovertime_Animation(content);

	if (content->pressed)
		MS_Pressed_Animation(content);

	if (content->type != 2 && content->type != 3)
		if (content->update_function)
			content->update_function(content);

	if (content->type == 1)
		MS_Draw_Content_Text(content);
	if (content->type == 2)
		MS_Draw_Content_Button(content);
	if (content->type == 3)
		MS_Draw_Content_Slider(content);
	if (content->type == 4)
		MS_Draw_Content_Editbox(content);
	if (content->type == 5)
		MS_Draw_Content_Picture(content);

}

void MS_Content_Update_Size(struct menu_ct *content)
{
	content->pixel_height = 8;

	if (content->type == 1)
	{
		if (content->text)
			content->pixel_width = strlen(content->text) * 8;
		else
			content->pixel_width = 1;
	}

	if (content->type == 2)
	{
		if (content->text)
			content->pixel_width = strlen(content->text) * 8;
		else
			content->pixel_width = 1;
	}

	if (content->type == 3)
	{
		content->pixel_width = content->width * 8 + 2 * 8;
	}

	if (content->type == 4)
	{
		content->pixel_width = content->width * 8;
	}

}

int MS_Disable_Selecting(void)
{
	if (resizing || scrolling || moving)
		return 1;
	return 0;
}

int MS_Is_Inside_Menu(struct menu_s *menu)
{
	if (mscx >= menu->lx + 1 && mscx <= menu->rx - 9 && mscy >= menu->ly + 9 && mscy <= menu->ry - 9)
	{
		return 1;
	}
	return 0;


}

void MS_Content(struct menu_s		*mm){
	int lx,ly,rx,ry,py;	// limits for the content
	char	buf[512];
	int		selected = 0;

	struct menu_ct	*content;
	struct menu_ct	*selected_content = NULL;
	int slider_x, slider_y;
	
	content = mm->content;
	

	slider_x = mm->slider_offset_x * ((mm->content_width - (mm->rx - mm->lx))/100.0f);
	if (slider_x < 0)
		slider_x = 0;
	slider_y = mm->slider_offset_y * ((mm->content_height - (mm->ry - mm->ly-22))/100.0f);
	if (slider_y < 0)
		slider_y = 0;


	while (content != NULL)
	{
		memset(buf,0,sizeof(char)*512);
		selected = 0;
		// the first content of the menu is always top left
		if (content->parent == NULL)
		{
			content->lx = mm->lx + 1 - slider_x;
			content->ly = mm->ly + 11 - slider_y;
			content->rx = mm->rx - 1 - slider_x;
			content->ry = mm->ry - 1 - slider_y;
		}
		else
		{
			// all other content relativ to the parent aligned to the right side
			if (content->align == 0 && content->parent)
			{
				content->lx = content->parent->rx;
				content->ly = content->parent->ly;
				content->rx = mm->rx -1 - slider_x;
				content->ry = content->parent->ry ;
			}
			else if (content->align == 1 && content->parent)
			{
				content->lx = content->parent->lx;
				content->ly = content->parent->ry;
				content->rx = mm->rx -1 - slider_x;
				content->ry = mm->ry -1 - slider_y;
			}
		}

		MS_Content_Update_Size(content);

		lx = content->lx;
		ly = content->ly;
		rx = content->rx;
		ry = content->ry;
		py = 8;
		if (content->lx + content->pixel_width > mm->content_width)
			mm->content_width = content->lx + content->pixel_width;
		if (content->ly + content->pixel_height > mm->content_height)
			mm->content_height = content->ly + content->pixel_height;

		
		//if (ly+8 >= mm->ry && rx +8 >= mm->rx)
		//	break;


		if (MS_Is_Inside_Menu(mm))
		if (!MS_Disable_Selecting())
		if (mscx < content->lx + content->pixel_width &&
			mscx > content->lx && 
			mscy > content->ly &&
			mscy < content->ly + content->pixel_height)
		{
				selected = 1;
				selected_content = content;
		}

		if (selected_content == content)
			MS_Draw_Content(content, 1);
		else
			MS_Draw_Content(content, 0);
		
		mm->minx = content->rx;
		mm->miny = content->ly;
	
		content = content->next;
	}

	if (selected_content != NULL){
		hoverend_content = selected_content;
		if (last_hovered != selected_content && resizing == 0 && moving == 0 && scrolling == 0)
		{
			selected_content->hovertime = cls.realrealtime;
			last_hovered = selected_content;
		}
		if (selected_content->tooltip != NULL){
			if (selected_content->type == 1 && selected_content->subtype == 1)
			{

			}
			else
			{
				MS_Draw_Tooltip(mscx,mscy,selected_content->tooltip,0.1,0.1,0.1,0.7);
			}
		}
		MS_Handle_Content(selected_content);
	}
	
}


void Draw_AlphaLineRealRGB (int x_start, int y_start, int x_end, int y_end, float thickness, float r, float g, float b, float a);
void MS_Draw_ActiveMenu(struct menu_s	*mm)
{
	int lx,ly,rx,ry;
	int x, y;


	if (!mm)
		return;

	lx = mm->lx;
	ly = mm->ly;
	rx = mm->rx;
	ry = mm->ry;

	if (mm->minimized){
		rx = strlen(mm->name)*8+22 + lx;
		ry = ly + 11;
	}
	// 
	Draw_AlphaRectangleRealRGB(lx, ly, rx - lx, ry - ly, 0, 1, 0, 0, 0,jogi_menu_system_background_alpha.value);
	// border
	Draw_AlphaRectangleRealRGB(lx, ly, rx - lx, ry - ly, 1, 0, jogi_menu_system_red.value, jogi_menu_system_green.value, jogi_menu_system_blue.value, jogi_menu_system_border_alpha.value);
	// a line ;]
	Draw_AlphaLineRealRGB(lx, ly+10, rx, ly+10 , 1, jogi_menu_system_red.value, jogi_menu_system_green.value, jogi_menu_system_blue.value, jogi_menu_system_border_alpha.value);
	// menu name
	Draw_String(lx+2,ly+1,mm->name);
	// close
	Draw_AlphaRectangleRealRGB(rx - 9, ly + 1 ,8,8,1,1, 1,0,0,1);
	// minimize
	Draw_AlphaRectangleRealRGB(rx - 17, ly + 1 ,8,8,1,1, 0,0,1,1);

	// slider_x 
	if (mm->content_width > rx - lx - 10)
	{
		if (mm->content_width > 0)
		{

			x = rx - lx - 10; //100
			x = x *  (float)(x/(float)(mm->content_width));
			
			y = rx - lx - 10;
			y = y - x;
			y = y * (float)(mm->slider_offset_x/100.0f);

		}
		else
		{
			x = 1;
			y = 1;
		}
		Draw_AlphaRectangleRealRGB(lx + y + 1,ry - 9 , x,8,1,1, 0,0,1,1);
	}


	// slider_y 
	if (mm->content_height > ry - ly - 22)
	{
		if (mm->content_height> 0)
		{

			x = ry - ly - 22; //100
			x = x *  (float)(x/(float)(mm->content_height));
			
			y = ry - ly - 22;
			y = y - x;
			y = y * (float)(mm->slider_offset_y/100.0f);
			

		}
		else
		{
			x = 1;
			y = 1;
		}
		Draw_AlphaRectangleRealRGB(rx - 9, ly +10 + y , 8,x,1,1, 0,0,1,1);
	}





	if (!mm->minimized)
		Draw_AlphaRectangleRealRGB(rx - 9, ry - 9,8,8,0,1,1,1,0,1);

	Draw_EnableScissorRectangle(lx+1, ly+11, rx - lx -9, ry - ly-19);

	if (mm->content && !mm->minimized)
	{
		MS_Content(mm);
	}
		// resize
	
	
	Draw_DisableScissor();

}
int MS_Popup_Menu(void);
void MS_Popup_Menu_Draw(void);
void MS_Draw(void)
{
	if (!jogi_menu_system.value)
		return;

	MS_Popup_Menu_Draw();

	if (!MS_Popup_Menu())
		MS_Draw_ActiveMenu(menu_active);
	MS_Mouse();
	
	

}



void Menu_Mainhook(void){

	

}

struct menu_s *MS_Add_Menu(int lx, int ly, int rx, int ry, char *name, char *identifier, void (*function)(struct menu_s *self))
{
	struct menu_s	*lastmenu = NULL;
	struct menu_s	*my_menu;


	if (menu != NULL){
		lastmenu = menu;
		while (lastmenu){
			if (!strcmp(lastmenu->identifier,identifier)){
				Com_Printf("A menu with the identifier \"%s\" does already exist\n");
				return NULL;
			}
			if (lastmenu->next == NULL)
				break;
			lastmenu = lastmenu->next;
		}
	}

	my_menu = calloc(1,sizeof(struct menu_s));
	if (!my_menu)
		return NULL;
	
	my_menu->lx = lx;
	my_menu->ly = ly;
	my_menu->rx = rx;
	my_menu->ry = ry;

	my_menu->name = strdup(name);
	if (!my_menu->name)
		return NULL;

	my_menu->identifier = strdup(identifier);
	if (!my_menu->identifier)
		return NULL;
	
	if (my_menu->update_function)
		my_menu->update_function(my_menu);



	if (menu == NULL)
		menu=my_menu;
	else if (lastmenu != NULL && menu != NULL)
		lastmenu->next = my_menu;

	return my_menu;

}

void MS_Set_Content_Lua_State(struct menu_ct *ct, void *ls)
{
	ct->lua_state = ls;
}

void *MS_Get_Content_Lua_State(struct menu_ct *ct)
{
	return ct->lua_state;
}

char *MS_Get_Content_Identifier(struct menu_ct *ct)
{
	return ct->identifier;
}

struct menu_ct *MS_Add_Content(char *mid, char *pid, char *identifier, int type, int subtype, int align, char *text, char *tooltip, char *name, char *color, mpic_t *picture, cvar_t *variable, int width , float limmin, float limmax, char *ptype, void *vptr, void (*update_function)(struct menu_ct *self))
{
	struct menu_s		*my_menu;
	struct menu_ct	*my_content = NULL;
	struct menu_ct	*found_content = NULL;
	struct menu_ct	*my_parent = NULL;
	int		found	= 0;
	
	my_menu = menu;
	
	if (mid == NULL || identifier == NULL){
		Com_Printf("We need a parent identifier !\n");
		return NULL;
	}

	while (my_menu){
		if (!strcmp(mid,my_menu->identifier)){
			found = 1;
			break;
		}
		if (my_menu->next == NULL)
			break;
		my_menu=my_menu->next;
	}
	if (!found)
	{
		Com_Printf("A menu with the identifier \"%s\" could not be found.\n",mid);
		return NULL;
	}

	

	if (my_menu->content != NULL && pid != NULL){
		my_content = my_menu->content;
		while (my_content){
			if (!strcmp(my_content->identifier,pid)){
				my_parent = my_content;
			}
			if (my_content->next == NULL)
				break;
			my_content = my_content->next;
		}
	}
	if (my_parent == NULL && pid != NULL)
	{
		return NULL;
	}
	
	if (my_menu->content == NULL && pid == NULL && found_content == NULL)
	{
		my_menu->content = calloc(1,sizeof(struct menu_ct));
		if (my_menu->content == NULL)
			return NULL;
		my_content = my_menu->content;
	}
	else if (my_content != NULL)
	{
		my_content->next = calloc(1,sizeof(struct menu_ct));
		if (my_content->next == NULL)
			return NULL;
		my_content = my_content->next;
	}

	my_content->parent = my_parent;

	my_content->lx = 0;
	my_content->ly = 0;
	my_content->rx = 0;
	my_content->ry = 0;
	my_content->type		= type;
	my_content->subtype		= subtype;
	my_content->align		= align;
	my_content->picture		= picture;
	my_content->variable	= variable;
	my_content->width		= width;
	my_content->limit[0]	= limmin;
	my_content->limit[1]	= limmax;
	//my_content->function	= function;

	my_content->identifier = strdup(identifier);

	if (my_content->identifier == NULL)
	{
		if (my_parent)
		{
			my_parent->next = NULL;

		}
		if (pid == NULL)
		{
			my_menu->content = NULL;
		}
		free(my_content);
		return NULL;
	}
	

	if (name)
		my_content->name = strdup(name);

	if (text)
		my_content->text = strdup(text);
	
	if (tooltip)
		my_content->tooltip = strdup(tooltip);

	if (color)
		my_content->color = strdup(color);
	
	if (ptype == "i")
		my_content->iptr = (int **)vptr;
	
	if (ptype == "f")
		my_content->fptr = (float **)vptr;

	if (ptype == "d")
		my_content->dptr = (double **)vptr;

	if (my_content->type == 2 && my_content->subtype == 2)
	{
		my_content->function = vptr;
	}

	if (my_content->type == 1 && my_content->subtype == 2)
	{
		my_content->text_function = vptr;
	}

	my_content->update_function = update_function;
	
	return my_content;
}



void MS_Activate_Menu(void){
	struct menu_s	*lastmenu = NULL;
	struct menu_s	*my_menu = NULL;
	
	if (menu != NULL)
	{
		lastmenu = menu;
		while (lastmenu)
		{
			if (!strcmp(lastmenu->identifier,Cmd_Argv(1)))
			{
				//Com_Printf("A menu with the identifier \"%s\" does already exist\n");
				my_menu = lastmenu;
				break;
			}
			lastmenu = lastmenu->next;
		}
	}
	if (my_menu == NULL)
	{
		Com_Printf("Menu does not exist %s\n", Cmd_Argv(1));
		return;
	}
	menu_active = my_menu;
}

void MS_Activate_Menu_Struct(struct menu_s *menu)
{
	if (menu)
		menu_active = menu;
}
	
struct menu_ct *MS_Add_Content_Text(char *mid,char *pid,char *id,int align,char *text)
{
	return MS_Add_Content(mid,pid,id,1,1,align,text,NULL,NULL,NULL,NULL,NULL,0,0,0,NULL,NULL, NULL);
}


struct menu_ct *MS_Add_Content_Slider(char *mid, char *pid, char *id, int subtype, int align, char *ptype, void *vptr,float llim,float mlim){
	

	if (subtype == 1)
		return MS_Add_Content(mid,pid,id,3,subtype,align,NULL,NULL,NULL,NULL,NULL,(cvar_t *)vptr,20,llim,mlim,NULL,NULL, NULL);
	if (subtype == 2)
		return MS_Add_Content(mid,pid,id,3,subtype,align,NULL,NULL,NULL,NULL,NULL,NULL,20,llim,mlim,ptype,vptr, NULL);
	return NULL;
}

struct menu_ct *MS_Add_Content_Slider_Width(char *mid, char *pid, char *id, int subtype, int align, char *ptype, void *vptr,float llim,float mlim, int width)
{
	if (subtype == 1)
		return MS_Add_Content(mid,pid,id,3,subtype,align,NULL,NULL,NULL,NULL,NULL,(cvar_t *)vptr,width,llim,mlim,NULL,NULL, NULL);
	if (subtype == 2)
		return MS_Add_Content(mid,pid,id,3,subtype,align,NULL,NULL,NULL,NULL,NULL,NULL,width,llim,mlim,ptype,vptr, NULL);
	return NULL;
}

void MS_Console_Create_Menu(void)
{
	MS_Add_Menu(10,10,200,200,Cmd_Argv(1),Cmd_Argv(2), NULL);
}

void MS_Console_Add_Content_Slider(void)
{
	cvar_t	*c = NULL;
	
	c = Cvar_Find(Cmd_Argv(4));

	if (!c)
		return;
	if (strlen(Cmd_Argv(2)))
		MS_Add_Content_Slider(Cmd_Argv(1),Cmd_Argv(2),Cmd_Argv(3),1,1,NULL,c,-100,100);
	else
		MS_Add_Content_Slider(Cmd_Argv(1),NULL,Cmd_Argv(3),1,1,NULL,c,-100,100);


}

void Menu_Init(void){
	
	// Cvars
	Cvar_Register(&jogi_menu_system);
	Cvar_Register(&jogi_menu_system_debug);
	Cvar_Register(&jogi_menu_system_border_alpha);
	Cvar_Register(&jogi_menu_system_background_alpha);
	Cvar_Register(&jogi_menu_system_red);
	Cvar_Register(&jogi_menu_system_green);
	Cvar_Register(&jogi_menu_system_blue);
	Cvar_Register(&new_menu_sensitivity);
	Cmd_AddCommand("MS_activate_menu",MS_Activate_Menu);
	Cmd_AddCommand("MS_Console_Create_Menu",MS_Console_Create_Menu);
	Cmd_AddCommand("MS_Console_Add_Content_Slider",MS_Console_Add_Content_Slider);

}

void MS_CT_Change_Text(struct menu_ct *content, char *text)
{
	if (!content || !text)
		return;

	free(content->text);
	content->text = strdup(text);

}

void MS_CT_Change_Limits(struct menu_ct *content, int lower, int upper)
{
	if (!content)
		return;

	content->limit[0] = lower;
	content->limit[1] = upper;
}


// Popup Menu



struct MS_Popup_Entry
{
	struct MS_Popup_Entry *next, *prev;
	struct MS_Popup_Entry *sub_menu;
	char *name;
	char *console_command;
	void (*function)(void *data);
	void *data;
};

struct MS_Popup_Menu_Menu
{
	struct MS_Popup_Entry *entry;
	int x,y, type;
	struct MS_Popup_Menu_Menu *next, *prev;

	void (*function)(void *data);
	void *data;
};


struct MS_Popup_Menu_Menu MS_Popup_Menu_Root;
struct MS_Popup_Menu_Menu *popup_menus;

int popup_x, popup_y;
struct MS_Popup_Entry *popup_active_entry;
struct MS_Popup_Menu_Menu *popup_menu_active;

void Activate_Popup_Menu(void)
{
	popup_x = mscx;
	popup_y = mscy;
	popup_active = 1;
}
void Deactivate_Popup_Menu(void)
{
	popup_active = 0;
	popup_menu_active = NULL;
	if (popup_active_entry)
	{
		if (popup_active_entry->console_command)
			Cbuf_AddText(va("%s\n",popup_active_entry->console_command));
		if (popup_active_entry->function)
			popup_active_entry->function(popup_active_entry->data);
	}
}

#define MSP_RADIUS 5

int MS_Popup_Valid(struct MS_Popup_Menu_Menu *menu)
{
	struct MS_Popup_Menu_Menu *lmenu;
	lmenu = popup_menus;
	while (lmenu)
	{
		if (lmenu == menu)
			return 1;
		lmenu = lmenu->next;
	}
	return 0;
}

int MS_Popup_Menu(void)
{
	int i = 0;
	int x = 0;
	int width = -1;
	int dx, dy;
	struct MS_Popup_Entry *entry;
	struct MS_Popup_Menu_Menu *menu = NULL;

	if (!popup_menu_active && menu_active)
	{
		if (menu_active->lx <= mscx &&  menu_active->rx >= mscx)
			if (menu_active->ly <= mscy && menu_active->ry >= mscy)
				return popup_active;
	}

	if (popup_menus && !popup_menu_active)
	{
		menu = popup_menus;
		while (menu)
		{
			dx = menu->x - mscx;
			dy = menu->y - mscy;
			if (dx < MSP_RADIUS && dx > -MSP_RADIUS && dy < MSP_RADIUS && dy > -MSP_RADIUS)
				break;
			menu = menu->next;
		}

		if (menu)
		{
			MS_Draw_Tooltip(menu->x, menu->y, "press and hold right mousebutton", 0, 0, 1, 1);
		}

	}


	if (!popup_active)
		return popup_active;
	
	if (MS_Popup_Valid(popup_menu_active))
		menu = popup_menu_active;
	else
		if (!menu)
			menu = &MS_Popup_Menu_Root;
	

	if (menu->type == 1)
	{
		popup_active_entry = menu->entry;
		Deactivate_Popup_Menu();
		return popup_active;
	}
	
	entry = menu->entry;

	if (!entry)
		return popup_active;

	while (entry)
	{
		i = strlen(entry->name) * 8;
		if (i > width)
			width = i;
		x++;
		entry = entry->next;
	}

	Draw_AlphaRectangleRealRGB(popup_x - 1, popup_y - 1, width+2, x*8+2, 1, 1, 0, 0, 0, 1);
	Draw_AlphaRectangleRealRGB(popup_x - 1, popup_y - 1, width+2, x*8+2, 1, 0, 0, 1, 0, 1);

	i = 0;
	popup_active_entry = NULL;
	entry = menu->entry;
	while (entry)
	{
		if (mscx >= popup_x && mscx <= popup_x + width && mscy > popup_y + i*8 && mscy < popup_y + 8 + i*8)
		{
			popup_active_entry = entry;
			Draw_ColoredString(popup_x, popup_y + i++ * 8, va("&c00f%s",entry->name), 0);
		}
		else
			Draw_String(popup_x, popup_y + i++ * 8, entry->name);
		entry = entry->next;
	}

	popup_menu_active = menu;

	return popup_active;
}

struct MS_Popup_Entry *MS_Popup_Add_Entry(struct MS_Popup_Menu_Menu *menu, struct MS_Popup_Entry *entry, char *name, void (*f)(void *data), void *data, char *command)
{
	struct MS_Popup_Entry *lentry, *eentry;

	if (entry == NULL && menu == NULL)
		return NULL;

	lentry = calloc(1, sizeof(struct MS_Popup_Entry));
	if (!lentry)
		return NULL;

	if (menu)
	{
		if (menu->entry)
		{
			eentry = menu->entry;
			while (eentry->next)
				eentry = eentry->next;
			eentry->next = lentry;
			lentry->prev = eentry;
		}
		else
			menu->entry = lentry;
	}

	if (entry)
	{
		entry->next = lentry;
		lentry->prev = entry;
	}

	lentry->function = f;
	lentry->data = data;
	if (command)
		lentry->console_command = strdup(command);
	if (name)
		lentry->name = strdup(name);
	return lentry;
}



struct MS_Popup_Menu_Menu *MS_Popup_Add_Menu(void)
{
	struct MS_Popup_Menu_Menu *lentry, *eentry;

	lentry = calloc(1, sizeof(struct MS_Popup_Menu_Menu));
	if (!lentry)
		return NULL;

	if (popup_menus == NULL)
		popup_menus = lentry;
	else
	{
		eentry = popup_menus;
		while (eentry->next)
			eentry = eentry->next;
		eentry->next = lentry;
		lentry->prev = eentry;
	}

	return lentry;
}

void MS_Popup_Cleanup_Entry(struct MS_Popup_Entry *entry)
{
	struct MS_Popup_Entry *lentry, *llentry;

	lentry = entry;

	while (lentry)
	{
		free(lentry->name);
		free(lentry->console_command);
		free(lentry->data);

		llentry = lentry;
		lentry = lentry->next;
		free(llentry);
	}

}

void MS_Popup_Cleanup_Menu(struct MS_Popup_Menu_Menu *menu)
{
	struct MS_Popup_Entry *entry;
	if (menu->entry)
	{
		entry = menu->entry;
		MS_Popup_Cleanup_Entry(entry);
	}
	free(menu);


}

void MS_Popup_Remove_Menu(struct MS_Popup_Menu_Menu *menu)
{

	if (!menu)
		return;

	if (!menu->prev && !menu->next)
		popup_menus = NULL;
	else if (!menu->prev && menu->next)
	{
		popup_menus = menu->next;
		menu->next->prev = NULL;
	}
	else if (menu->prev && !menu->next)
	{
		menu->prev->next = NULL;
	}
	else if (menu->prev && menu->next)
	{
		menu->prev->next = menu->next;
		menu->next->prev = menu->prev;
	}
	MS_Popup_Cleanup_Menu(menu);

}

void MS_Popup_Menu_Set(struct MS_Popup_Menu_Menu *menu, int x, int y)
{
	if (!menu)
		return;

	menu->x = x;
	menu->y = y;
}

void MS_Popup_Menu_Draw(void)
{
	struct MS_Popup_Menu_Menu *menu;
	int c_r, c_b;
	int z;


	menu = popup_menus;
	c_r = RGBA_TO_COLOR(255, 0, 0, 255);
	c_b = RGBA_TO_COLOR(0, 0, 255, 255);

	
	z = 0;
	while (menu)
	{
		if (menu->type == 0)
			Draw_AlphaCircleFillRGB(menu->x, menu->y, MSP_RADIUS -2, c_r);
		else
			Draw_AlphaCircleFillRGB(menu->x, menu->y, MSP_RADIUS -2, c_b);
		menu = menu->next;
		z++;
	}

}

void MS_Popup_Menu_Set_Type(struct MS_Popup_Menu_Menu *menu, int type)
{
	if (!menu)
		return;

	menu->type = type;

}
