require "helpers";
require "vector";

my_ident = "lua_spiral";
spline_start_stop = 0;
spline_start_stop_show = 0;
spline_spiral = 0;

point1_radius = 0;
point1_radius_last = 0;

point2_radius = 0;
point2_radius_last = 0;

point1_rotation = 0;
point1_rotation_last = 0;

rotation_angle = 0;
rotation_angle_last = 0;

distance_length = 0;
distance_length_last = 0;

spline_point_count = 0;
spline_point_count_last = 0;

point1_x = 0;
point1_y = 0;
point1_z = 0;
point2_x = 0;
point2_y = 0;
point2_z = 0;

point1_x_last = 0;
point1_y_last = 0;
point1_z_last = 0;
point2_x_last = 0;
point2_y_last = 0;
point2_z_last = 0;

function redo_spiral()
	p1 = Vector:new(Menu:double_get(point1_x), Menu:double_get(point1_y), Menu:double_get(point1_z));
	p2 = Vector:new(Menu:double_get(point2_x), Menu:double_get(point2_y), Menu:double_get(point2_z));

	if p1 == p2 then
		return;
	end

	Spline:delete_all_points(spline_spiral);

	v = p2 - p1;

	if v.x > v.z and v.x > v.y then
		v1 = Vector:new(1, 0, 0) + p2;
	elseif v.y > v.z and v.y > v.x then
		v1 = Vector:new(0, 1, 0) + p2;
	elseif v.z > v.x and v.z > v.y then
		v1 = Vector:new(0, 0, 1) + p2;
	end

	v2 = v:cross(v1);

	vn = v2:duplicate();
	vn:normalize();

	rdiff = point2_radius_last - point1_radius_last;
	rdiff = rdiff / spline_point_count_last;

	v:scale(1/spline_point_count_last);

	v2 = vn:duplicate();
	v2:scale(point1_radius_last - rdiff);
	v2 = v2 + p1 - v;

	i = rotation_angle_last / spline_point_count_last;

	v2:set(Quake:rotate_point_around_line(v2.x, v2.y, v2.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, point1_rotation_last));

	p = Vector:new(Quake:rotate_point_around_line(v2.x, v2.y, v2.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, i * -1));

	Spline:add_point(spline_spiral, p.x, p.y, p.z);

	for x = 0, spline_point_count_last + 1, 1 do
		v_temp1 = vn:duplicate();
		v_temp2 = v:duplicate();
		v_temp1:scale(point1_radius_last + rdiff * x);
		v2:set(v_temp1:components());
		v_temp2:scale(x);
		v2 = v2 + p1 + v_temp2;
		v_temp3 = Vector:new(Quake:rotate_point_around_line(v2.x, v2.y, v2.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, i * x + point1_rotation_last));
		Spline:add_point(spline_spiral, v_temp3:components());
	end
end

function ls_point1_set_button_button (self)
	x, y, z = Quake:get_position();
	Menu:double_set(point1_x, x);
	Menu:double_set(point1_y, y);
	Menu:double_set(point1_z, z);
	point_1_x_last = x;
	point_1_y_last = y;
	point_1_z_last = z;
	Spline:set_point(spline_start_stop, 1, x, y, z);
	redo_spiral();
end

function ls_point2_set_button_button (self)
	x, y, z = Quake:get_position();
	Menu:double_set(point2_x, x);
	Menu:double_set(point2_y, y);
	Menu:double_set(point2_z, z);
	point_2_x_last = x;
	point_2_y_last = y;
	point_2_z_last = z;
	Spline:set_point(spline_start_stop, 2, x, y, z);
	redo_spiral();
end

function ls_show_text_button_button (self)
	if spline_start_stop_show == 0 then
		spline_start_stop_show = 1;
		Spline:show(spline_start_stop, 1);
		Spline:show(spline_spiral, 1);
		Menu:change_content_text(self, "enabled");
	else
		spline_start_stop_show = 0;
		Spline:show(spline_start_stop, 0);
		Spline:show(spline_spiral, 0);
		Menu:change_content_text(self, "disabled");
	end
end

function ls_spline_to_cambutton_button (self)
	Spline:to_cam(spline_spiral);
end

function ls_spline_to_viewbutton_button (self)
	Spline:to_view(spline_spiral);
end

