draw_set_font(fnt_test);
draw_set_color(c_white);

draw_text(5, 5, @"Try things:
1: Non-fatal error (show_error)
2: Non-fatal error (built-in)
3: Fatal error (show_error)
4: Fatal error (built-in)
5: Fatal error (new format - ignore only)
Timer: " + string(current_time / 1000));
draw_text(5, 140, error_text);
if (keyboard_check_pressed(ord("1"))) show_error("An error! Time:" + date_datetime_string(date_current_datetime()), false);
if (keyboard_check_pressed(ord("2"))) draw_surface(-4, 0, 0);
if (keyboard_check_pressed(ord("3"))) show_error("A fatal error! Time:" + date_datetime_string(date_current_datetime()), 1);
if (keyboard_check_pressed(ord("4"))) vertex_submit(-4, -4, -4);
if (keyboard_check_pressed(ord("5"))) {
	//catch_error_set_newer(catch_error_newer_ignore);
	var v = 1;v += undefine;
	//vertex_begin(-1, -1);
	//catch_error_set_newer(catch_error_newer_allow);
}