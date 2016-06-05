#ifndef RENDERER_H
#define RENDERER_H
//C Standard Library
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

//#define MAC_COMPILE 0
//#define LINUX_COMPILE 1

//Namespaces
//using namespace std

//Globals
const char* AssetFolderPath = "objects";
const char* OutputFolderPath = "new_output";
const char* SettingsINIPath = "settings.ini";

//Debug Functions
#if MAC_COMPILE
    #include <malloc/malloc.h>
#endif
#if LINUX_COMPILE
    #include <malloc.h>
#endif
//Memory Manager
typedef struct c_memory {
    size_t MemoryAllocated;
    size_t MemoryFreed;
} Memory_Info;

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

#if 1 //Logging for memory
    #define walloc wrapped_alloc
    #define wrealloc wrapped_realloc
    #define wfree(VARIABLE) \
    do { puts("---"); } \
    while(0)
#else
    #define walloc malloc
    #define wrealloc realloc
    #define wfree free
#endif

//Other Libraries
#include <GL/glew.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <SDL.h>
#include <SDL_opengl.h>
#if MAC_COMPILE
    #include <SDL_opengl_glext.h>
#endif
#if LINUX_COMPILE
    #include <GL/glext.h>
#endif
#include <ruby.h>
//Local Includes
#include "lodepng.hpp"
#include "structs.cpp"
#include "logging.cpp"
#include "utilities.cpp"
#include "ini_handling.cpp"
#include "gl_functions.cpp"
#include "octree.cpp"
#include "loader_model.cpp"
#include "loaders.cpp"
#include "hid_input.cpp"
//#include "ruby_functions.cpp"

#endif
