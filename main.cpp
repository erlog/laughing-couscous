//Compile-time Flags
    //#define MAC_COMPILE 0
    //#define LINUX_COMPILE 1
    #define MEMORY_LOGGING 0
    #define DEBUG 1

#include "main.h"

int main() {
    //INITIALIZATION- Failures here cause a hard exit
    //Start Ruby
    ruby_setup_environment(); //VALUE rb_update_func = rb_intern("ruby_update");

    //Initialize State & Debug Memory Manager
    Memory_Info mem_info = {0}; Global_State = &mem_info;
    State* state = (State*)walloc(sizeof(State));

    //Load global strings
    String_DB _string_db;
    state->Strings = &_string_db;
    load_strings(state->Strings, "english");
    wfree_string_db(state->Strings);

    //Initialize game input
    state->Input = (Game_Input*)walloc(sizeof(Game_Input));
    clear_input(state->Input);

    //Load global settings file
    load_settings(state);

    state->IsRunning = true; //Set this to false to exit after one frame
    state->IsPaused = true; //Pause will be toggled when our window gains focus

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
    glPixelStorei(GL_PACK_ALIGNMENT, 1); glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glMatrixMode( GL_PROJECTION ); glLoadIdentity();
    glMatrixMode( GL_MODELVIEW ); glLoadIdentity();
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND); glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CCW); //Default is CCW, counter-clockwise
    //glDepthRange(1.0, -1.0); //change the handedness of the z axis
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //GAME INIT- Failures here may cause a proper smooth exit when necessary

    //Construct Camera
    state->Camera = (Scene_Camera*)walloc(sizeof(Scene_Camera));
    state->Camera->physics = (Physics_Object*)walloc(sizeof(Physics_Object));
    load_physics(state->Camera->physics);
    state->Camera->projection = glm::perspective(glm::radians(45.0f),
        (float)state->Screen->width/state->Screen->height, 0.1f, 100.0f);

    //Construct screen-space camera for HUD elements
    Scene_Camera screen_camera;
    screen_camera.physics = (Physics_Object*)walloc(sizeof(Physics_Object));
    load_physics(screen_camera.physics);
    //TODO: figure out how to make this in semi-normalized coords that
    //can be somewhat resolution-independent
    screen_camera.projection = glm::ortho(0.0f, (float)state->Screen->width,
        0.0f, (float)state->Screen->height);

    //Load Objects
    state->Debug_Cube = (Object*)walloc(sizeof(Object));
    load_object(state->Debug_Cube, "cube", "blank", "blank_nm",
        "blank_spec", "shader");

    state->Debug_Sphere = (Object*)walloc(sizeof(Object));
    load_object(state->Debug_Sphere, "sphere", "blank", "blank_nm",
        "blank_spec", "flat_shaded");

    state->Player = (Object*)walloc(sizeof(Object));
    load_object(state->Player, "wedge", "cheese", "blank_nm_512",
        "cheese_spec", "shader");
    state->Player->model->color = rgb_to_vector(0xE7, 0xE0, 0x8B);
    state->Player->physics->position = glm::vec3(0.0f, -1.0f, 0.0f);

    glm::vec3 light_direction = glm::vec3(0.0f, -1.0f, -1.0f);
    normalize(&light_direction);

    //TODO: break this out into level loading code
    state->ObjectCount = 3;
    state->Objects = (Object*)walloc(sizeof(Object)*state->ObjectCount);

    load_object(&state->Objects[0], "cube", "blank", "blank_nm",
        "blank_spec", "flat");
    state->Objects[0].physics->position = glm::vec3(-10.5f, 0.0f, 0.0f);
    state->Objects[0].physics->rotation_vector = glm::vec3(1.0f, 1.0f, 0.0f);
    state->Objects[0].light_direction = light_direction;
    state->Objects[0].model->color = rgb_to_vector(0xE3, 0x1F, 0x1F);

    load_object(&state->Objects[1], "african_head", "african_head", "african_head",
        "african_head", "flat_shaded");
    state->Objects[1].physics->position = glm::vec3(10.5f, 0.0f, 0.0f);
    state->Objects[1].physics->rotation_vector = glm::vec3(0.0f, 1.0f, 0.0f);
    state->Objects[1].light_direction = light_direction;
    state->Objects[1].model->color = rgb_to_vector(0xE3, 0x78, 0x1F);
    state->Objects[1].model->scale = glm::vec3(5.0f, 5.0f, 5.0f);

    load_object(&state->Objects[2], "cone", "blank", "blank_nm",
        "blank_spec", "flat_shaded");
    state->Objects[2].physics->position = glm::vec3(-15.5f, 0.0f, 0.0f);
    state->Objects[2].physics->rotation_vector = glm::vec3(0.0f, 0.0f, 1.0f);
    state->Objects[2].light_direction = light_direction;
    state->Objects[2].model->color = rgb_to_vector(0x19, 0xB5, 0x19);

    state->Level = (Game_Level*)walloc(sizeof(Game_Level));
    load_level(state->Level, "test_level");
    //octree_print(&state->Level->octree->root);

    Font test_font;
    UChar* test_text = UChar_convert("#testing, unicode, 微研");
    load_font(&test_font, "DroidSans");

    //MAIN LOOP- Failures here may cause a proper smooth exit when necessary
    message_log("Starting update loop.");

    //TODO: figure out how to start my clock without eating the first frame
    state->TimeDifference = 0;
    state->LastUpdateTime = 0;
    state->PauseStartTime = 0;

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

        poll_input(state, SDL_KeyState);

        //TODO: is there a better way to control our framerate?
        if( state->DeltaTimeMS > 30 ) {
            process_input(state);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //TODO: unified camera system
            //first person camera

            #if 0
            state->Camera->direction = glm::normalize(
                glm::vec3(0.0f, 0.0f, -1.0f) * state->Camera->physics->quaternion);
            state->Camera->view  = glm::lookAt(state->Camera->physics->position,
                state->Camera->direction + state->Camera->physics->position,
                glm::vec3(0.0f, 1.0f, 0.0f));
            state->Camera->physics->quaternion =
                glm::quat_cast(state->Camera->view);
            #endif

            //3rd-person camera
            #if 1
            state->Camera->physics->position = state->Player->physics->position +
                (glm::vec3(0.0f, 6.0f, 6.0f) * state->Camera->physics->quaternion);
            state->Camera->view = glm::lookAt(state->Camera->physics->position,
                 state->Player->physics->position,
                glm::vec3(0.0f, 1.0f, 0.0f));
            #endif

            //draw dynamic objects
            for(int i = 0; i < state->ObjectCount; i++) {
                //state->Objects[i].physics->angular_velocity = 30.0f;
                //update_physics_object(state->Objects[i].physics, state->DeltaTimeS);
                gl_draw_object(state->Camera, &state->Objects[i]);
            }

            //draw level
            gl_draw_object(state->Camera, state->Level->geometry);
            //octree_debug_draw(state->Level->octree, state);


            //do player movement relative to the camera
            state->Player->physics->quaternion = state->Camera->physics->quaternion;
            state->Player->physics->time_remaining = state->DeltaTimeS;
            physics_process_movement(state->Player->physics);
            //TODO: make our collision detection not require this sanity check
            if(state->Player->physics->moved) {
                for(int reps = 0; reps < 25; reps ++) {
                    if(!process_collision(state->Level, state->Player->physics)) {
                        break;
                    }
                }
            }
            //face model in direction of movement
            physics_face_movement_direction(state->Player);
            gl_draw_object(state->Camera, state->Player);

            //draw bounding box
            #if 1
            state->Debug_Sphere->physics->position = state->Player->physics->position;
            state->Debug_Sphere->model->scale = state->Player->physics->radii;
            //state->Debug_Sphere->model->rotation = state->Player->model->rotation;

            gl_toggle_wireframe(true);
            gl_draw_object(state->Camera, state->Debug_Sphere);
            gl_toggle_wireframe(false);
            #endif


            //draw test text
            //TODO: make this an FPS counter
            test_font.quad->physics->position = glm::vec3(0.0f, 8.0f, 0.0f);
            gl_draw_text(&screen_camera, &test_font, test_text, 32.0f);

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
                message_log("Loops spent idling per second-", fps*1000);

                message_log("Memory in use-", mem_info.MemoryAllocated -
                    mem_info.MemoryFreed);

                passed_frames = 0;
                state->FrameCounter = 0;
                state->LastFPSUpdateTime = state->WallTime;
            }
            clear_input(state->Input);
        }
        else { passed_frames++; }

    }

    take_screenshot(state);
    wfree_font(&test_font);
    wfree_camera(&screen_camera);
    wfree(test_text);
    wfree_state(state);
    wfree(state);

    ruby_cleanup(0);
    printf("Leaked- %lu Bytes", mem_info.MemoryAllocated - mem_info.MemoryFreed);

    SDL_Quit();
    return 0;
}
