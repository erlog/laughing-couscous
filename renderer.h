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
    GLuint face_count;
    Face* faces;
    glm::vec4 color;
    glm::vec3 bounding_minimum;
    glm::vec3 bounding_maximum;
    glm::vec3 local_position; //local position offset
    glm::vec3 local_scale; //local scale of the model
    glm::quat local_quaternion; //local orientation of the model
} Model;

typedef struct shader {
    char* name;
    GLuint id; //ID assigned to the shader program by OpenGL
} Shader;

typedef struct c_physics_object {
    glm::vec3 position; //where in world-space something is in meters
    GLfloat velocity;   //how fast something is moving in meters/sec
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

typedef struct c_camera {
    Physics_Object* physics;
    glm::mat4 projection;
} Scene_Camera;

typedef struct c_settings {
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    bool vsync;
    bool fullscreen;
} Settings_Object;

typedef struct c_state {
    bool IsRunning;
    bool IsPaused;
    Settings_Object* Settings;
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
    Object* Objects;
    int ObjectCount;
    Object* StaticObjects;
    int StaticObjectCount;
    Scene_Camera* Camera;
    SDL_Window* Window;
} State;

typedef struct c_memory {
    size_t MemoryAllocated;
    size_t MemoryFreed;
} Memory_Info;

//Globals
const char* AssetFolderPath = "objects";
const char* OutputFolderPath = "new_output";
const char* SettingsINIPath = "settings.ini";

//Generic utility functions
//TODO: re-arrange source to not require any functions here
char* construct_asset_path(const char* folder, const char* filename,
        const char* file_extension);

//Debug Functions
#if MAC_COMPILE
    #include <malloc/malloc.h>
#endif
#if LINUX_COMPILE
    #include <malloc.h>
#endif
Memory_Info* Global_State; //use this only for debug
void* wrapped_alloc(size_t size) {
    void* pointer = malloc(size);
    #if LINUX_COMPILE
        size_t real_size = malloc_usable_size(pointer);
    #endif
    #if MAC_COMPILE
        size_t real_size = malloc_size(pointer);
    #endif
    Global_State->MemoryAllocated += real_size;
    //printf("(Mem) %lu B Allocated\n", real_size);
    return pointer;
}

void* wrapped_realloc(void* pointer, size_t new_size) {
    #if LINUX_COMPILE
        size_t real_size = malloc_usable_size(pointer);
    #endif
    #if MAC_COMPILE
        size_t real_size = malloc_size(pointer);
    #endif
    Global_State->MemoryAllocated -= real_size;
    //printf("(Mem) %lu B Deallocated\n", real_size);
    void* new_pointer = realloc(pointer, new_size);
    #if LINUX_COMPILE
        real_size = malloc_usable_size(new_pointer);
    #endif
    #if MAC_COMPILE
        real_size = malloc_size(new_pointer);
    #endif
    Global_State->MemoryAllocated += real_size;
    //printf("(Mem) %lu B Allocated\n", real_size);
    return new_pointer;
}

void wrapped_free(void* pointer) {
    if(pointer == NULL) { return; }
    #if LINUX_COMPILE
        size_t real_size = malloc_usable_size(pointer);
    #endif
    #if MAC_COMPILE
        size_t real_size = malloc_size(pointer);
    #endif
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

