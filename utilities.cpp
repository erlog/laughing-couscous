//Small functions that don't fit anywhere else with minimal dependencies
void clamp(GLfloat* number, GLfloat min, GLfloat max) {
    if(*number > max) { *number = max; return; }
    if(*number < max) { *number = min; return; }
}
void clamp(int* number, int min, int max) {
    if(*number > max) { *number = max; return; }
    if(*number < max) { *number = min; return; }
}

glm::vec4 rgb_to_vector(uint8_t r, uint8_t g, uint8_t b) {
    return glm::vec4(r/255.f, g/255.f, b/255.f, 1.f);
}

char* str_lit(const char* string) {
    int length = strlen(string) + 1;
    char* result = (char *)walloc(sizeof(char)*length);
    strcpy(result, string);
    return result;
}

char* construct_asset_path(const char* folder, const char* filename,
        const char* file_extension) {
    //TODO: dynamically allocate appropriately sized string;
    //TODO: decide which of these are actually const
    char* buffer = (char*)walloc(sizeof(char)*255);
    snprintf(buffer, 254, "%s/%s/%s.%s", AssetFolderPath, folder,
        filename, file_extension);
    return buffer;
}

char* get_datetime_string() {
    time_t rawtime; struct tm *info;
    time( &rawtime ); info = localtime( &rawtime );

    int output_length = 16;
    char* output = (char*)walloc(sizeof(char)*output_length);
    while(strftime(output, output_length,"%Y-%m-%d %H:%M:%S", info) == 0) {
        wfree(output);
        output_length *= 2;
        output = (char*)walloc(sizeof(char)*output_length);
    }
    return output;
}

void flip_texture(Texture* texture) {
    uint8_t* buffer = (uint8_t*)walloc(texture->buffer_size);
    int dest_i; int y; int x;
    for(int src_i = 0; src_i < texture->buffer_size; src_i++) {
        y = src_i / texture->pitch; x = src_i % texture->pitch;
        dest_i = ((texture->height - y - 1) * texture->pitch) + x;
        buffer[dest_i] = texture->buffer[src_i];
    }
    wfree(texture->buffer);
    texture->buffer = buffer;
    return;
}

