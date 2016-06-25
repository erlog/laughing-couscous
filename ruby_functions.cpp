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
    GLfloat scaleW = (GLfloat)NUM2INT(ruby_hash_get(rb_common, "scaleW"));
    GLfloat scaleH = (GLfloat)NUM2INT(ruby_hash_get(rb_common, "scaleH"));
    GLfloat font_size = (GLfloat)NUM2INT(ruby_hash_get(rb_info, "size"));

    int glyph_count = RARRAY_LEN(rb_glyph_ids);
    for(int i = 0; i < glyph_count; i++) {
        rb_glyph_info = ruby_hash_get(rb_glyph, rb_ary_entry(rb_glyph_ids, i));
        glyph.char_id = (char)NUM2INT(ruby_hash_get(rb_glyph_info, "id"));
        glyph.uv_info = glm::vec4(
            NUM2INT(ruby_hash_get(rb_glyph_info, "x"))/scaleW,
            NUM2INT(ruby_hash_get(rb_glyph_info, "y"))/scaleH,
            NUM2INT(ruby_hash_get(rb_glyph_info, "width"))/scaleW,
            NUM2INT(ruby_hash_get(rb_glyph_info, "height"))/scaleH );
        glyph.offset = glm::vec3(
            NUM2INT(ruby_hash_get(rb_glyph_info, "xoffset"))/font_size,
            NUM2INT(ruby_hash_get(rb_glyph_info, "yoffset"))/font_size, 0.0f);
        glyph.size = glm::vec3(
            NUM2INT(ruby_hash_get(rb_glyph_info, "width"))/font_size,
            NUM2INT(ruby_hash_get(rb_glyph_info, "height"))/font_size, 0.0f);
        glyph.advance = glm::vec3(
            NUM2INT(ruby_hash_get(rb_glyph_info, "xadvance"))/font_size, 0.0f,
            0.0f);

        DEBUG_LOG(glyph.offset);
        //pre-compute values for our renderer

        font->glyphs[glyph.char_id] = glyph;
    }

    //Load texture page data
    font->page->asset_path = construct_asset_path("fonts",
        StringValueCStr(rb_texture_name));
}
