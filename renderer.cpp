//C Standard Library
#include <time.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <malloc.h>
//C++ Stuff

//Other Libraries
#include <GL/glew.h>
#include <GL/glext.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL.h>
#include <SDL_opengl.h>
//#include <SDL_opengl_glext.h>
#include <ruby.h>
//Namespaces
using namespace std;
//Local Headers
#include "renderer.h"
//Local Includes
#include "lodepng.hpp"
#include "free_structs.cpp"
#include "logging.cpp"
#include "gl_functions.cpp"
#include "wavefront.cpp"
#include "utilities.cpp"
#include "hid_input.cpp"
//#include "ruby_functions.cpp"

uint32_t current_time() {
    return SDL_GetTicks();
}

void update_physics_object(Physics_Object* object, float delta_time_s) {
    //rotate
    #if 1
    object->quaternion = glm::rotate(object->quaternion,
        glm::radians(delta_time_s * object->angular_velocity),
        object->rotation_vector);
    #endif
    //translate
    object->position += (delta_time_s *
        object->velocity * object->facing * object->quaternion);
    //decelerate
    object->velocity *= (delta_time_s * object->deceleration_factor);
    object->angular_velocity *= (delta_time_s * object->deceleration_factor);
    return;
}

int main() {
    //INITIALIZATION- Failures here cause a hard exit
    Memory_Info mem_info = {0}; Global_State = &mem_info;
    State* state = (State*)walloc(sizeof(State));
    state->IsRunning = true;

    //Initialize screen struct and buffer for taking screenshots
    state->Screen = (Texture*)walloc(sizeof(Texture));
    state->Screen->asset_path = str_lit("Flamerokz");
    state->Screen->width = 682;
    state->Screen->height = 384;
    state->Screen->bytes_per_pixel = 3;
    state->Screen->pitch = state->Screen->width * state->Screen->bytes_per_pixel;
    state->Screen->buffer_size = state->Screen->pitch * state->Screen->height;
    state->Screen->buffer = (uint8_t*)walloc(state->Screen->buffer_size);

    //Start Ruby
    //ruby_setup_render_environment();
    //VALUE rb_update_func = rb_intern("ruby_update");

    //Initialize SDL and OpenGL
    SDL_Event event;
    int error = SDL_Init(SDL_INIT_VIDEO); if(error != 0) {
        message_log("SDL Init Error, Code", error); return 0;
    }
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 ); //Use OpenGL 3.1
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window* window = SDL_CreateWindow( state->Screen->asset_path,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, state->Screen->width,
        state->Screen->height, SDL_WINDOW_OPENGL);
    if(window == NULL) {
        message_log("Couldn't initialize-", "SDL OpenGL window"); return 0;
    }
    SDL_GLContext context = SDL_GL_CreateContext( window );
    if(context == NULL) {
        message_log("Couldn't get-", "OpenGL Context for window"); return 0;
    }
    if(SDL_GL_SetSwapInterval(1) != 0) { //try for vsync
        SDL_GL_SetSwapInterval(-1); //late-swap tearing if the vsync call fails
    }

    const uint8_t* SDL_KeyState = SDL_GetKeyboardState(NULL);

    //GLEW
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        message_log("Couldn't initialize-", "GLEW"); return 0;
    }
    if(!GLEW_VERSION_2_1) {
        message_log("OpenGL 2.1 not supported by GLEW", ""); return 0;
    }

    //Set up simple OpenGL environment for rendering
    glMatrixMode( GL_PROJECTION ); glLoadIdentity();
    glMatrixMode( GL_MODELVIEW ); glLoadIdentity();
    glClearColor( 0.f, 0.f, 0.f, 0.f );
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CCW); //Default is CCW, counter-clockwise
    //glDepthRange(1.0, -1.0); //change the handedness of the z axis
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //GAME INIT- Failures here may cause a proper smooth exit when necessary

    //Construct Camera
    state->Camera = (Scene_Camera*)walloc(sizeof(Scene_Camera));
    state->Camera->physics = (Physics_Object*)walloc(sizeof(Physics_Object));
    load_physics(state->Camera->physics);
    state->Camera->physics->position = glm::vec3(0.f, 0.f, 3.f);
    state->Camera->physics->facing = glm::vec3(0.f, 0.f, -1.f);

    //Construct Camera Matrices
    glm::mat4 projection_matrix;
    projection_matrix = glm::perspective(glm::radians(45.f),
        (float)state->Screen->width/state->Screen->height, 0.1f, 100.f);
    state->Camera->projection = projection_matrix;


    //Load Objects
    state->ObjectCount = 3;
    state->Objects = (Object*)walloc(sizeof(Object)*3);
    load_object(&state->Objects[0], "cube", "blank", "blank_nm", "blank_spec",
        "flat");
    state->Objects[0].physics->position = glm::vec3(0.f, 0.f, 0.f);
    state->Objects[0].physics->rotation_vector = glm::vec3(1.f, 0.f, 0.f);
    load_object(&state->Objects[1], "wt_teapot", "blank", "blank_nm",
        "blank_spec", "flat");
    state->Objects[1].model->local_position = glm::vec3(0.f, -0.5f, 0.f);
    state->Objects[1].physics->position = glm::vec3(1.5f, 0.f, 0.f);
    state->Objects[1].physics->rotation_vector = glm::vec3(0.f, 1.f, 0.f);
    load_object(&state->Objects[2], "african_head", "african_head",
        "african_head_nm", "african_head_spec", "shader");
    state->Objects[2].physics->position = glm::vec3(-1.5f, 0.f, 0.f);
    state->Objects[2].physics->rotation_vector = glm::vec3(0.f, 0.f, 1.f);

    Object* object;

    state->StartTime = current_time();
    state->LastFPSUpdateTime = state->StartTime;

    //MAIN LOOP- Failures here may cause a proper smooth exit when necessary
    message_log("Starting update loop.", "");

    while(state->IsRunning) {
        state->CurrentTime = current_time();
        state->DeltaTimeMS = state->CurrentTime - state->LastUpdateTime;
        state->DeltaTimeS = state->DeltaTimeMS / 1000.f;

        //Process keyboard state
        SDL_PumpEvents();
        process_keyboard(state, SDL_KeyState);

        //Process keydown events
        while(SDL_PollEvent(&event)) { switch(event.type) {
            case SDL_WINDOWEVENT:
                break;

            case SDL_KEYDOWN:
                handle_keyboard(state, event.key);
                break;

            case SDL_QUIT:
                state->IsRunning = false;
                break;
        } }

        //TODO: is there a better way to control our framerate?
        if( state->DeltaTimeMS > 32 ) {
            //rb_funcall(rb_cObject, rb_update_func, 0, NULL);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //Update physics objects
            update_physics_object(state->Camera->physics, state->DeltaTimeS);

            //draw objects
            for(int i = 0; i < state->ObjectCount; i++) {
                state->Objects[i].physics->angular_velocity = 30.f;
                update_physics_object(state->Objects[i].physics, state->DeltaTimeS);
                gl_draw_object(state->Camera, &state->Objects[i]);
            }

            gl_draw_debug_grid_lines();

            SDL_GL_SwapWindow(window);
            state->LastUpdateTime = state->CurrentTime;
            state->FrameCounter += 1;

            //Spit out debug info
            if(state->FrameCounter > 100) {
                float fps = (float)state->FrameCounter;
                fps /= (state->CurrentTime - state->LastFPSUpdateTime);

                message_log("FPS-", fps*1000);
                message_log("Memory in use-", mem_info.MemoryAllocated -
                    mem_info.MemoryFreed);

                state->FrameCounter = 0;
                state->LastFPSUpdateTime = state->CurrentTime;
            }
        }

    }

    take_screenshot(state);
    wfree_state(state);
    //wfree_texture(state->Screen);
    //wfree(state->Camera);
    //ruby_cleanup(0);
    message_log("Allocated-", mem_info.MemoryAllocated);
    message_log("Freed-", mem_info.MemoryFreed);
    message_log("Leaked-", mem_info.MemoryAllocated - mem_info.MemoryFreed);
    SDL_Quit();
    return 0;
}
