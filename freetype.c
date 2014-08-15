#include "quakedef.h"
#include "gl_model.h"
#include "gl_local.h"
#include "gl_texture.h"

#include "sys_lib.h"

#include "ft2build.h"
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

/*
 * Dynamic loading
 */

FT_UInt		(*qwft_get_char_index)			(FT_Face face, FT_ULong charcode);
FT_Error	(*qwft_load_glyph)			(FT_Face face, FT_UInt glyph_index, FT_Int32 load_flags);
FT_Error	(*qwft_get_glyph)			(FT_GlyphSlot slot, FT_Glyph *aglyph);
FT_Error	(*qwft_glyph_to_bitmap)			(FT_Glyph *the_glyph, FT_Render_Mode render_mode, FT_Vector *origin, FT_Bool destroy);
FT_Error	(*qwft_init_freetype)			(FT_Library *alibrary);
FT_Error	(*qwft_new_face)			(FT_Library library, const char *filepathname, FT_Long face_index, FT_Face *aface);
FT_Error	(*qwft_set_char_size)			(FT_Face face, FT_F26Dot6  char_width, FT_F26Dot6  char_height, FT_UInt horz_resolution, FT_UInt vert_resolution);
FT_Error	(*qwft_done_face)			(FT_Face face);
FT_Error	(*qwft_done_freetype)			(FT_Library library);


struct lib_functions ft_functions[] =
	{
	{	"FT_Get_Char_Index",	(void *)&qwft_get_char_index},
	{	"FT_Load_Glyph", 	(void *)&qwft_load_glyph},
	{	"FT_Get_Glyph", 	(void *)&qwft_get_glyph},
	{	"FT_Glyph_To_Bitmap", 	(void *)&qwft_glyph_to_bitmap},
	{	"FT_Init_FreeType", 	(void *)&qwft_init_freetype},
	{	"FT_New_Face",		(void *)&qwft_new_face},
	{	"FT_Set_Char_Size",	(void *)&qwft_set_char_size},
	{	"FT_Done_Face",		(void *)&qwft_done_face},
	{	"FT_Done_FreeType",	(void *)&qwft_done_freetype},
	{NULL}
	};

	


cvar_t	freetype_tex = {"freetype_tex" , "1"};
cvar_t	freetype_x = {"freetype_x" , "1"};
cvar_t	freetype_y = {"freetype_y" , "1"};
cvar_t	freetype_x_scale = {"freetype_x_scale" , "0.4"};
cvar_t	freetype_y_scale = {"freetype_y_scale" , "0.4"};
cvar_t	freetype_spacing = {"freetype_spacing" , "1"};
cvar_t	freetype_console_placement = {"freetype_console_placement" , "-22"};

int current_font_size = 20	;

inline int next_p2 (int a )
{
	int rval=1;
	// rval<<=1 Is A Prettier Way Of Writing rval*=2; 
	while(rval<a) rval<<=1;
	return rval;
}

/*
inline void pushScreenCoordinateMatrix()
{
	glPushAttrib(GL_TRANSFORM_BIT);
	GLint   viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
	glPopAttrib();
}

inline void pop_projection_matrix()
{
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}
*/

struct font_list_t	
{
	struct font_list_t *next;
	GLuint		*textures;			// link to the 128 textures
	GLuint		list_base;
	float		h;
	char		*name;					// font name
};

static struct font_list_t *Freetype_Fonts = NULL;

