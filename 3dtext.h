void Draw_3D_Text(vec3_t position, vec3_t orientation, int x, int y,char *text, int type);
void Draw_3D_Text_Position(vec3_t position, vec3_t destination, int x, int y, char *text, int type, float scale);
void Draw_3D_Text_Pop_Matrix(void);
void Draw_3D_Text_Position_Setup_Matrix(vec3_t position, vec3_t destination, float scale);
void Draw_3D_Text_Setup_Drawable_Rectangle(int width, int height);
void Draw_3D_Text_Remove_Drawable_Rectangle(void);