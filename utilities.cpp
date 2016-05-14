//Small functions that don't fit anywhere else with minimal dependencies
string construct_asset_path(string object_name, string filename) {
    //TODO:this seems gross
    return State.AssetFolderPath + "/" + object_name + "/" + filename;
}

char* get_datetime_string() {
    time_t rawtime; struct tm *info;
    char* output = (char*)malloc(sizeof(char)*255);

    time( &rawtime ); info = localtime( &rawtime );
    strftime(output, 255,"%Y-%m-%d %H:%M:%S", info); //TODO: 255?
    return output;
}

bool read_entire_file(string file_path, string* output_string) {
    ifstream infile; infile.open(file_path);
    //TODO: error handling
    stringstream str_stream; str_stream << infile.rdbuf();
    *output_string = str_stream.str();
    return true;
}


void message_log(const char* message, const char* predicate) {
    printf("%i: %s %s\n", State.CurrentTime, message, predicate);
}
void message_log(const char* message, string predicate) {
    message_log(message, predicate.c_str());
}

void flip_texture(Texture* texture) {
    uint8_t* buffer = (uint8_t*)malloc(texture->buffer_size);
    int src_i; int dest_i; int y; int x;
    for(src_i = 0; src_i < texture->buffer_size; src_i++) {
        y = src_i / texture->pitch; x = src_i % texture->pitch;
        dest_i = ((texture->height - y - 1) * texture->pitch) + x;
        buffer[dest_i] = texture->buffer[src_i];
    }
    free(texture->buffer);
    texture->buffer = buffer;
    return;
}

bool load_shader(string shader_path, GLuint* shader_id, GLenum shader_type) {
    string shader_source_string;
    if(!read_entire_file(shader_path, &shader_source_string)) {
        message_log("Error loading file-", shader_path);
    }
    *shader_id = glCreateShader(shader_type);
    const GLchar* shader_source = shader_source_string.c_str();
    glShaderSource(*shader_id, 1, (const GLchar**)&shader_source, NULL);
    glCompileShader(*shader_id);

    GLint compiled = GL_FALSE;
    glGetShaderiv(*shader_id, GL_COMPILE_STATUS, &compiled);
    if(compiled != GL_TRUE ) {
        message_log("Open GL-", "error compiling shader");
        message_log("Source:", shader_source);
        GLchar buffer[1024];
        glGetShaderInfoLog(*shader_id, 1024, NULL, buffer);
        message_log("Open GL-", buffer);
        return false;
    }
    return true;
}


bool load_texture(string object_name, string filename, Texture* texture) {
    texture->asset_path = construct_asset_path(object_name, filename);

    //Load PNG
    unsigned width; unsigned height;
    if(lodepng_decode32_file(&texture->buffer, &width, &height,
        texture->asset_path.c_str())) { return false; }

    //Create texture struct
    texture->bytes_per_pixel = 4;
    texture->width = (GLsizei)width; texture->height = (GLsizei)height;
    texture->pitch = texture->bytes_per_pixel * texture->width;
    texture->buffer_size = texture->pitch * texture->height;

    //Flip it because OpenGL coords start in the lower left
    flip_texture(texture);

    //Register our texture with OpenGL
    //TODO: error handling
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0,
         GL_RGBA, GL_UNSIGNED_BYTE, texture->buffer);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
    glBindTexture(GL_TEXTURE_2D, 0); //unbind the texture
    return true;
}

bool load_object(Object* object) {
    //Texture
    object->texture = (Texture*)malloc(sizeof(Texture));
    if(!load_texture(object->object_name, string("diffuse.png"), object->texture)) {
        message_log("Error loading texture-", object->object_name);
        return false;
    }
    //Normal Map
    object->normal_map= (Texture*)malloc(sizeof(Texture));
    if(!load_texture(object->object_name, string("nm_tangent.png"), object->normal_map)) {
        message_log("Error loading normal map-", object->object_name);
        return false;
    }
    //Specular Map
    object->specular_map = (Texture*)malloc(sizeof(Texture));
    if(!load_texture(object->object_name, string("spec.png"), object->specular_map)) {
        message_log("Error loading specular map-", object->object_name);
        return false;
    }
    //Model
    object->model = (Model*)malloc(sizeof(Model));
    if(!load_model(object->object_name, object->model)) {
        message_log("Error loading model-", object->object_name);
        return false;
    }
    //Shaders
    string path;
    object->shader_program = glCreateProgram();
    path = construct_asset_path(object->object_name, string("vertex.shader"));
    GLuint shader_id;
    if(!load_shader(path, &shader_id, GL_VERTEX_SHADER)) {
        message_log("Error loading vertex shader-", object->object_name);
        return false;
    }
    glAttachShader(object->shader_program, shader_id);
    path = construct_asset_path(object->object_name, "fragment.shader");
    if(!load_shader(path, &shader_id, GL_FRAGMENT_SHADER)) {
        message_log("Error loading vertex shader-", object->object_name);
        return false;
    }
    glAttachShader(object->shader_program, shader_id);
    glLinkProgram(object->shader_program);

    return true;
}

void take_screenshot() {
    //construct path TODO: better way to do string nonsense?
    string datetime_string = get_datetime_string();
    string output_path = State.OutputFolderPath + string("/renderer - ") +
        datetime_string + string(".png");
    message_log("Taking screenshot-", output_path);

    //write screenshot
    glReadPixels(0, 0, State.screen->width, State.screen->height,
        GL_RGB, GL_UNSIGNED_BYTE, State.screen->buffer);
    flip_texture(State.screen);
    lodepng_encode24_file(output_path.c_str(), (const unsigned char*)State.screen->buffer,
        State.screen->width, State.screen->height);

}
