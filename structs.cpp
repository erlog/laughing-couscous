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

void wfree_texture(Texture* object) {
    wfree(object->asset_path);
    wfree(object->buffer); return;
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

typedef struct c_quad_face {
    Vertex a; Vertex b; Vertex c; Vertex d;
    glm::vec3 center;
    glm::vec3 radii;
    glm::vec3 normal;
} QuadFace;

typedef struct c_model {
    char* asset_path;
    GLuint face_count;
    Face* faces;
    glm::vec4 color;
    glm::vec3 bounding_minimum;
    glm::vec3 bounding_maximum;
    glm::vec3 local_position; //local position offset
    glm::vec3 local_scale; //local scale of the model
    glm::quat local_quaternion; //local orientation of the model
} Model;

void wfree_model(Model* object) {
    wfree(object->asset_path);
    wfree(object->faces);
    return;
}

typedef struct c_quad_model {
    char* asset_path;
    GLuint face_count;
    QuadFace* faces;
    glm::vec3 bounding_minimum;
    glm::vec3 bounding_maximum;
} QuadModel;

void wfree_quadmodel(QuadModel* object) {
    wfree(object->asset_path);
    wfree(object->faces);
    return;
}

typedef struct shader {
    char* name;
    GLuint id; //ID assigned to the shader program by OpenGL
} Shader;

void wfree_shader(Shader* object) {
    wfree(object->name);
    return;
}

//Game Objects
typedef struct c_physics_object {
    glm::vec3 position; //where in world-space something is in meters
    GLfloat velocity;   //how fast something is moving in meters/sec
    GLfloat fall_speed;
    GLfloat deceleration_factor;
    glm::quat quaternion; //orientation
    GLfloat angular_velocity;   //how fast something is rotating in deg/sec
    glm::vec3 rotation_vector;   //the axis of rotation
    glm::vec3 movement_vector;  //the axis of movement
    glm::vec3 scale; //world scale of the object
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
    wfree_model(object->model);
    wfree_texture(object->texture);
    wfree_texture(object->normal_map);
    wfree_texture(object->specular_map);
    wfree_shader(object->shader);
    wfree(object->physics);
}

typedef struct c_camera {
    Physics_Object* physics;
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 direction;
} Scene_Camera;

void wfree_camera(Scene_Camera* object) {
    wfree(object->physics);
    return;
}

//Levels
typedef struct c_octree_node {
    uint32_t filled;
    uint8_t depth;
    GLfloat hard_radius;
    GLfloat soft_radius;
    QuadFace** faces;
    uint32_t face_count;
    glm::vec3 position;
    c_octree_node* parent;
    c_octree_node* children;
} Octree_Node;

void wfree_octree_node(Octree_Node* object) {
    if(object->children != NULL) {
        for(int i = 0; i < 8; i++) {
            wfree_octree_node(&object->children[i]);
        }
    }

    wfree(object->children);
    wfree(object->faces);
    return;
}

typedef struct c_octree {
    uint32_t max_depth;
    c_octree_node root;
} Octree;

void wfree_octree(Octree* object) {
    wfree_octree_node(&object->root);
    return;
}

typedef struct c_level {
    char* asset_path;
    Object* geometry;
    QuadModel* collision_model;
    Octree* octree;
} Game_Level;

void wfree_game_level(Game_Level* object) {
    wfree(object->asset_path);
    wfree_object(object->geometry);
    wfree_quadmodel(object->collision_model);
    wfree_octree(object->octree);
    return;
}

//Game State
typedef struct c_settings {
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    bool vsync;
    bool fullscreen;
} Settings_Object;

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
    Object* Debug_Cube;
    Object* Objects;
    int ObjectCount;
    Game_Level* Level;
    int StaticObjectCount;
    Scene_Camera* Camera;
    SDL_Window* Window;
} State;

void wfree_state(State* object) {
    wfree_texture(object->Screen);
    for(int i = 0; i < object->ObjectCount; i++) {
        wfree_object(&object->Objects[i]);
    }
    wfree(object->Objects);
    wfree_object(object->Debug_Cube);
    wfree(object->Debug_Cube);
    wfree_game_level(object->Level);
    wfree(object->Level);
    wfree_camera(object->Camera);
    wfree(object->Camera);
    return;
}

