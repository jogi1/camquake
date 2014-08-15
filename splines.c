#include "quakedef.h"


struct spline_point
{
	struct	spline_point *next, *prev;
	vec3_t	origin;
	int		number;
	double	time;
	void	*data;
	
};

struct spline
{
	int		points_count;
	struct	spline *next, *prev;
	struct	spline_point *points;
	void	*data;
};






struct spline	*splines;



void Spline_Cleanup_Points(struct spline *spline)
{
	struct spline_point *sp, *spp;;

	sp = spline->points;

	while (sp)
	{
		free(sp->data);
		spp = sp;
		sp = sp->next;
		free(spp);
	}


}

void Spline_Cleanup(struct spline *spline)
{

	Spline_Cleanup_Points(spline);
	free(spline->data);
	free(spline);

}



struct spline *Spline_Add(void *data)
{
	struct spline *spline;


	if (splines == NULL)
	{
		splines = calloc(1, sizeof(struct spline));
		spline	= splines;
	}
	else
	{
		spline = splines;

		while (spline->next)
			spline = spline->next;

		spline->next = calloc(1, sizeof(struct spline));
		spline->next->prev = spline;
		spline = spline->next;
	}

	spline->data = data;

	return spline;
}

void Spline_Remove(struct spline *spline)
{
	if (spline->next && spline->prev)
	{
		spline->next->prev = spline->prev;
		spline->prev->next = spline->next;
		Spline_Cleanup(spline);
		return;
	}

	if (spline->next && !spline->prev)
	{
		spline->next->prev = NULL;
		splines = spline->next;
		Spline_Cleanup(spline);
		return;
	}

	if (!spline->next && spline->prev)
	{
		spline->prev->next = NULL;
		Spline_Cleanup(spline);
		return;
	}

	if (!spline->next && !spline->prev)
	{
		splines = NULL;
		Spline_Cleanup(spline);
		return;

	}


}

void Spline_Update(struct spline *spline)
{
	struct spline_point *s_point, *last_point;
	int i = 0;
	int j;
	double time;
	s_point = spline->points;

	while (s_point)
	{
		i++;
		s_point->number = i;
		last_point = s_point;
		s_point = s_point->next;
	}
	spline->points_count = i;

	s_point = spline->points;

	if (i < 4)
		return;

	s_point->time = -1;
	last_point->time = 2;
	s_point->next->time = 0;
	last_point->prev->time = 1;

	if (i == 4)
		return;

	

	s_point = s_point->next;
	i -= 4;
	time = 1.0f/(i+1);
	for (j = 0; j < i; j++)
	{
		s_point = s_point->next;
		if (s_point)
		{
			//Com_Printf("%i/%i: %f\n", s_point->number, spline->points_count, (j+1) * time);
			s_point->time = (j+1) * time;	
		}
		else
			break;
	}





}

struct spline_point *Spline_Insert_Point(struct spline *spline, vec3_t origin, struct spline_point *s_point, int before, void *data)
{
	struct spline_point *spoint, *spoint1;

	if (!spline && !s_point)
	{
		Com_Printf("Spline_Insert_Point: neither \"spline\" nor \"s_point\"\n");
		return NULL;

	}

	spoint1 = calloc(1, sizeof(struct spline_point));
	if (!spoint1)
	{
		Com_Printf("Spline_Insert_Point: calloc error! Woops\n");
		return NULL;
	}

	spoint1->data = data;
	VectorCopy(origin, spoint1->origin);
	
	if (spline && !s_point)
	{
		if (spline->points)
		{
			if (before)
			{
				spline->points->prev = spoint1;
				spoint1->next = spline->points;
				spline->points = spline->points->prev;
				Spline_Update(spline);
				return spoint1;
			}
			else
			{
				spoint = spline->points;
				while (spoint->next)
					spoint = spoint->next;
				spoint->next = spoint1;
				spoint1->prev = spoint;
				spoint = spoint->next;
				Spline_Update(spline);
				return spoint1;
			}
		}
		else
		{
			spline->points = spoint1;
			Spline_Update(spline);
			return spoint1;
		}
		
	}

	if (s_point)
	{
		if (before)
		{
			if (s_point->prev == NULL)
			{
				if (!spline)
				{
					Com_Printf("Spline_Insert_Point: First point in the spline and you did not supply the spline!\n");
					free(spoint1);
					return NULL;
				}
				spoint1->next = spline->points;
				spline->points->prev = spoint1;
				spline->points = spoint1;
				Spline_Update(spline);
				return spoint1;
			}
			else
			{
				spoint1->prev = s_point->prev;
				s_point->prev->next = spoint1;
				spoint1->next = s_point;
				s_point->prev = spoint1;
				Spline_Update(spline);
				return spoint1;
				
			}

		}

		if (s_point->next)
		{
			spoint1->next = s_point->next;
			s_point->next->prev = spoint1;
		}
		s_point->next = spoint1;
		spoint1->prev = s_point;
		Spline_Update(spline);
		return spoint1;
	}

