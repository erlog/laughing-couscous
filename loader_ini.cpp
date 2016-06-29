//Parser for the dictionary spit out by the Ruby script
void load_settings(State* state) {
    //Initialize default settings
    state->Settings.horizontal_resolution = 682;
    state->Settings.vertical_resolution = 384;
    state->Settings.vsync = false;
    state->Settings.fullscreen = false;

    //Load our ini data through Ruby
    VALUE rb_hash = rb_funcall(Qnil, rb_intern("load_ini"), 1,
        rb_str_new_cstr(SettingsINIPath));

        state->Settings.horizontal_resolution =
            ruby_hash_get_int(rb_hash, "horizontal_resolution");
        state->Settings.vertical_resolution =
            ruby_hash_get_int(rb_hash, "vertical_resolution");
        state->Settings.vsync =
            (ruby_hash_get_int(rb_hash, "vsync") == 1);
        state->Settings.fullscreen =
            (ruby_hash_get_int(rb_hash, "fullscreen") == 1);
}
