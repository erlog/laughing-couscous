//Compile-time Flags
    //#define MAC_COMPILE 0
    //#define LINUX_COMPILE 1
    #define MEMORY_LOGGING 0
    #define DEBUG 1

//C Standard Library
    #include <stdlib.h>
    #include <time.h>
    #include <float.h>
    #include <math.h>
    #include <stdarg.h>
    #include <stdio.h>

//C++ Standard Library
    #include <unordered_map>

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
        #define malloc_size malloc_usable_size
    #endif

//Memory Usage Tracking Functions
    typedef struct c_memory {
        size_t MemoryAllocated;
        size_t MemoryFreed;
    } Memory_Info;

    Memory_Info* Global_State; //use this only for debug
    void* wrapped_alloc(size_t size, const char* label) {
        void* pointer = malloc(size);
        size_t real_size = malloc_size(pointer);
        Global_State->MemoryAllocated += real_size;
        #if MEMORY_LOGGING
        if(real_size > 0) {
            printf("%lu B + Allocated (%s)\n", real_size, label);
        }
        #endif
        return pointer;
    }

    void* wrapped_realloc(void* pointer, size_t new_size, const char* label) {
        size_t real_size = malloc_size(pointer);
        Global_State->MemoryAllocated -= real_size;
        #if MEMORY_LOGGING
        if(real_size > 0) {
            printf("%lu B - (%s)\n", real_size, label);
        }
        #endif
        void* new_pointer = realloc(pointer, new_size);
        real_size = malloc_size(new_pointer);
        Global_State->MemoryAllocated += real_size;
        #if MEMORY_LOGGING
        if(real_size > 0) {
            printf("%lu B + (%s)\n", real_size, label);
        }
        #endif
        return new_pointer;
    }

    void wrapped_free(void* pointer, const char* label) {
        size_t real_size = malloc_size(pointer);
        Global_State->MemoryFreed += real_size;
        #if MEMORY_LOGGING
        if(real_size > 0) {
            printf("%lu B - (%s)\n", real_size, label);
        }
        #endif
        free(pointer); return;
    }

    #if DEBUG //Logging for memory
        #define walloc(SIZE) wrapped_alloc(SIZE, #SIZE)
        #define wrealloc(POINTER, SIZE) wrapped_realloc(POINTER, SIZE, #POINTER)
        #define wfree(POINTER) wrapped_free(POINTER, #POINTER)
    #else
        #define walloc malloc
        #define wrealloc realloc
        #define wfree free
    #endif

//Other Libraries
    //ICU - International Components for Unicode
    #define U_CHARSET_IS_UTF8 1
    #include <unicode/utypes.h>
    #include <unicode/putil.h>
    #include <unicode/ustdio.h>

    //GLEW - OpenGL Extension Wrangler
    #include <GL/glew.h>

    //GLM - OpenGL Math Library
    #define GLM_FORCE_RADIANS
    #include <glm/glm.hpp>
    #include <glm/gtc/matrix_transform.hpp>
    #include <glm/gtc/type_ptr.hpp>
    #include <glm/gtc/constants.hpp>
    #include <glm/gtx/euler_angles.hpp>

    //ASSIMP - Model Parser
    #include <assimp/Importer.hpp>
    #include <assimp/scene.h>
    #include <assimp/postprocess.h>

    //SDL w/ OpenGL
    #include <SDL.h>
    #include <SDL_opengl.h>
    #if MAC_COMPILE
        #include <SDL_opengl_glext.h>
    #endif
    #if LINUX_COMPILE
        #include <GL/glext.h>
    #endif

    //Ruby
    #include <ruby.h>

//Local Libraries
    #include "lodepng.hpp"

//Local Includes
    #include "structs.hpp"
    #include "logging.cpp"
    #include "utilities.cpp"
    #include "ruby_functions.cpp"
    #include "gl_functions.cpp"
    #include "octree.cpp"
    #include "physics.cpp"
    #include "loader_string.cpp"
    #include "loader_font.cpp"
    #include "loader_model.cpp"
    #include "loader_ini.cpp"
    #include "loaders.cpp"
    #include "hid_input.cpp"

