struct spline 
{
	void *test;
};

struct spline_point
{
	void *test;
};

struct spline *Spline_Add(void *data);
void Spline_Remove(struct spline *spline);
int Spline_Get_Point_Count(struct spline *spline);
void Spline_Remove_Point_By_Number(struct spline *spline, int number);
void *Spline_Get_Point_Data_By_Number(struct spline *spline, int point);
void Spline_Even_Speed(struct spline *spline);
float *Spline_Get_Point_Origin_By_Number(struct spline *spline, int point);
double *Spline_Get_Point_Time_By_Number(struct spline *spline, int point);
void Spline_Check_Selected_Point_Time(struct spline *spline, int point);
void Spline_Update(struct spline *spline);
int Spline_Calculate_From_Point(struct spline *spline, double time, vec3_t vec, int point);
void Spline_Get_Point_Position(struct spline *spline, int point_number, vec3_t position);
struct spline_point *Spline_Insert_Point(struct spline *spline, vec3_t origin, struct spline_point *s_point, int before, void *data);
void *Spline_Set_Point_Time_By_Number(struct spline *spline, int point, double time);
void Spline_Set_Point_From_Number(struct spline *spline, int point_number, vec3_t location);
int Spline_Calculate(struct spline *spline, struct spline_point *s_point ,double time, vec3_t vec);
void Spline_Cleanup(struct spline *spline);
