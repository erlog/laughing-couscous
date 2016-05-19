#ifndef RENDERER_H
#define RENDERER_H

//Structs
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

typedef struct c_face_indices {
    //Struct to store vertex indices in order to average tangent/bitangent
    //after the fact
    int a_v_index; int a_n_index; int a_uv_index;
    int b_v_index; int b_n_index; int b_uv_index;
    int c_v_index; int c_n_index; int c_uv_index;
} Indexed_Face;

typedef struct c_vertex {
    glm::vec3 v;    //Vertex
    glm::vec3 uv;   //Texture Coordinate (vec3 for the W coord)
    glm::vec3 n;    //Normal Vector
    glm::vec3 t;    //Tangent Vector
    glm::vec3 b;    //Bitangent Vector
} Vertex;

typedef struct c_face {
    Vertex a;
    Vertex b;
    Vertex c;
} Face;

typedef struct c_model {
    char* asset_path;
    int face_count;
    Face* faces;
    glm::vec3 local_position; //local position offset
    glm::vec3 local_scale; //local scale of the model
    GLfloat local_rotation_angle;
    glm::vec3 local_rotation; //local rotation of the model
    GLuint vbo; //index of the Vertex Buffer object in OpenGL
    GLuint vao; //index of the Vertex Array object in OpenGL
} Model;

typedef struct shader {
    char* asset_path_vert;
    char* asset_path_frag;
    GLuint id; //ID assigned to the shader program by OpenGL
} Shader;

typedef struct c_physics_object {
    glm::vec3 position; //where in world-space something is in meters
    GLfloat velocity;   //how fast something is moving in meters/sec
    GLfloat acceleration; //how much the meters/sec changes over time
    glm::vec3 rotation;  //which direction something is facing
    GLfloat rotation_angle; //how far something is facing in that direction
    glm::vec3 angular_velocity; //how much in degrees/sec rotation changes over time
} Physics_Object;

typedef struct c_object {
    //TODO: write code to free an object from memory
    Model* model;
    Texture* texture;
    Texture* normal_map;
    Texture* specular_map;
    Shader* shader;
    Physics_Object* physics;
} Object;

typedef struct c_camera {
    glm::vec3 position;
    GLfloat velocity; //velocity in units per second
    GLfloat rotational_velocity;
    GLfloat rotation_speed;
    GLfloat deceleration;
    glm::vec3 facing;
    glm::vec3 orientation;
    GLfloat yaw;
    GLfloat pitch;
    glm::mat4 projection;
} Scene_Camera;

typedef struct c_state {
    bool IsRunning;
    bool IsPaused;
    Texture* Screen;
    uint32_t StartTime;
    uint32_t CurrentTime;
    uint32_t LastUpdateTime;
    uint32_t DeltaTimeMS;
    float DeltaTimeS;
    uint32_t FrameCounter;
    uint32_t LastFPSUpdateTime;
    Object* Objects;
    int ObjectCount;
    Scene_Camera* Camera;
} State;

typedef struct c_memory {
    size_t MemoryAllocated;
    size_t MemoryFreed;
} Memory_Info;

//Globals
const char* AssetFolderPath = "objects";
const char* OutputFolderPath = "output";

//Generic utility functions
//TODO: re-arrange source to not require any functions here
char* construct_asset_path(const char* folder, const char* filename,
        const char* file_extension);

//Debug Functions
Memory_Info* Global_State; //use this only for debug
void* wrapped_alloc(size_t size) {
    void* pointer = malloc(size);
    size_t real_size = malloc_usable_size(pointer);
    Global_State->MemoryAllocated += real_size;
    //printf("(Mem) %lu B Allocated\n", real_size);
    return pointer;
}

void* wrapped_realloc(void* pointer, size_t new_size) {
    size_t real_size = malloc_usable_size(pointer);
    Global_State->MemoryAllocated -= real_size;
    //printf("(Mem) %lu B Deallocated\n", real_size);
    void* new_pointer = realloc(pointer, new_size);
    real_size = malloc_usable_size(new_pointer);
    Global_State->MemoryAllocated += real_size;
    //printf("(Mem) %lu B Allocated\n", real_size);
    return new_pointer;
}

void wrapped_free(void* pointer) {
    if(pointer == NULL) { return; }
    size_t real_size = malloc_usable_size(pointer);
    Global_State->MemoryFreed += real_size;
    //printf("(Mem) %lu B Deallocated\n", real_size);
    free(pointer); return;
}

void* (*walloc)(size_t size) = wrapped_alloc;
void* (*wrealloc)(void* pointer, size_t new_size) = wrapped_realloc;
void (*wfree)(void* pointer) = wrapped_free;
//void* (*walloc)(size_t size) = malloc;
//void* (*wrealloc)(void* pointer, size_t new_size) = realloc;
//void (*wfree)(void* pointer) = free;

#endif