static void Create_Textures (FT_Face face, char ch, GLuint list_base, GLuint *tex_base)
{
	FT_Glyph glyph;
	FT_Bitmap	bitmap;
	FT_BitmapGlyph	bitmap_glyph;
	GLubyte *expanded_data;
	int		j,i;

	if (qwft_load_glyph(face, qwft_get_char_index(face, ch), FT_LOAD_DEFAULT))
		return;

	
	if (qwft_get_glyph(face->glyph, &glyph))
		return;

	printf("qwft_glyph_to_bitmap: %i\n", qwft_glyph_to_bitmap(&glyph, ft_render_mode_normal, 0, 1));
	printf("FT_BitmapGlyph: %p\n", bitmap_glyph = (FT_BitmapGlyph)glyph);
	bitmap=bitmap_glyph->bitmap;

	int width = next_p2( bitmap.width );
	int height = next_p2( bitmap.rows );

	expanded_data = malloc( 2 * width * height);

	for(j=0; j <height;j++) {
		for(i=0; i < width; i++){
			expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] = 
				(i>=bitmap.width || j>=bitmap.rows) ?
				0 : bitmap.buffer[i + bitmap.width*j];
		}
	}

	glBindTexture( GL_TEXTURE_2D, tex_base[(int)ch]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);


	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

	// With The Texture Created, We Don't Need The Expanded Data Anymore.
	free( expanded_data);

	glNewList(list_base+ch,GL_COMPILE);

	glBindTexture(GL_TEXTURE_2D,tex_base[(int)ch]);

	glPushMatrix();

	glTranslatef(bitmap_glyph->left,0,0);

	glTranslatef(0,bitmap_glyph->top-bitmap.rows,0);

	float   x=(float)bitmap.width / (float)width,
	y=(float)bitmap.rows / (float)height;

	glColor3f(1,1,1);

	glBegin(GL_QUADS);
	glTexCoord2d(0,0); glVertex2f(0,bitmap.rows);
	glTexCoord2d(0,y); glVertex2f(0,0);
	glTexCoord2d(x,y); glVertex2f(bitmap.width,0);
	glTexCoord2d(x,0); glVertex2f(bitmap.width,bitmap.rows);
	glEnd();
	glTranslatef(face->glyph->advance.x >> 6 ,0,0);
	glBitmap(0,0,0,0,face->glyph->advance.x >> 6,0,NULL);

	glPopMatrix();
	glEndList();

	glNewList(list_base+ch+128,GL_COMPILE);
	glPushMatrix();

	glBindTexture(GL_TEXTURE_2D,tex_base[(int)ch]);


	glTranslatef(bitmap_glyph->left,0,0);

	glTranslatef(0,bitmap_glyph->top-bitmap.rows,0);

	glColor3f(1,0,0);

	glBegin(GL_QUADS);
	glTexCoord2d(0,0); glVertex2f(0,bitmap.rows);
	glTexCoord2d(0,y); glVertex2f(0,0);
	glTexCoord2d(x,y); glVertex2f(bitmap.width,0);
	glTexCoord2d(x,0); glVertex2f(bitmap.width,bitmap.rows);
	glEnd();
	glPopMatrix();
	glTranslatef(face->glyph->advance.x >> 6 ,0,0);
	glBitmap(0,0,0,0,face->glyph->advance.x >> 6,0,NULL);
	glEndList();
}

struct font_list_t *Freetype_Add_Font(void)
{
	struct font_list_t *f, *entry;

	f = calloc(1, sizeof(struct font_list_t));

	if (Freetype_Fonts == NULL)
	{
		Freetype_Fonts = f;
	}
	else
	{
		entry = Freetype_Fonts;
		while (entry->next)
			entry = entry->next;
		entry->next = f;
	}

	return f;
}

struct font_list_t *Freetype_Get_Font (char *name)
{
	struct font_list_t *font;

	font = Freetype_Fonts;

	while (font)
	{
		if (strcmp(font->name, name) == 0)
			return font;
		font = font->next;
	}

	return NULL;

}

void Freetype_Unload (struct font_list_t *f)
{
	if (!f)
		return;

	free(f->name);
	glDeleteTextures(128, f->textures);
	if (f == Freetype_Fonts)
		Freetype_Fonts = f->next;
	free(f);
}