	free(spoint1);
	Spline_Update(spline);
	return NULL;

}


void Spline_Remove_Point(struct spline *spline, struct spline_point *s_point)
{

	if (s_point->prev && s_point->next)
	{
		s_point->prev->next = s_point->next;
		s_point->next->prev = s_point->prev;
	}

	if (!s_point->prev && s_point->next)
	{
		s_point->next->prev = NULL;
		spline->points = s_point->next;
	}

	if (s_point->prev && !s_point->next)
	{
		s_point->prev->next = NULL;
	}

	if (!s_point->prev && !s_point->next)
	{
		spline->points = NULL;
	}

	free(s_point->data);
	free(s_point);
	Spline_Update(spline);

}

int Spline_Get_Point_Count(struct spline *spline)
{
	if (!spline)
		return -1;
	return spline->points_count;

}

struct spline_point *Spline_Get_Point_From_Number(struct spline *spline, int point_number)
{

	struct spline_point *spoint;

	if (!spline || point_number < 1)
		return NULL;

	if (spline->points_count < point_number)
		return NULL;

	spoint = spline->points;

	while(spoint)
	{
		if (spoint->number == point_number)
		{
			return spoint;
		}

		spoint = spoint->next;
	}

	return NULL;
}

void Spline_Set_Point_From_Number(struct spline *spline, int point_number, vec3_t location)
{
	struct spline_point *spoint;
	
	spoint = Spline_Get_Point_From_Number(spline, point_number);
	if (!spoint)
		return;
	VectorCopy(location, spoint->origin);

}

void Spline_Get_Point_Position(struct spline *spline, int point_number, vec3_t position)
{

	struct spline_point *spoint;

	if (!spline || point_number < 1)
		return;

	if (spline->points_count < point_number)
		return;

	spoint = spline->points;

	while(spoint)
	{
		if (spoint->number == point_number)
		{
			VectorCopy(spoint->origin, position);
			return;
		}

		spoint = spoint->next;
	}
}

void Spline_Remove_Point_By_Number(struct spline *spline, int number)
{
	struct spline_point *spoint;

	if (!spline || number<1)
		return;

	if (number > spline->points_count)
		return;

	spoint = spline->points;

	while (spoint)
	{
		if (spoint->number == number)
		{
			Spline_Remove_Point(spline, spoint);
			return;
		}
		spoint = spoint->next;
	}




}


// 1  - point means static
// 2  - points mean straight line
// 3  - points mean cubic spline
// 4+ - points mean catmull rom

int Spline_Calculate(struct spline *spline, struct spline_point *s_point ,double time, vec3_t vec)
{
	double t1,t2,t3,t4;
	int i;
	vec3_t	points[4];
	struct spline_point *ls_point;
	double time_point_1, time_point_2, time_stop, time_current;

	int point_count;

	if (!spline)
		return 0;

	if (s_point)
		ls_point = s_point;
	else
		ls_point = spline->points;

	point_count = spline->points_count;// - (s_point->number -1);


	if (point_count < 0)
	{
		Com_Printf("Spline_Calculate: Wow negative!\n");
		return 0;
	}

	if (point_count == 0)
	{
		return 0;
	}

	if (point_count == 1)
	{
		VectorCopy(s_point->origin, vec);
		return 1;
	}

	if (point_count == 2 || point_count == 3)
	{
		t1 = 1 - time;
		t2 = pow(t1,point_count-1);
		t3 = pow(time,point_count-1);
		for (i=0;i<point_count && ls_point;i++){
			if (i == 0){
				vec[0] = ls_point->origin[0] * t2;
				vec[1] = ls_point->origin[1] * t2;
				vec[2] = ls_point->origin[2] * t2;
			}else if ( i ==point_count-1){
				vec[0] += ls_point->origin[0] * t3;
				vec[1] += ls_point->origin[1] * t3;
				vec[2] += ls_point->origin[2] * t3;
			} else {
				t4 = (point_count-1) * pow(time,i) * pow(t1,point_count-1-i);
				vec[0] += ls_point->origin[0]   * t4;
				vec[1] += ls_point->origin[1] * t4;
				vec[2] += ls_point->origin[2] * t4;
			}
			ls_point = ls_point->next;
		}
		return 1;
		
	}

	if (point_count > 3)
	{
		if (s_point)
			ls_point = s_point;
		else
		{
			ls_point = spline->points;
			if (!ls_point)
				return 0;
			while (ls_point->time < time)
			{
				ls_point = ls_point->next;
			}
			if (ls_point->prev)
				ls_point = ls_point->prev;
			if (ls_point->prev)
				ls_point = ls_point->prev; 
		}


		if (time == 0)
		{
			VectorCopy(ls_point->next->origin, vec);
			return 1;
		}

		if (time == 1)
		{
			VectorCopy(ls_point->next->next->origin, vec);
			return 1;
		}


		for (i=0;i<4 && ls_point;i++)
		{
			if (!ls_point)
			{
				Com_Printf("un error!\n");
				break;
			}
			if (i == 1)
				time_point_1 = ls_point->time;
			if (i == 2)
				time_point_2 = ls_point->time;

			VectorCopy(ls_point->origin, points[i]);
			ls_point = ls_point->next;
		}

		

		if (!s_point)
		{
			time_current = time - time_point_1;
			time_stop = time_point_2 - time_point_1;
			time = time_current/time_stop;

		}

		for (i=0;i<3;i++)
		{
			t1	=	points[1][i];
			t2	=	-0.5 * points[0][i] + 0.5 * points[2][i];
			t3	=	points[0][i] + -2.5 * points[1][i] + 2.0 * points[2][i] + -0.5 * points[3][i];
			t4	=	-0.5 * points[0][i] + 1.5 * points[1][i] + -1.5 * points[2][i] + 0.5 * points[3][i];
			vec[i] = ((t4*time + t3)*time +t2)*time + t1;
		}
		return 1;
	}

	return 0;
	
}

