void ruby_setup_render_environment() {
    RUBY_INIT_STACK;
    ruby_init();

    //Ruby is stupid and won't initialize encodings
    char* dummy_argv[] = {"vim-ruby", "-e0"};
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
    rb_require("./rb_main.rb");
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

void ruby_load_font(Font* font, char* path) {
    //Load our font data through Ruby
    VALUE rb_font_dict = rb_funcall(Qnil, rb_intern("load_font"), 1, rb_str_new_cstr(path));
    //Grab the resulting dictionaries
    VALUE rb_info = ruby_hash_get(rb_font_dict, "info");
    VALUE rb_common = ruby_hash_get(rb_font_dict, "common");
    VALUE rb_page = ruby_hash_get(rb_font_dict, "page");
    VALUE rb_glyph = ruby_hash_get(rb_font_dict, "char");
    VALUE rb_glyph_ids = ruby_hash_get(rb_font_dict, "char_ids");
    VALUE rb_texture_name = ruby_hash_get(rb_page, 0);
    //Load glyph data
    VALUE rb_glyph_info;
    Glyph glyph;
    int glyph_count = RARRAY_LEN(rb_glyph_ids);
    for(int i = 0; i < glyph_count; i++) {
        rb_glyph_info = ruby_hash_get(rb_glyph, rb_ary_entry(rb_glyph_ids, i));
        glyph.char_id = (char)NUM2INT(ruby_hash_get(rb_glyph_info, "id"));
        glyph.size = glm::vec2(
            (GLfloat)RFLOAT_VALUE(ruby_hash_get(rb_glyph_info, "width")),
            (GLfloat)RFLOAT_VALUE(ruby_hash_get(rb_glyph_info, "height")) );
        glyph.center = glm::vec2(
            (GLfloat)RFLOAT_VALUE(ruby_hash_get(rb_glyph_info, "x")),
            (GLfloat)RFLOAT_VALUE(ruby_hash_get(rb_glyph_info, "y")) );
        glyph.advance = glm::vec2(
            (GLfloat)RFLOAT_VALUE(ruby_hash_get(rb_glyph_info, "xadvance")),
            0.0f );
        font->glyphs[glyph.char_id] = glyph;
    }

    //Load texture page data
    font->page->asset_path = construct_asset_path("fonts",
        StringValueCStr(rb_texture_name), "");
}
