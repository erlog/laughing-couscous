//#define MAC_COMPILE 0
//#define LINUX_COMPILE 1

//C Standard Library
#include <time.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
//C++ Stuff

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
//Namespaces
using namespace std;
//Local Headers
#include "renderer.h"
//Local Includes
#include "lodepng.hpp"
#include "free_structs.cpp"
#include "logging.cpp"
#include "ini_handling.cpp"
#include "gl_functions.cpp"
#include "model_loader.cpp"
#include "utilities.cpp"
#include "hid_input.cpp"
#include "octree.cpp"
//#include "ruby_functions.cpp"

void update_physics_object(Physics_Object* object, float delta_time_s) {
    //rotate
    if(object->angular_velocity > 0) {
        normalize(&object->rotation_vector);
        float amount = glm::radians(delta_time_s * object->angular_velocity);
        object->quaternion = glm::rotate(object->quaternion, amount, object->rotation_vector);
        object->angular_velocity *= (delta_time_s * object->deceleration_factor);
    }
    //translate
    if(object->velocity > 0) {
        normalize(&object->movement_vector);
        glm::vec3 new_position = delta_time_s * object->velocity *
            object->movement_vector * object->quaternion;
        new_position.y = 0;
        object->position += new_position;
        //decelerate
        object->velocity *= (delta_time_s * object->deceleration_factor);
    }

    return;
}

int main() {
    //INITIALIZATION- Failures here cause a hard exit
    Memory_Info mem_info = {0}; Global_State = &mem_info;
    State* state = (State*)walloc(sizeof(State));
    state->IsRunning = true;
    state->IsPaused = true; //Pause will be toggled when our window gains focus
    load_settings(state);

    //DEBUG: Initialize rand()
    //TODO: remove this
    time_t t;
    srand((unsigned) time(&t));

    //Initialize screen struct and buffer for taking screenshots
    state->Screen = (Texture*)walloc(sizeof(Texture));
    state->Screen->asset_path = str_lit("Flamerokz");
    state->Screen->width = state->Settings.horizontal_resolution;
    state->Screen->height = state->Settings.vertical_resolution;
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
    //TODO: implement fullscreen
    SDL_Window* window = SDL_CreateWindow( state->Screen->asset_path,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, state->Screen->width,
        state->Screen->height, SDL_WINDOW_OPENGL);
    if(window == NULL) {
        message_log("Couldn't initialize-", "SDL OpenGL window"); return 0;
    }
    state->Window = window;
    SDL_GLContext context = SDL_GL_CreateContext( window );
    if(context == NULL) {
        message_log("Couldn't get-", "OpenGL Context for window"); return 0;
    }
    if(state->Settings.vsync) { if(SDL_GL_SetSwapInterval(1) != 0) {
        //late-swap tearing if the vsync call fails
        SDL_GL_SetSwapInterval(-1);
    } }

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
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glMatrixMode( GL_PROJECTION ); glLoadIdentity();
    glMatrixMode( GL_MODELVIEW ); glLoadIdentity();
    glClearColor( 0.f, 0.f, 0.f, 1.f );
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CCW); //Default is CCW, counter-clockwise
    //glDepthRange(1.0, -1.0); //change the handedness of the z axis
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //GAME INIT- Failures here may cause a proper smooth exit when necessary

    //Construct Camera
    state->Camera = (Scene_Camera*)walloc(sizeof(Scene_Camera));
    state->Camera->physics = (Physics_Object*)walloc(sizeof(Physics_Object));
    load_physics(state->Camera->physics);
    state->Camera->physics->position = glm::vec3(0.f, 1.4f, 3.f);

    //Construct Camera Matrices
    glm::mat4 projection_matrix;
    projection_matrix = glm::perspective(glm::radians(45.f),
        (float)state->Screen->width/state->Screen->height, 0.1f, 100.f);
    state->Camera->projection = projection_matrix;


    //Load Objects
    state->Debug_Cube = (Object*)walloc(sizeof(Object));
    load_object(state->Debug_Cube, "cube", "blank", "blank_nm",
        "blank_spec", "flat");

    glm::vec3 light_direction = glm::vec3(0.f, -1.f, -1.f);
    normalize(&light_direction);

    state->ObjectCount = 3;
    state->Objects = (Object*)walloc(sizeof(Object)*state->ObjectCount);

    load_object(&state->Objects[0], "cube", "blank", "blank_nm",
        "blank_spec", "flat");
    state->Objects[0].physics->position = glm::vec3(0.f, 1.f, 0.f);
    state->Objects[0].physics->rotation_vector = glm::vec3(1.f, 1.f, 0.f);
    state->Objects[0].light_direction = light_direction;
    state->Objects[0].model->local_scale = glm::vec3(0.5f, 0.5f, 0.5f);
    state->Objects[0].model->color = rgb_to_vector(0xE3, 0x1F, 0x1F);

    load_object(&state->Objects[1], "wt_teapot", "blank", "blank_nm",
        "blank_spec", "flat_shaded");
    state->Objects[1].model->local_position = glm::vec3(0.f, 0.5f, 0.f);
    state->Objects[1].physics->position = glm::vec3(5.0f, 0.f, 0.f);
    state->Objects[1].physics->rotation_vector = glm::vec3(0.f, 1.f, 0.f);
    state->Objects[1].light_direction = light_direction;
    state->Objects[1].model->color = rgb_to_vector(0xE3, 0x78, 0x1F);
    state->Objects[1].physics->scale = glm::vec3(5.0f, 5.0f, 5.0f);

    load_object(&state->Objects[2], "cone", "blank", "blank_nm",
        "blank_spec", "flat_shaded");
    state->Objects[2].physics->position = glm::vec3(-16.5f, -16.5f, -16.5f);
    state->Objects[2].physics->rotation_vector = glm::vec3(0.f, 0.f, 1.f);
    state->Objects[2].light_direction = light_direction;
    state->Objects[2].model->color = rgb_to_vector(0x19, 0xB5, 0x19);

    state->StaticObjectCount = 1;
    state->StaticObjects = (Object*)walloc(sizeof(Object)*state->StaticObjectCount);
    load_object(&state->StaticObjects[0], "test_level",
        "blank", "blank_nm", "blank_spec", "flat_shaded");
    state->StaticObjects[0].physics->position = glm::vec3(0.f, 0.f, 0.f);
    state->StaticObjects[0].light_direction = light_direction;
    state->StaticObjects[0].model->color = rgb_to_vector(0x13, 0x88, 0x88);

    Octree octree;
    octree_from_object(&octree, &state->StaticObjects[0]);
    put_object_in_octree(&octree, &state->Objects[1]);
