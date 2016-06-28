void ruby_setup_environment() {
    //Ruby global init
    RUBY_INIT_STACK; ruby_init();

    //Initialize encodings with args
    char* dummy_argv[] = {"gl-ruby", "-e0"}; ruby_options(2, dummy_argv);

    //Set our program title and load paths
    ruby_script("renderer"); ruby_init_loadpath();

    //Require our main script
    rb_require("./objects/ruby/main.rb");
}

//Convenience functions for working with Ruby objects
inline int ruby_hash_get_int(VALUE hash, const char* key) {
    return NUM2INT(rb_hash_aref(hash, rb_str_new_cstr(key)));
}
inline VALUE ruby_hash_get(VALUE hash, VALUE key) {
    return rb_hash_aref(hash, key);
}
inline VALUE ruby_hash_get(VALUE hash, const char* key) {
    return rb_hash_aref(hash, rb_str_new_cstr(key));
}
inline VALUE ruby_hash_get(VALUE hash, int key) {
    return rb_hash_aref(hash, INT2NUM(key));
}