void Freetype_Load(char *fname, int h)
{
	GLuint *textures;
	FT_Library library;
	FT_Face face;
	struct font_list_t	*f;

	unsigned char i;
	int	j;
	int	k;
	k = h;
	
	Com_Printf("Freetype_Load called\n");
	
	printf("qwft_init_freetype: %i\n", qwft_init_freetype(&library));
	if ((j = qwft_new_face(library, va("fonts/%s.ttf", fname), 0, &face)))
	{
			Com_Printf("Loading the font \"%s\" failed\n",fname);
			if (j == FT_Err_Unknown_File_Format)
			{
				printf("failed unknown file format\n");
			}
			printf("failed %s %i\n",fname,j);
			return;
	}

	printf("qwft_set_char_size: %i\n", qwft_set_char_size(face, k*64, k*64, 96,96));

	f = Freetype_Add_Font();

	if (f == NULL)
		return;

	f->list_base = glGenLists(256);
	f->name = strdup(fname);
	textures = malloc(sizeof(GLuint) * 128);
	glGenTextures(128, textures);
	f->textures = textures;
	f->h = k;

	for(i=0;i<128;i++)
		Create_Textures(face,i,f->list_base,textures);
	
	qwft_done_face(face);

	qwft_done_freetype(library);

}

void Freetype_Print(struct font_list_t *ft_font, int x, int y, char *str)
{
	char	*ptr;

	if (ft_font == NULL)
		return;
	if (str == NULL)
		return;

	float h = ft_font->h/.63f;
	char	text[1024];
	char *index = &text[0];

	ptr = str;
	while (*ptr != '\0' && *ptr != '\n')
	{
		if ((char *)*ptr < 0)
			*index = *ptr + 256;
		else
			*index = *ptr;
		index++;ptr++;
	}
	index = &text[0];

	glEnable(GL_TEXTURE_2D);

	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT); 
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glColor3f(1.0f, 1.0f, 1.0f);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      

	glListBase(ft_font->list_base);

	glPushMatrix();
	
	glLoadIdentity();
	glTranslatef(x+h + freetype_x.value,y-h+freetype_y.value,0);
	glRotatef(180,0,0,1);
	glRotatef(180,0,1,0);
	glScalef(freetype_x_scale.value,freetype_y_scale.value,0);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, (GLvoid*)index);
	glPopMatrix();
	glPopAttrib();          
	glDisable(GL_BLEND);

	//return &len;
}

void Freetype_Frame(void)
{
}

void Freetype_Console_Load(void)
{
	static int x;
	static char *y;
	x = atoi(Cmd_Argv(2));
	y = strdup(Cmd_Argv(1));
	Freetype_Load(y,x);
	free(y);
}

struct font_list_t *test;

void Freetype_Init(void)
{
	Com_Printf("freetype initiated\n");
//		x=80;	this worked and looked good
	//x=160;
	Freetype_Load("arial", 50);
	Cvar_Register(&freetype_tex);
	Cvar_Register(&freetype_x);
	Cvar_Register(&freetype_y);
	Cvar_Register(&freetype_x_scale);
	Cvar_Register(&freetype_y_scale);
	Cvar_Register(&freetype_spacing);
	Cvar_Register(&freetype_console_placement);
	test = Freetype_Get_Font("arial");
	if (test == NULL)
		Com_Printf("Could not load default font arial.\n");
}

void Test_Draw(int x, int y, char *string)
{
	if (test == NULL)
		return;
	Freetype_Print(test, x, y, string);
}

int Freetype_LoadLibs(void)
{
#ifdef WIN32
	return (LIB_Load("freetype.dll", ft_functions, true));
#else
	return (LIB_Load("libfreetype.so", ft_functions, true));
#endif
}

void Freetype_Init1 (void)
{
	if (Freetype_LoadLibs())
	{
		Com_Printf("could not load freeype.\n");
		return;
	}
	else
	{
		Com_Printf("loaded freetype.\n");
	}

	Cmd_AddCommand("freetype_load",Freetype_Console_Load);
	Freetype_Init();
}

void Freetype_GFX_Init(void)
{
	if (Freetype_Fonts)
	{
		while (Freetype_Fonts)
		{
			Freetype_Unload(Freetype_Fonts);
		}
		Freetype_Load("arial", 50);
		test = Freetype_Get_Font("arial");
	}
}