#if 0
    put_bounding_box_in_octree(&octree, &state->Objects[2]);
    put_object_in_octree(&octree, &state->StaticObjects[0]);
    put_bounding_box_in_octree(&octree, &state->Objects[0]);
#endif

    Object* object;
    //MAIN LOOP- Failures here may cause a proper smooth exit when necessary
    message_log("Starting update loop.", "");

    update_time(state);
    state->TimeDifference = SDL_GetTicks();
    state->DeltaTimeMS = 33;
    state->DeltaTimeS = 33/1000.0f;

    int passed_frames = 0;
    while(state->IsRunning) {
        update_time(state);

        //Process keydown events
        while(SDL_PollEvent(&event)) { switch(event.type) {
            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_SHOWN) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    //eat first input to avoid movement jitter
                    SDL_GetRelativeMouseState(NULL, NULL);
                    toggle_pause(state);
                } else if(event.window.event == SDL_WINDOWEVENT_HIDDEN) {
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    toggle_pause(state);
                }
                break;
            case SDL_KEYDOWN:
                handle_keyboard(state, event);
                break;
            case SDL_QUIT:
                state->IsRunning = false;
                break;
        } }

        if(state->IsPaused) { continue; }

        process_keyboard(state, SDL_KeyState);
        //TODO: is there a better way to control our framerate?
        if( state->DeltaTimeMS > 30 ) {
            state->DeltaTimeMS = 30;
            process_mouse(state);

            //rb_funcall(rb_cObject, rb_update_func, 0, NULL);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //Update camera
            update_physics_object(state->Camera->physics, state->DeltaTimeS);
            //We're using quaternions for the camera which will lead
            //to the camera rolling around the Z axis. To get around this we choose
            //a point in front of the camera, rotate that point using a quaternion, and
            //then use the GLM function LookAt to generate a camera orientation in the
            //coordinate space we generally consider to be upright and facing forward
            state->Camera->direction = glm::normalize(
                glm::vec3(0.f, 0.f, -1.f) * state->Camera->physics->quaternion);
            state->Camera->view  = glm::lookAt(state->Camera->physics->position,
                state->Camera->direction + state->Camera->physics->position,
                glm::vec3(0.f, 1.f, 0.f));
            state->Camera->physics->quaternion =
                glm::quat_cast(state->Camera->view);

            //draw static objects
            for(int i = 0; i < state->StaticObjectCount; i++) {
                //gl_draw_object(state->Camera, &state->StaticObjects[i]);
            }

            //draw dynamic objects
            for(int i = 0; i < state->ObjectCount; i++) {
                //state->Objects[i].physics->angular_velocity = 30.f;
                //update_physics_object(state->Objects[i].physics, state->DeltaTimeS);
                gl_draw_object(state->Camera, &state->Objects[i]);
            }

            //draw debug octree
            octree_debug_draw(&octree, state);

            SDL_GL_SwapWindow(window);
            state->LastUpdateTime = state->GameTime;
            state->FrameCounter += 1;

            //Spit out debug info
            if(state->FrameCounter > 100) {
                float fps = (float)state->FrameCounter;
                fps /= (state->WallTime - state->LastFPSUpdateTime);

                message_log("FPS-", fps*1000);

                fps = (float)passed_frames;
                fps /= (state->WallTime - state->LastFPSUpdateTime);
                message_log("Frames passed per second-", fps*1000);

                message_log("Memory in use-", mem_info.MemoryAllocated -
                    mem_info.MemoryFreed);

                passed_frames = 0;
                state->FrameCounter = 0;
                state->LastFPSUpdateTime = state->WallTime;
            }
        }
        else { passed_frames++; }

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