int Spline_Calculate_From_Point(struct spline *spline, double time, vec3_t vec, int point)
{
	struct spline_point *sp;
	sp = Spline_Get_Point_From_Number(spline, point);
	if (!sp)
	{
		return Spline_Calculate(spline, NULL, time, vec);
	}
	return Spline_Calculate(spline, sp, time, vec);


}


float	*Spline_Get_Point_Origin_By_Number(struct spline *spline, int point)
{
	struct spline_point *sp;

	sp = Spline_Get_Point_From_Number(spline, point);
	if (!sp)
	{
		return NULL;
	}
	return sp->origin;
}

void Spline_Set_Point_Time_By_Number(struct spline *spline, int point, double time)
{
	struct spline_point *sp;

	sp = Spline_Get_Point_From_Number(spline, point);
	if (!sp)
	{
		return;
	}
	sp->time = time;
}

double *Spline_Get_Point_Time_By_Number(struct spline *spline, int point)
{
	struct spline_point *sp;

	sp = Spline_Get_Point_From_Number(spline, point);
	if (!sp)
	{
		return NULL;
	}
	return &sp->time;
}

void *Spline_Get_Point_Data_By_Number(struct spline *spline, int point)
{
	struct spline_point *sp;

	sp = Spline_Get_Point_From_Number(spline, point);
	if (!sp)
	{
		return NULL;
	}
	return &sp->data;
}

void Spline_Check_Selected_Point_Time(struct spline *spline, int point)
{
	struct spline_point *sp;

	sp = Spline_Get_Point_From_Number(spline, point);
	if (!sp)
	{
		return;
	}

	if (spline->points_count < 4)
		return;

	if (sp->number == 1)
	{
		sp->time = - 1;
		return;
	}
	else if (sp->number == 2)
	{
		sp->time = 0;
		return;
	}
	else if (sp->number == spline->points_count)
	{
		sp->time = 2;
		return;
	}
	else if (sp->number== spline->points_count  -1 )
	{
		sp->time = 1;
		return;
	}
	else
	{
		if (sp->prev)
			if (sp->prev->time >= sp->time)
			{
				sp->time = sp->prev->time + 0.000001;
			}
		if (sp->next)
			if (sp->next->time <= sp->time)
			{
				sp->time = sp->next->time - 0.000001;
			}
	}



	return;
}

void Spline_Even_Speed(struct spline *spline)
{
	int point_count;
	double distance = 0;
	double *segment_distance;
	double time = 0;
	int i, j;
	vec3_t	old_pos, new_pos, temp;
	struct spline_point *point;


	point_count = spline->points_count;

	VectorSet(old_pos, 0, 0, 0);
	VectorSet(new_pos, 0, 0, 0);

	distance = 0;

	segment_distance = calloc(point_count - 2, sizeof(double));
	
	point = spline->points;

	for (i = 0; i < point_count - 3; i++)
	{
		for (j = 0; j <= 500 ; j++)
		{
			if (j > 0)
				VectorCopy(new_pos, old_pos);

			Spline_Calculate(spline, point, j/500.0f, new_pos);
			
			if (j > 0)
			{
				VectorSubtract(new_pos, old_pos, temp);
				segment_distance[i] += VectorLength(temp);
				
			}
		}
		distance += segment_distance[i];
		point = point->next;
	}

	time = 0;
	point = spline->points->next->next;
	for (i = 0; i < point_count - 3; i++)
	{
		time += segment_distance[i]/distance;
		point->time = time;
		point = point->next;
	}
	free(segment_distance);
}
