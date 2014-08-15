struct CamSys_Spline
{
	struct	CamSys_Spline *next, *prev;
	void	*spline;
	double	starttime, stoptime;
	float	r,g,b,a;
	int		show, enable, resolution, number;
	int		active_point;

};


struct CamSys_Spline *CamSys_Add_Spline(int type);


