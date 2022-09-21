catch_error_init();
show_debug_message("ready? " + string(catch_error_is_ready()));
catch_error_set_prompt(catch_error_prompt_question,
    @"This wonderful little demo encountered an error.
Would you like to restart the program and view the error?",
    "Oh no,");
catch_error_set_normal(catch_error_normal_queue);
catch_error_set_fatal(catch_error_fatal_queue);
show_message("");
//catch_error_set_newer(catch_error_newer_ignore);
error_file = "misc/error.log";
catch_error_set_dump_path(error_file);
// let the program restart itself on error
var argv = "", argc = parameter_count();
for (var i = 1; i < argc; i++) {
    if (i > 1) argv += " ";
    var v = parameter_string(i);
    if (string_pos(" ", v)) argv += @'"' + v + @'"'; else argv += v;
}
catch_error_set_exec(parameter_string(0), argv);
//
//show_message("?");
error_text = "";
if (file_exists(error_file)) {
    var buf = buffer_load(error_file);
    error_text = buffer_read(buf, buffer_string);
    buffer_delete(buf);
    file_delete(error_file);
}
