#include "quakedef.h"
#include "mvd_cam.h"
#include <time.h>
#include <string.h>
#include "gl_model.h"
#include "gl_local.h"


void Draw_3D_Text(vec3_t position, vec3_t orientation, int x, int y, char *text, int type)
{
	if (text == NULL)
		return;

	glPushMatrix();
	glTranslatef(position[0], position[1], position[2]);
	glRotatef(-90, 1, 0, 0);
	Draw_ColoredString(x, y, text, 0);
	glPopMatrix();

	return;
}

void Draw_3D_Text_Scale(vec3_t position, vec3_t orientation, int x, int y, char *text, int type, float scale)
{
	if (text == NULL)
		return;

	glPushMatrix();
	glTranslatef(position[0], position[1], position[2]);
	glRotatef(-90, 1, 0, 0);
	glScalef(scale, scale, scale);
	Draw_ColoredString(x, y, text, 0);
	glPopMatrix();

	return;
}


void mvd_vectoangles(vec3_t vec, vec3_t ang);

void Draw_3D_Text_Position(vec3_t position, vec3_t destination, int x, int y, char *text, int type, float scale)
{
	vec3_t	vec1, vec2, vec3,s, u, angles, billboard[4];
	float m[16];
	if (text == NULL)
		return;



	VectorSubtract(destination, position, vec1);
	VectorCopy(vec1, vec2);
	VectorNormalize(vec2);
	VectorCopy(vup, vec3);
	VectorNormalize(vec3);
	CrossProduct(vec1, vec3,s);
	CrossProduct(s, vec1,u);
	VectorNormalize(s);
	VectorNormalize(u);

	m[0] = s[0]; m[1] = s[1]; m[2] = s[2]; m[3] = 0; 
	m[4] = u[0]; m[5] = u[1]; m[6] = u[2]; m[7] = 0;
	m[8] = -vec2[0]; m[9] =  -vec2[1]; m[10] = -vec2[2]; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;



	glPushMatrix();
	glTranslatef(position[0], position[1], position[2]);

	glMultMatrixf(m);
	glRotatef(180, 0, 0, 1);

	glScalef(scale, scale, scale);
	Draw_ColoredString(x, y, text, 0);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glColor4f(1,0,0,1);
	glVertex3f(0,0,0);
	glVertex3f(100,0, 0);
	glEnd();

	glBegin(GL_LINES);
	glColor4f(0,1,0,1);
	glVertex3f(0,0,0);
	glVertex3f(0,100, 0);
	glEnd();

	glBegin(GL_LINES);
	glColor4f(0,0,1,1);
	glVertex3f(0,0,0);
	glVertex3f(0,0,100);
	glEnd();


	glEnable(GL_TEXTURE_2D);
	glPopMatrix();

	return;
}
void Draw_3D_Text_Pop_Matrix(void)
{
	glPopMatrix();
}
void Draw_3D_Text_Position_Setup_Matrix(vec3_t position, vec3_t destination, float scale)
{
	vec3_t	vec1, vec2, vec3,s, u, angles, billboard[4];
	float m[16];



	VectorSubtract(destination, position, vec1);
	VectorCopy(vec1, vec2);
	VectorNormalize(vec2);
	VectorCopy(vup, vec3);
	VectorNormalize(vec3);
	CrossProduct(vec1, vec3,s);
	CrossProduct(s, vec1,u);
	VectorNormalize(s);
	VectorNormalize(u);

	m[0] = s[0]; m[1] = s[1]; m[2] = s[2]; m[3] = 0; 
	m[4] = u[0]; m[5] = u[1]; m[6] = u[2]; m[7] = 0;
	m[8] = -vec2[0]; m[9] =  -vec2[1]; m[10] = -vec2[2]; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;



	glPushMatrix();
	glTranslatef(position[0], position[1], position[2]);

	glMultMatrixf(m);
	glRotatef(180, 0, 0, 1);

	glScalef(scale, scale, scale);
	return;
}

void Draw_3D_Text_Setup_Drawable_Rectangle(int width, int height)
{

	glClear(GL_STENCIL_BUFFER_BIT);
	glClearStencil(0);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0x0, 0x0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);


	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glColor4f(0,0,0,0);
	glRectf(0, 0, width, height);
	glEnable(GL_ALPHA_TEST);

	glStencilFunc(GL_EQUAL, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
}

void Draw_3D_Text_Remove_Drawable_Rectangle(void)
{
			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_STENCIL_TEST);
}