function init ()

	spline_start_stop = Spline:create();
	Spline:add_point(spline_start_stop, 0, 0, 0);
	Spline:add_point(spline_start_stop, 0, 0, 0);
	Spline:show(spline_start_stop, 0);
	Spline:color(spline_start_stop, 1, 0.5, 0.5);

	spline_spiral = Spline:create();
	Spline:show(spline_spiral, 0);
	Spline:color(spline_spiral, 1, 1, 1);

	point1_x = Menu:double_create();
	point1_y = Menu:double_create();
	point1_z = Menu:double_create();

	point1_radius = Menu:double_create();
	Menu:double_set(point1_radius, 100);
	point1_rotation = Menu:double_create();

	point2_radius = Menu:double_create();
	Menu:double_set(point2_radius, 100);

	point2_x = Menu:double_create();
	point2_y = Menu:double_create();
	point2_z = Menu:double_create();

	rotation_angle = Menu:double_create();
	Menu:double_set(rotation_angle, 360);

	distance_length = Menu:double_create();
	Menu:double_set(distance_length, 1);

	spline_point_count = Menu:int_create();
	Menu:int_set(spline_point_count, 100);

	Menu:add_to_mainmenu(10, 10, 210, 210, "Spiral", my_ident);

	Menu:add_content_text(my_ident, "NULL", "ls_show_text", 1, "Show: ");
		Menu:add_content_button_color(my_ident, "ls_show_text", "ls_show_text_button", 0, "f00","disabled");

	Menu:add_content_text(my_ident, "ls_show_text", "ls_point1_text", 1, "Start Point: ");
		Menu:add_content_button_color(my_ident, "ls_point1_text", "ls_point1_set_button", 0, "f00", "set");
	Menu:add_content_text(my_ident, "ls_point1_text", "ls_point1_text_x", 1, "x: ");
		Menu:add_content_editbox_double(my_ident, "ls_point1_text_x", "ls_point1_editbox_x", 0, point1_x); 
		Menu:add_content_slider_width_double(my_ident, "ls_point1_editbox_x", "ls_point1_slider_x", 0, 1, -100000, 100000, point1_x);
	Menu:add_content_text(my_ident, "ls_point1_text_x", "ls_point1_text_y", 1, "y: ");
		Menu:add_content_editbox_double(my_ident, "ls_point1_text_y", "ls_point1_editbox_y", 0, point1_y); 
		Menu:add_content_slider_width_double(my_ident, "ls_point1_editbox_y", "ls_point1_slider_y", 0, 1, -100000, 100000, point1_y);
	Menu:add_content_text(my_ident, "ls_point1_text_y", "ls_point1_text_z", 1, "z: ");
		Menu:add_content_editbox_double(my_ident, "ls_point1_text_z", "ls_point1_editbox_z", 0, point1_z); 
		Menu:add_content_slider_width_double(my_ident, "ls_point1_editbox_z", "ls_point1_slider_z", 0, 1, -100000, 100000, point1_z);

	Menu:add_content_text(my_ident, "ls_point1_text_z", "ls_point2_text", 1, "Stop Point: ");
		Menu:add_content_button_color(my_ident, "ls_point2_text", "ls_point2_set_button", 0, "f00", "set");
	Menu:add_content_text(my_ident, "ls_point2_text", "ls_point2_text_x", 1, "x: ");
		Menu:add_content_editbox_double(my_ident, "ls_point2_text_x", "ls_point2_editbox_x", 0, point2_x); 
		Menu:add_content_slider_width_double(my_ident, "ls_point2_editbox_x", "ls_point2_slider_x", 0, 1, -100000, 100000, point2_x);
	Menu:add_content_text(my_ident, "ls_point2_text_x", "ls_point2_text_y", 1, "y: ");
		Menu:add_content_editbox_double(my_ident, "ls_point2_text_y", "ls_point2_editbox_y", 0, point2_y); 
		Menu:add_content_slider_width_double(my_ident, "ls_point2_editbox_y", "ls_point2_slider_y", 0, 1, -100000, 100000, point2_y);
	Menu:add_content_text(my_ident, "ls_point2_text_y", "ls_point2_text_z", 1, "z: ");
		Menu:add_content_editbox_double(my_ident, "ls_point2_text_z", "ls_point2_editbox_z", 0, point2_z); 
		Menu:add_content_slider_width_double(my_ident, "ls_point2_editbox_z", "ls_point2_slider_z", 0, 1, -100000, 100000, point2_z);
	
	Menu:add_content_text(my_ident, "ls_point2_text_z", "ls_divider", 1, "-----------------------------------------------");

	Menu:add_content_text(my_ident, "ls_divider", "ls_radius_point1_text", 1, "point 1 radius: ");
		Menu:add_content_editbox_double(my_ident, "ls_radius_point1_text", "ls_radius_point1_editbox", 0, point1_radius); 
		Menu:add_content_slider_width_double(my_ident, "ls_radius_point1_editbox", "ls_radius_point1", 0, 1, 0, 100000, point1_radius);
	Menu:add_content_text(my_ident, "ls_radius_point1_text", "ls_point1_rotation_text", 1, "point 1 rotation: ");
		Menu:add_content_slider_width_double(my_ident, "ls_point1_rotation_text", "ls_point1_rotation_slider", 0, 1, 0, 1440, point1_rotation);
	Menu:add_content_text(my_ident, "ls_point1_rotation_text", "ls_point2_radius_text", 1, "point 2 radius: ");
		Menu:add_content_editbox_double(my_ident, "ls_point2_radius_text", "ls_point2_radius_editbox", 0, point2_radius); 
		Menu:add_content_slider_width_double(my_ident, "ls_point2_radius_editbox", "ls_point2_radius_slider", 0, 1, 0, 100000, point2_radius);

	Menu:add_content_text(my_ident, "ls_point2_radius_text", "ls_rotation_angle", 1, "rotation angle: ");
		Menu:add_content_editbox_double(my_ident, "ls_rotation_angle", "ls_rotation_angle_editbox", 0, rotation_angle); 
		Menu:add_content_slider_width_double(my_ident, "ls_rotation_angle_editbox", "ls_rotation_angle_slider", 0, 1, -100000, 100000, rotation_angle);

	Menu:add_content_text(my_ident, "ls_rotation_angle", "ls_spline_point_count_text", 1, "points: ");
		Menu:add_content_slider_width_int(my_ident, "ls_spline_point_count_text", "ls_spline_point_count_slider", 0, 1, 5, 100, spline_point_count);
	Menu:add_content_text(my_ident, "ls_spline_point_count_text", "ls_divider1", 1, "-----------------------------------------------");

	Menu:add_content_button_color(my_ident, "ls_divider1", "ls_spline_to_cambutton", 1, "f00", "convert to cam");
	Menu:add_content_button_color(my_ident, "ls_spline_to_cambutton", "ls_spline_to_viewbutton", 1, "f00", "convert to view");



