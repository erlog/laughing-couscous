//Texture
typedef struct c_texture {
    char* asset_path;
    GLuint id;  //ID assigned to us by OpenGL
    GLsizei width;
    GLsizei height;
    int bytes_per_pixel;
    int pitch;  //width in bytes of each row of the image
    int buffer_size;
    uint8_t* buffer;
} Texture;

void wfree_texture(Texture* texture) {
    wfree(texture->asset_path);
    wfree(texture->buffer);
    return;
}

//3D Model
typedef struct c_vertex {
    glm::vec3 v;    //Vertex
    glm::vec3 uv;   //Texture Coordinate (vec3 for the W coord)
    glm::vec3 n;    //Normal Vector
    glm::vec3 t;    //Tangent Vector
    glm::vec3 b;    //Bitangent Vector
} Vertex;

typedef struct c_face {
    Vertex a; Vertex b; Vertex c;
} Face;

typedef struct c_collision_face {
    Vertex a; Vertex b; Vertex c;
    glm::vec3 center;
    glm::vec3 radii;
    glm::vec3 normal;
    glm::vec3 maximum;
    glm::vec3 minimum;
    GLfloat distance; //planar equation distance d = -1(ax+by+cz)
} Collision_Face;

typedef struct c_model {
    char* asset_path;
    GLuint face_count;
    Face* faces;
    glm::vec4 color;
    glm::mat4 rotation;
    glm::vec3 scale;
    glm::vec3 bounding_minimum;
    glm::vec3 bounding_maximum;
} Model;

void wfree_model(Model* model) {
    wfree(model->asset_path);
    wfree(model->faces);
    return;
}

typedef struct c_collision_model {
    char* asset_path;
    GLuint face_count;
    Collision_Face* faces;
    glm::vec3 bounding_minimum;
    glm::vec3 bounding_maximum;
} Collision_Model;

void wfree_collision_model(Collision_Model* collision_model) {
    wfree(collision_model->asset_path);
    wfree(collision_model->faces);
    return;
}

typedef struct shader {
    char* name;
    GLuint id; //ID assigned to the shader program by OpenGL
} Shader;

void wfree_shader(Shader* shader) {
    wfree(shader->name);
    return;
}


//Game Objects
typedef struct c_physics_object {
    glm::vec3 old_position;
    glm::vec3 position; //where in world-space something is in meters
    GLfloat time_remaining;
    GLfloat velocity;   //how fast something is moving in meters/sec
    GLfloat fall_speed;
    GLfloat deceleration_factor;
    glm::quat quaternion; //movement orientation
    GLfloat angular_velocity;   //how fast something is rotating in deg/sec
    glm::vec3 rotation_vector;   //the axis of rotation
    glm::vec3 movement_vector;  //the axis of movement
    glm::vec3 radii; //size of each axis of object
} Physics_Object;

typedef struct c_object {
    Model* model;
    Texture* texture;
    Texture* normal_map;
    Texture* specular_map;
    Shader* shader;
    Physics_Object* physics;
    glm::vec3 light_direction;
    GLuint vbo; //index of the Vertex Buffer object in OpenGL
    GLuint vao; //index of the Vertex Array object in OpenGL
} Object;

void wfree_object(Object* object) {
    wfree_model(object->model); wfree(object->model);
    wfree_texture(object->texture); wfree(object->texture);
    wfree_texture(object->normal_map); wfree(object->normal_map);
    wfree_texture(object->specular_map); wfree(object->specular_map);
    wfree_shader(object->shader); wfree(object->shader);
    wfree(object->physics);
}

typedef struct c_camera {
    Physics_Object* physics;
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 direction;
} Scene_Camera;

void wfree_camera(Scene_Camera* camera) {
    wfree(camera->physics);
    return;
}

//Text
typedef struct c_glyph {
    //TODO: Unicode!
    char char_id;
    GLuint page_id;
    glm::vec4 uv_info; //(UV space) top left coord and size
    glm::vec3 size;    //(Glyph space) size of the character
    glm::vec3 offset;  //(Glyph space) new center of the character
    glm::vec3 advance; //(Glyph space) amount to advance pen by after drawing
} Glyph;

typedef struct c_font {
    char* asset_path;
    GLfloat size;
    Texture* page;
    Object* quad;
    //TODO: Unicode!
    std::unordered_map<char, Glyph> glyphs;
} Font;

//Levels
typedef struct c_octree_node {
    uint32_t filled;
    uint8_t depth;
    GLfloat hard_radius;
    GLfloat soft_radius;
    Collision_Face** faces;
    uint32_t face_count;
    glm::vec3 position;
    c_octree_node* parent;
    c_octree_node* children;
} Octree_Node;

void wfree_octree_node(Octree_Node* node) {
    if(node->children != NULL) {
        for(int i = 0; i < 8; i++) {
            wfree_octree_node(&node->children[i]);
        }
    }

    wfree(node->children);
    wfree(node->faces);
    return;
}

typedef struct c_octree {
    uint32_t max_depth;
    c_octree_node root;
} Octree;

void wfree_octree(Octree* octree) {
    wfree_octree_node(&octree->root);
    return;
}

typedef struct c_level {
    char* asset_path;
    Object* geometry;
    Collision_Model* collision_model;
    Octree* octree;
    //DEBUG STUFF BELOW
    glm::vec3 last_collision;
} Game_Level;

void wfree_game_level(Game_Level* level) {
    wfree(level->asset_path);
    wfree_object(level->geometry); wfree(level->geometry);
    wfree_collision_model(level->collision_model); wfree(level->collision_model);
    wfree_octree(level->octree); wfree(level->octree);
    return;
}

//Game State
typedef struct c_settings {
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    bool vsync;
    bool fullscreen;
} Settings_Object;

typedef struct c_game_input {
    uint32_t times_polled;
    bool relative_movement;
    GLfloat relative_horizontal_movement;
    GLfloat relative_vertical_movement;
    bool relative_camera_movement;
    GLfloat relative_camera_x;
    GLfloat relative_camera_y;
} Game_Input;

typedef struct c_state {
    bool IsRunning;
    bool IsPaused;
    Settings_Object Settings;
    Texture* Screen;
    //Timers
    uint32_t WallTime; //in wall time
    uint32_t PauseStartTime; //in wall time
    uint32_t TimeDifference; //difference between wall time and game time
    uint32_t GameTime; //in game time
    uint32_t LastUpdateTime; //in game time
    uint32_t DeltaTimeMS; //in game time
    float DeltaTimeS; //in game time
    uint32_t FrameCounter;
    uint32_t LastFPSUpdateTime;
    Game_Input* Input;
    Object* Debug_Cube;
    Object* Debug_Sphere;
    Object* Player;
    Object* Objects;
    int ObjectCount;
    Game_Level* Level;
    int StaticObjectCount;
    Scene_Camera* Camera;
    SDL_Window* Window;
} State;

void wfree_state(State* state) {
    wfree_texture(state->Screen); wfree(state->Screen);
    for(int i = 0; i < state->ObjectCount; i++) {
        wfree_object(&state->Objects[i]);
    }
    wfree(state->Objects);
    wfree(state->Input);
    wfree_object(state->Debug_Cube); wfree(state->Debug_Cube);
    wfree_object(state->Debug_Sphere); wfree(state->Debug_Sphere);
    wfree_object(state->Player); wfree(state->Player);
    wfree_game_level(state->Level); wfree(state->Level);
    wfree_camera(state->Camera); wfree(state->Camera);
    return;
}