bool load_texture(const char* filename, Texture* texture) {
    texture->asset_path = construct_asset_path("textures", filename, "png");

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

bool load_shaders(Object* object) {
    char* asset_path_vert =
        construct_asset_path("shaders", object->shader->name, "vert");
    char* asset_path_frag =
        construct_asset_path("shaders", object->shader->name, "frag");

    GLint program_id = glCreateProgram();
    GLuint shader_id;

    if(!gl_load_shader(asset_path_vert, &shader_id, GL_VERTEX_SHADER)) {
        message_log("Error loading vertex shader-", asset_path_vert);
        return false;
    }
    glAttachShader(program_id, shader_id);
    if(!gl_load_shader(asset_path_frag, &shader_id, GL_FRAGMENT_SHADER)) {
        message_log("Error loading fragment shader-", asset_path_frag);
        return false;
    }

    object->shader->id = program_id;
    glAttachShader(program_id, shader_id);
    glBindAttribLocation(object->shader->id, 0, "local_position");
    glBindAttribLocation(object->shader->id, 1, "texture_coord");
    glBindAttribLocation(object->shader->id, 2, "surface_normal");
    glBindAttribLocation(object->shader->id, 3, "surface_tangent");
    glBindAttribLocation(object->shader->id, 4, "surface_bitangent");
    glLinkProgram(object->shader->id);

    //Bind textures
    gl_bind_texture(object->shader->id, object->texture, 0, "diffuse");
    gl_bind_texture(object->shader->id, object->normal_map, 1, "normal");
    gl_bind_texture(object->shader->id, object->specular_map, 2, "specular");

    wfree(asset_path_vert);
    wfree(asset_path_frag);
    return true;
}

void load_physics(Physics_Object* physics) {
    physics->position = glm::vec3(0.f, 0.f, 0.f);
    physics->velocity = 0.f;
    physics->deceleration_factor = 26.5f;
    physics->quaternion = glm::quat();
    physics->angular_velocity = 0.f;
    physics->rotation_vector = glm::vec3(0.f, 0.f, 0.f);
    physics->movement_vector = glm::vec3(0.f, 0.f, 0.f);
    physics->scale = glm::vec3(1.f, 1.f, 1.f);
}

bool load_object(Object* object, const char* model_name,
    const char* texture_name, const char* nm_name, const char* spec_name,
    const char* shader_name) {

    //Texture
    object->texture = (Texture*)walloc(sizeof(Texture));
    if(!load_texture(texture_name, object->texture)) {
        message_log("Error loading texture-", texture_name);
        return false;
    }
    //Normal Map
    object->normal_map= (Texture*)walloc(sizeof(Texture));
    if(!load_texture(nm_name, object->normal_map)) {
        message_log("Error loading normal map-", nm_name);
        return false;
    }
    //Specular Map
    object->specular_map = (Texture*)walloc(sizeof(Texture));
    if(!load_texture(spec_name, object->specular_map)) {
        message_log("Error loading specular map-", spec_name);
        return false;
    }

    //Model
    object->model = (Model*)walloc(sizeof(Model));
    if(!load_model(model_name, object->model)) {
        message_log("Error loading model-", model_name);
        return false;
    }

    object->model->color = glm::vec4(1.f, 1.f, 1.f, 1.f);
    object->model->local_position = glm::vec3(0.f, 0.f, 0.f);
    object->model->local_scale = glm::vec3(1.f, 1.f, 1.f);
    object->model->local_quaternion = glm::quat();

    //Physics
    object->physics = (Physics_Object*)walloc(sizeof(Physics_Object));
    load_physics(object->physics);

    //Shaders
    object->shader = (Shader*)walloc(sizeof(Shader));
    object->shader->name = str_lit(shader_name);
    load_shaders(object);

    //Bind textures
    gl_bind_texture(object->shader->id, object->texture, 0, "diffuse");
    gl_bind_texture(object->shader->id, object->normal_map, 1, "normal");
    gl_bind_texture(object->shader->id, object->specular_map, 2, "specular");

    //Make OpenGL VBO and VAO
    gl_register_object(object);
    return true;
}

void take_screenshot(State* state) {
    //construct path TODO: better way to do string nonsense?
    char* datetime_string = get_datetime_string();
    char* output_path = (char*)walloc(sizeof(char)*255);

    sprintf(output_path, "%s/renderer - %s.png", OutputFolderPath,
        datetime_string);
    message_log("Taking screenshot-", output_path);

    //write screenshot
    glReadPixels(0, 0, state->Screen->width, state->Screen->height,
        GL_RGB, GL_UNSIGNED_BYTE, state->Screen->buffer);
    flip_texture(state->Screen);
    lodepng_encode24_file(output_path, (const unsigned char*)state->Screen->buffer,
        state->Screen->width, state->Screen->height);

    wfree(datetime_string);
    wfree(output_path);
}

void reload_shaders(State* state) {
    message_log("Reloading shaders");
    for(int i = 0; i < state->StaticObjectCount; i++) {
        load_shaders(&state->StaticObjects[i]);
    }

    for(int i = 0; i < state->ObjectCount; i++) {
        load_shaders(&state->Objects[i]);
    }
}

void update_time(State* state) {
    state->WallTime = SDL_GetTicks();
    state->GameTime = state->WallTime - state->TimeDifference;
    state->DeltaTimeMS = state->GameTime - state->LastUpdateTime;
    state->DeltaTimeS = state->DeltaTimeMS / 1000.f;
    return;
}

void toggle_pause(State* state) {
    if(state->IsPaused) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_GetRelativeMouseState(NULL, NULL); //to fix the jumping problem
        state->IsPaused = false;
        state->TimeDifference = state->WallTime - state->PauseStartTime;
        update_time(state);
        message_log("Unpaused-", state->GameTime);
    } else {
        message_log("Pausing-", state->WallTime);
        SDL_SetRelativeMouseMode(SDL_FALSE);
        state->IsPaused = true;
        state->PauseStartTime = state->GameTime;
        message_log("Paused-", state->GameTime);
    }
    return;
}
