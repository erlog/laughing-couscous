//Small functions that don't fit anywhere else with minimal dependencies
char* construct_asset_path(const char* folder, const char* filename) {
    char* buffer = (char*)malloc(sizeof(char)*255);
    snprintf(buffer, 254, "%s/%s/%s", State.AssetFolderPath, folder,
        filename);
    return buffer;
}

char* get_datetime_string() {
    time_t rawtime; struct tm *info;
    char* output = (char*)malloc(sizeof(char)*255);

    time( &rawtime ); info = localtime( &rawtime );
    strftime(output, 255,"%Y-%m-%d %H:%M:%S", info); //TODO: 255?
    return output;
}

void message_log(const char* message, const char* predicate) {
    printf("%i: %s %s\n", State.CurrentTime, message, predicate);
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

bool load_shader(const char* shader_path, GLuint* shader_id, GLenum shader_type) {
    //Read in shader source
    FILE* file = fopen(shader_path, "r");
    if(file == NULL) {
        return false;
        message_log("Error loading file-", shader_path);
    }
    fseek(file, 0, SEEK_END); int length = ftell(file); fseek(file, 0, SEEK_SET);
    GLchar* shader_source = (GLchar*)malloc(sizeof(GLchar)*length+1);
    fread(shader_source, sizeof(GLchar), length, file);
    shader_source[length] = (GLchar)0;
    fclose(file);

    //Compile shader
    *shader_id = glCreateShader(shader_type);
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


bool load_texture(const char* filename, Texture* texture) {
    texture->asset_path = construct_asset_path("textures", filename);

    //Load PNG
    unsigned width; unsigned height;
    if(lodepng_decode32_file(&texture->buffer, &width, &height,
        texture->asset_path)) { return false; }

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
    if(!load_texture(object->texture_name, object->texture)) {
        message_log("Error loading texture-", object->texture_name);
        return false;
    }
    //Normal Map
    object->normal_map= (Texture*)malloc(sizeof(Texture));
    if(!load_texture(object->nm_name, object->normal_map)) {
        message_log("Error loading normal map-", object->nm_name);
        return false;
    }
    //Specular Map
    object->specular_map = (Texture*)malloc(sizeof(Texture));
    if(!load_texture(object->spec_name, object->specular_map)) {
        message_log("Error loading specular map-", object->spec_name);
        return false;
    }
    //Model
    object->model = (Model*)malloc(sizeof(Model));
    if(!load_model(object->model_name, object->model)) {
        message_log("Error loading model-", object->model_name);
        return false;
    }
    //Shaders
    object->shader_program = glCreateProgram();
    char* path = construct_asset_path("shaders", object->vert_shader_name);
    GLuint shader_id;
    if(!load_shader(path, &shader_id, GL_VERTEX_SHADER)) {
        message_log("Error loading vertex shader-", object->vert_shader_name);
        return false;
    }
    glAttachShader(object->shader_program, shader_id);
    path = construct_asset_path("shaders", object->frag_shader_name);
    if(!load_shader(path, &shader_id, GL_FRAGMENT_SHADER)) {
        message_log("Error loading fragment shader-", object->frag_shader_name);
        return false;
    }
    glAttachShader(object->shader_program, shader_id);
    glLinkProgram(object->shader_program);

    return true;
}

void take_screenshot() {
    //construct path TODO: better way to do string nonsense?
    char* datetime_string = get_datetime_string();
    char* output_path = (char*)malloc(sizeof(char)*255);

    sprintf(output_path, "%s/renderer - %s.png", State.OutputFolderPath,
        datetime_string);
    message_log("Taking screenshot-", output_path);

    //write screenshot
    glReadPixels(0, 0, State.screen->width, State.screen->height,
        GL_RGB, GL_UNSIGNED_BYTE, State.screen->buffer);
    flip_texture(State.screen);
    lodepng_encode24_file(output_path, (const unsigned char*)State.screen->buffer,
        State.screen->width, State.screen->height);

    free(datetime_string);
    free(output_path);
}
