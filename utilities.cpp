//Small functions that don't fit anywhere else with minimal dependencies
char* construct_asset_path(const char* folder, const char* filename) {
    //TODO: dynamically allocate appropriately sized string;
    char* buffer = (char*)malloc(sizeof(char)*255);
    snprintf(buffer, 254, "%s/%s/%s", AssetFolderPath, folder,
        filename);
    return buffer;
}

char* get_datetime_string() {
    time_t rawtime; struct tm *info;
    time( &rawtime ); info = localtime( &rawtime );

    int output_length = 16;
    char* output = (char*)malloc(sizeof(char)*output_length);
    while(strftime(output, output_length,"%Y-%m-%d %H:%M:%S", info) == 0) {
        free(output);
        output_length *= 2;
        output = (char*)malloc(sizeof(char)*output_length);
    }
    return output;
}

void flip_texture(Texture* texture) {
    uint8_t* buffer = (uint8_t*)malloc(texture->buffer_size);
    int dest_i; int y; int x;
    for(int src_i = 0; src_i < texture->buffer_size; src_i++) {
        y = src_i / texture->pitch; x = src_i % texture->pitch;
        dest_i = ((texture->height - y - 1) * texture->pitch) + x;
        buffer[dest_i] = texture->buffer[src_i];
    }
    free(texture->buffer);
    texture->buffer = buffer;
    return;
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

    gl_register_texture(texture);
    return true;
}

bool load_object(Object* object, const char* model_name,
    const char* texture_name, const char* nm_name, const char* spec_name,
    const char* vert_shader_name, const char* frag_shader_name) {

    //Load filenames in object
    object->model_name = model_name; object->texture_name = texture_name;
    object->nm_name = nm_name; object->spec_name = spec_name;
    object->vert_shader_name = vert_shader_name;
    object->frag_shader_name = frag_shader_name;

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

    //Set defaults for our position/rotation/scale information
    object->model->scale = glm::vec3(1.f, 1.f, 1.f);
    object->model->position = glm::vec3(0.f, 0.f, 0.f);
    object->model->rotation = glm::vec3(1.f, 0.f, 0.f);
    object->model->rotation_angle = 0.f;

    gl_register_model(object->model);

    //Shaders
    object->shader_id = glCreateProgram();
    char* path = construct_asset_path("shaders", object->vert_shader_name);
    GLuint shader_id;
    if(!gl_load_shader(path, &shader_id, GL_VERTEX_SHADER)) {
        message_log("Error loading vertex shader-", object->vert_shader_name);
        return false;
    }
    glAttachShader(object->shader_id, shader_id);
    path = construct_asset_path("shaders", object->frag_shader_name);
    if(!gl_load_shader(path, &shader_id, GL_FRAGMENT_SHADER)) {
        message_log("Error loading fragment shader-", object->frag_shader_name);
        return false;
    }
    glAttachShader(object->shader_id, shader_id);
    glBindAttribLocation(object->shader_id, 0, "local_position");
    glBindAttribLocation(object->shader_id, 1, "texture_coord");
    glBindAttribLocation(object->shader_id, 2, "surface_normal");
    glBindAttribLocation(object->shader_id, 3, "surface_tangent");
    glBindAttribLocation(object->shader_id, 4, "surface_bitangent");
    glLinkProgram(object->shader_id);

    return true;
}

void take_screenshot(State* state) {
    //construct path TODO: better way to do string nonsense?
    char* datetime_string = get_datetime_string();
    char* output_path = (char*)malloc(sizeof(char)*255);

    sprintf(output_path, "%s/renderer - %s.png", OutputFolderPath,
        datetime_string);
    message_log("Taking screenshot-", output_path);

    //write screenshot
    glReadPixels(0, 0, state->screen->width, state->screen->height,
        GL_RGB, GL_UNSIGNED_BYTE, state->screen->buffer);
    flip_texture(state->screen);
    lodepng_encode24_file(output_path, (const unsigned char*)state->screen->buffer,
        state->screen->width, state->screen->height);

    free(datetime_string);
    free(output_path);
}
