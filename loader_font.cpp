//Parser for the dictionary spit out by loader_font.rb
void ruby_load_font(Font* font, char* path) {
    //Load our font data through Ruby
    VALUE rb_font_dict = rb_funcall(Qnil, rb_intern("load_font"), 1, rb_str_new_cstr(path));
    //Grab the resulting dictionaries
    VALUE rb_info = ruby_hash_get(rb_font_dict, "info");
    VALUE rb_common = ruby_hash_get(rb_font_dict, "common");
    VALUE rb_page = ruby_hash_get(rb_font_dict, "page");
    VALUE rb_glyph_array = ruby_hash_get(rb_font_dict, "chars");
    VALUE rb_texture_name = ruby_hash_get(rb_page, 0);


    //Get info common to all glyphs
    VALUE rb_ary_padding = ruby_hash_get(rb_info, "padding");
    GLfloat scaleW = (GLfloat)ruby_hash_get_int(rb_common, "scaleW");
    GLfloat scaleH = (GLfloat)ruby_hash_get_int(rb_common, "scaleH");
    GLfloat font_size = (GLfloat)ruby_hash_get_int(rb_info, "size");
    int x_padding = NUM2INT(rb_ary_entry(rb_ary_padding, 1));
        NUM2INT(rb_ary_entry(rb_ary_padding, 3));
    int y_padding = NUM2INT(rb_ary_entry(rb_ary_padding, 0));
        NUM2INT(rb_ary_entry(rb_ary_padding, 2));

    //Load glyph data
    Glyph glyph;
    VALUE rb_glyph_info;
    uint32_t id;
    int glyph_count = RARRAY_LEN(rb_glyph_array);

    for(int i = 0; i < glyph_count; i++) {
        rb_glyph_info = rb_ary_entry(rb_glyph_array, i);
        id = FIX2UINT(ruby_hash_get(rb_glyph_info, "id"));
        glyph.char_id = (UChar32)id;

        glyph.uv_info = glm::vec4(
            ruby_hash_get_int(rb_glyph_info, "x")/scaleW,
            ruby_hash_get_int(rb_glyph_info, "y")/scaleH,
            ruby_hash_get_int(rb_glyph_info, "width")/scaleW,
            ruby_hash_get_int(rb_glyph_info, "height")/scaleH );
        glyph.offset = glm::vec3(
            (GLfloat)ruby_hash_get_int(rb_glyph_info, "xoffset"),
            (GLfloat)ruby_hash_get_int(rb_glyph_info, "yoffset"), 0.0f);
        glyph.size = glm::vec3(
            (GLfloat)ruby_hash_get_int(rb_glyph_info, "width"),
            (GLfloat)ruby_hash_get_int(rb_glyph_info, "height"), 0.0f);
        glyph.advance = glm::vec3(
            ruby_hash_get_int(rb_glyph_info, "xadvance")/font_size, 0.0f,
            0.0f);

        //pre-compute values to convert from edge of glyph to
        //center for our renderer
        glyph.offset.x += ( (glyph.size.x - x_padding) * 0.5f);
        glyph.offset.y += ( (glyph.size.y - y_padding) * 0.5f);

        //normalize pixel values
        glyph.size /= font_size;
        glyph.offset /= font_size;

        //invert the y offset so that our coord becomes the lower left corner
        //rather than the upper left corner.
        glyph.offset.y = 1.0 - glyph.offset.y;

        //save glyph in hash map
        font->glyphs[glyph.char_id] = glyph;
    }

    //Load texture page data
    font->page->asset_path = construct_asset_path("fonts",
        StringValueCStr(rb_texture_name));
}

