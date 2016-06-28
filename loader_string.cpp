//Parser for the dictionary spit out by loader_font.rb
inline bool ruby_load_strings(String_DB* string_db) {
    //Load our font data through Ruby
    VALUE rb_strings_array = rb_funcall(Qnil, rb_intern("load_strings"), 1,
        rb_str_new_cstr(string_db->asset_path));

    //Convert array of string dictionary objects to our internal represenation
    VALUE rb_string_dict; VALUE rb_string; int id;
    int string_count = RARRAY_LEN(rb_strings_array);
    for(int i = 0; i < string_count; i++) {
        rb_string_dict = rb_ary_entry(rb_strings_array, i);
        id = ruby_hash_get_int(rb_string_dict, "id");
        rb_string = ruby_hash_get(rb_string_dict, "string");

        //convert string to ICU Unicode Chars and store in map
        string_db->db[id] = UChar_convert(StringValueCStr(rb_string));
    }

    return true;
}

//shim
bool load_strings(String_DB* string_db, const char* db_name) {
    string_db->asset_path = construct_asset_path("strings", db_name, "utf8");
    return ruby_load_strings(string_db);
}
