//C Standard Library
#include <time.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
//C++ Stuff

//Other Libraries
#include <GL/glew.h>
#include <GL/glext.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
//#include <SDL_opengl_glext.h>
#include <ruby.h>
#include "lodepng.hpp"
//Namespaces
using namespace std;
//Local Headers
#include "renderer.h"
//Local Includes
#include "logging.cpp"
#include "gl_functions.cpp"
#include "wavefront.cpp"
#include "utilities.cpp"
#include "hid_input.cpp"
//#include "ruby_functions.cpp"

uint32_t current_time() {
    return SDL_GetTicks();
}

int main() {
    //INITIALIZATION- Failures here cause a hard exit
    State state = {0};
    state.IsRunning = true;

    //Initialize screen struct and buffer for taking screenshots
    Texture screen; screen.asset_path = "Flamerokz";
    screen.width = 384; screen.height = screen.width; screen.bytes_per_pixel = 3;
    screen.pitch = screen.width * screen.bytes_per_pixel;
    screen.buffer_size = screen.pitch * screen.height;
    screen.buffer = (uint8_t*)malloc(screen.buffer_size);
    state.screen = &screen;

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
    SDL_Window* window = SDL_CreateWindow( screen.asset_path, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, screen.width, screen.height, SDL_WINDOW_OPENGL);
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
    glClearColor( 0.f, 0.f, 0.f, 1.f );
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CCW); //Default is CCW, counter-clockwise
    //glDepthRange(1.0, -1.0); //change the handedness of the z axis
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //GAME INIT- Failures here may cause a proper smooth exit when necessary

    //Construct Camera
    state.Camera = (Scene_Camera*)malloc(sizeof(Scene_Camera));
    state.Camera->position = glm::vec3(0.f, 0.f, 3.f);
    state.Camera->orientation = glm::vec3(0.f, 1.f, 0.f);
    state.Camera->yaw = 270.f;
    state.Camera->pitch = 0.f;
    gl_recompute_camera_vector(state.Camera);

    //Construct Camera Matrices
    glm::mat4 projection_matrix;
    projection_matrix = glm::perspective(glm::radians(85.f),
        (float)screen.width/screen.height, 0.1f, 100.f);
    state.Camera->projection = projection_matrix;


    //Load Objects
    state.ObjectCount = 3;
    state.Objects = (Object*)malloc(sizeof(Object)*3);
    load_object(&state.Objects[0], "cube.obj", "blank.png", "blank_nm.png",
        "blank_spec.png", "flat.vert", "flat.frag");
    state.Objects[0].model->position = glm::vec3(0.f, 0.f, 0.f);
    state.Objects[0].model->rotation = glm::vec3(1.f, 0.f, 0.f);
    load_object(&state.Objects[1], "wt_teapot.obj", "blank.png", "blank_nm.png",
        "blank_spec.png", "flat.vert", "flat.frag");
    state.Objects[1].model->position = glm::vec3(1.5f, 0.f, 0.f);
    state.Objects[1].model->rotation = glm::vec3(0.f, 1.f, 0.f);
    load_object(&state.Objects[2], "african_head.obj", "african_head.png",
        "african_head_nm.png", "african_head_spec.png", "shader.vert",
        "shader.frag");
    state.Objects[2].model->position = glm::vec3(-1.5f, 0.f, 0.f);
    state.Objects[2].model->rotation = glm::vec3(0.f, 0.f, 1.f);

    Object* object;

    state.StartTime = current_time();
    state.LastFPSUpdateTime = state.StartTime;

    //MAIN LOOP- Failures here may cause a proper smooth exit when necessary
    message_log("Starting update loop.", "");

    while(state.IsRunning) {
        state.CurrentTime = current_time();
        state.DeltaTime = state.CurrentTime - state.LastUpdateTime;

        while(SDL_PollEvent(&event)) { switch(event.type) {
            case SDL_WINDOWEVENT:
                break;

            case SDL_KEYDOWN:
                handle_keyboard(&state, event.key);
                break;

            case SDL_QUIT:
                state.IsRunning = false;
                break;
        } }

        //TODO: is there a better way to control our framerate?
        if( state.DeltaTime > 32 ) {
            //rb_funcall(rb_cObject, rb_update_func, 0, NULL);
            //draw stuff
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for(int i = 0; i < state.ObjectCount; i++) {
                gl_draw_object(state.Camera, &state.Objects[i]);
            }

            gl_draw_debug_grid_lines();

            SDL_GL_SwapWindow(window);
            state.LastUpdateTime = state.CurrentTime;
            state.FrameCounter += 1;

            //Spit out FPS info
            if(state.FrameCounter > 100) {
                float fps = (float)state.FrameCounter;

                fps /= (state.CurrentTime - state.LastFPSUpdateTime);


                message_log("FPS-", fps*1000);
                state.FrameCounter = 0;
                state.LastFPSUpdateTime = state.CurrentTime;
            }
        }

    }

    take_screenshot(&state);
    SDL_Quit();
    //ruby_cleanup(0);
    return 0;
}
