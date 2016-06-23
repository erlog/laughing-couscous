//Textures
bool load_texture_from_path(Texture* texture) {
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

bool load_texture(const char* filename, Texture* texture) {
    texture->asset_path = construct_asset_path("textures", filename, "png");
    return load_texture_from_path(texture);
}

//Shaders
bool load_shaders(Shader* shader) {
    char* asset_path_vert =
        construct_asset_path("shaders", shader->name, "vert");
    char* asset_path_frag =
        construct_asset_path("shaders", shader->name, "frag");

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
    glAttachShader(program_id, shader_id);

    //Bind shader variables
    shader->id = program_id;
    glBindAttribLocation(shader->id, 0, "local_position");
    glBindAttribLocation(shader->id, 1, "texture_coord");
    glBindAttribLocation(shader->id, 2, "surface_normal");
    glBindAttribLocation(shader->id, 3, "surface_tangent");
    glBindAttribLocation(shader->id, 4, "surface_bitangent");
    glLinkProgram(shader->id);

    wfree(asset_path_vert);
    wfree(asset_path_frag);
    return true;
}

void reload_shaders(State* state) {
    message_log("Reloading shaders");
    //TODO: reload level shaders
    for(int i = 0; i < state->ObjectCount; i++) {
        load_shaders(state->Objects[i].shader);
    }
}

//Physics
void load_physics(Physics_Object* physics) {
    physics->old_position = glm::vec3(0.f, 0.f, 0.f);
    physics->position = glm::vec3(0.f, 0.f, 0.f);
    physics->velocity = 0.f;
    physics->fall_speed = 0.0f;
    physics->deceleration_factor = 26.5f;
    physics->fall_speed = 0.0f;
    physics->quaternion = glm::quat();
    physics->angular_velocity = 0.f;
    physics->rotation_vector = glm::vec3(0.f, 0.f, 0.f);
    physics->movement_vector = glm::vec3(0.f, 0.f, 0.f);
    physics->radii = glm::vec3(1.f, 1.f, 1.f);
}

//Input
void clear_input(Game_Input* input) {
    input->times_polled = 0;
    input->relative_movement = false;
    input->relative_horizontal_movement = 0.0f;
    input->relative_vertical_movement = 0.0f;
    input->relative_camera_movement = false;
    input->relative_camera_x = 0.0f;
    input->relative_camera_y = 0.0f;
    return;
}


//Objects
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

    //Physics
    object->physics = (Physics_Object*)walloc(sizeof(Physics_Object));
    load_physics(object->physics);

    //Model
    object->model = (Model*)walloc(sizeof(Model));
    if(!load_model(model_name, object->model)) {
        message_log("Error loading model-", model_name);
        return false;
    }

    object->light_direction = glm::vec3(0.0f, -1.0f, -1.0f);
    object->model->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    object->model->scale = glm::vec3(1.0f, 1.0f, 1.0f);
    object->model->rotation = glm::mat4();
    object->physics->radii = absolute_difference(
        object->model->bounding_maximum, object->model->bounding_minimum)/2.0f;


    //Shaders
    object->shader = (Shader*)walloc(sizeof(Shader));
    object->shader->name = str_lit(shader_name);
    load_shaders(object->shader);

    //Make OpenGL VBO and VAO
    gl_register_object(object);
    return true;
}

//Fonts
bool load_font(Font* font, const char* font_name) {
    font->asset_path = construct_asset_path("fonts", font_name, "fnt");

    //TODO: consider if we need a full object for this
    font->quad = (Object*)walloc(sizeof(Object));
    font->page = (Texture*)walloc(sizeof(Texture));
    load_object(font->quad, "quad", "blank", "blank_nm", "blank_spec",
        "flat_texture");
    ruby_load_font(font, font->asset_path);
    load_texture_from_path(font->page);
    return true;
}

//Levels
bool load_level(Game_Level* level, const char* level_name) {
    level->asset_path = str_lit("level_name");
    level->geometry = (Object*)walloc(sizeof(Object));
    load_object(level->geometry, level_name,
        "checkerboard", "blank_nm_1024", "checkerboard_spec", "shader");
    level->geometry->model->color = rgb_to_vector(0x13, 0x88, 0x88);
    level->collision_model = (Collision_Model*)walloc(sizeof(Collision_Model));
    level->last_collision = glm::vec3(0.f, 0.f, 0.f);
    load_collision_mesh(level->collision_model, level_name);
    octree_from_level(level);
    return true;
}