end

function main (time)
	redo_spline = 0;
	redo_point1 = 0;
	if (Menu:double_get(point1_x) ~= point1_x_last) then
		redo_point1 = 1;
	end

	if (Menu:double_get(point1_y) ~= point1_y_last) then
		redo_point1 = 1;
	end

	if (Menu:double_get(point1_z) ~= point1_z_last) then
		redo_point1 = 1;
	end
	
	if (redo_point1 == 1) then
		x = Menu:double_get(point1_x, x);
		y = Menu:double_get(point1_y, y);
		z = Menu:double_get(point1_z, z);
		point1_x_last = x;
		point1_y_last = y;
		point1_z_last = z;
		Spline:set_point(spline_start_stop, 1, x, y, z);
		redo_spline = 1;
	end

	redo_point2 = 0;
	if (Menu:double_get(point2_x) ~= point2_x_last) then
		redo_point2 = 1;
	end

	if (Menu:double_get(point2_y) ~= point2_y_last) then
		redo_point2 = 1;
	end

	if (Menu:double_get(point2_z) ~= point2_z_last) then
		redo_point2 = 1;
	end
	
	if (redo_point2 == 1) then
		x = Menu:double_get(point2_x, x);
		y = Menu:double_get(point2_y, y);
		z = Menu:double_get(point2_z, z);
		point2_x_last = x;
		point2_y_last = y;
		point2_z_last = z;
		Spline:set_point(spline_start_stop, 2, x, y, z);
		redo_spline = 1;
	end

	point1_radius_last, redo_spline = helpers.compare(Menu:double_get(point1_radius), point1_radius_last, redo_spline);

	point1_rotation_last, redo_spline = helpers.compare(Menu:double_get(point1_rotation), point1_rotation_last, redo_spline);

	point2_radius_last, redo_spline = helpers.compare(Menu:double_get(point2_radius), point2_radius_last, redo_spline);

	rotation_angle_last, redo_spline = helpers.compare(Menu:double_get(rotation_angle), rotation_angle_last, redo_spline);

	spline_point_count_last, redo_spline = helpers.compare(Menu:int_get(spline_point_count), spline_point_count_last, redo_spline);

	if (redo_spline == 1) then
		redo_spiral();
	end
end
