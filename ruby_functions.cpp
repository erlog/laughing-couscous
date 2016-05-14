void ruby_setup_render_environment() {
    RUBY_INIT_STACK;
    ruby_init();

    //Ruby is stupid and won't initialize encodings
    char *dummy_argv[] = {"vim-ruby", "-e0"};
    ruby_options(2, dummy_argv);

    ruby_script("renderer");
    ruby_init_loadpath();
    /*
    VALUE C_Point = rb_define_class("Point", rb_cObject);
    rb_define_alloc_func(C_Point, C_Point_allocate);
    rb_define_method(C_Point, "initialize", C_Point_initialize, 3);
    rb_define_method(C_Point, "x", C_Point_get_x, 0);
    rb_define_method(C_Point, "y", C_Point_get_y, 0);
    rb_define_method(C_Point, "z", C_Point_get_z, 0);
    rb_define_method(C_Point, "x=", C_Point_set_x, 1);
    rb_define_method(C_Point, "y=", C_Point_set_y, 1);
    rb_define_method(C_Point, "z=", C_Point_set_z, 1);
    */
    rb_require("./main.rb");
}
