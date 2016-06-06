#include "renderer.h"

void update_physics_object(Physics_Object* object, float delta_time_s) {
    object->old_position = object->position;
    //rotate
    if(object->angular_velocity > 0) {
        normalize(&object->rotation_vector);
        float amount = glm::radians(delta_time_s * object->angular_velocity);
        object->quaternion = glm::rotate(object->quaternion, amount, object->rotation_vector);
        object->angular_velocity -= object->angular_velocity * delta_time_s *
            object->deceleration_factor;
    } else {
        object->angular_velocity = 0;
    }
    //translate
    if(object->velocity > 0) {
        normalize(&object->movement_vector);
        glm::vec3 new_position = delta_time_s * object->velocity *
            object->movement_vector * object->quaternion;
        object->position += new_position;
        object->velocity -= object->velocity * delta_time_s *
            object->deceleration_factor;
    } else {
        object->velocity = 0;
    }

    //object->fall_speed += 9.81 * delta_time_s;
    //object->position.y -= object->fall_speed * delta_time_s;
    return;
}

void process_collision(State* state, Game_Level* level, Physics_Object* physics) {
    bool collided = false;

    QuadFace* face;
    glm::vec3 difference;
    GLfloat source_p;
    bool source_positive;
    GLfloat destination_p;
    bool destination_positive;
    glm::vec3 intersection_point;
    GLfloat t;
    GLfloat t_denominator;

    //TODO: re-implement this with octree
    for(unsigned int i = 0; i < level->collision_model->face_count; i++) {
        //implemented using as reference:
        //http://www.flipcode.com/archives/Basic_Collision_Detection.shtml
        face = &state->Level->collision_model->faces[i];


        //Check if plane is being crossed
        source_p = glm::dot(physics->old_position, face->normal) + face->distance;
        source_positive = (source_p >= 0);
        destination_p = glm::dot(physics->position, face->normal) + face->distance;
        destination_positive = (destination_p >= 0);

        difference = physics->position - physics->old_position;

        //Plane is crossed if signs don't match
        if(source_positive != destination_positive) {

            //Get the point of intersection
            t_denominator = glm::dot(face->normal, difference);
            if(t_denominator != 0.0f) {
                t = -1.0f * (glm::dot(face->normal, physics->old_position) +
                    face->distance) / t_denominator;
                intersection_point = physics->old_position + (difference * t);

                //check if point in quad
                if(intersection_point.x < face->minimum.x) { continue; }
                if(intersection_point.y < face->minimum.y) { continue; }
                if(intersection_point.z < face->minimum.z) { continue; }
                if(intersection_point.x > face->maximum.x) { continue; }
                if(intersection_point.y > face->maximum.y) { continue; }
                if(intersection_point.z > face->maximum.z) { continue; }


                //collided, we need to react
                //TODO: find the sliding plane
                DEBUG_LOG(i);
                DEBUG_LOG(face->center);
                DEBUG_LOG(physics->old_position);
                DEBUG_LOG(physics->position);
                DEBUG_LOG(intersection_point);
                DEBUG_LOG(physics->velocity);

                physics->movement_vector = intersection_point - physics->position;
                update_physics_object(physics, state->DeltaTimeS);
                process_collision(state, level, physics);
                return;
            }

            //Plane crossed!
            //level->last_collision = face->center;
        }

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
    state->Camera->physics->position = glm::vec3(0.f, 0.25f, 0.f);

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
    state->Objects[0].physics->position = glm::vec3(-10.5f, 0.f, 0.f);
    state->Objects[0].physics->rotation_vector = glm::vec3(1.f, 1.f, 0.f);
    state->Objects[0].light_direction = light_direction;
    state->Objects[0].model->color = rgb_to_vector(0xE3, 0x1F, 0x1F);

    load_object(&state->Objects[1], "wt_teapot", "blank", "blank_nm",
        "blank_spec", "flat_shaded");
    state->Objects[1].physics->position = glm::vec3(10.5f, 0.0f, 0.f);
    state->Objects[1].physics->rotation_vector = glm::vec3(0.f, 1.f, 0.f);
    state->Objects[1].light_direction = light_direction;
    state->Objects[1].model->color = rgb_to_vector(0xE3, 0x78, 0x1F);
    state->Objects[1].physics->scale = glm::vec3(5.0f, 5.0f, 5.0f);

    load_object(&state->Objects[2], "cone", "blank", "blank_nm",
        "blank_spec", "flat_shaded");
    state->Objects[2].physics->position = glm::vec3(-15.5f, 0.f, 0.f);
    state->Objects[2].physics->rotation_vector = glm::vec3(0.f, 0.f, 1.f);
    state->Objects[2].light_direction = light_direction;
    state->Objects[2].model->color = rgb_to_vector(0x19, 0xB5, 0x19);

    state->Level = (Game_Level*)walloc(sizeof(Game_Level));
    load_level(state->Level, "test_level");
    octree_print(&state->Level->octree->root);

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
            process_mouse(state);

            //rb_funcall(rb_cObject, rb_update_func, 0, NULL);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //Update camera
            glUseProgram(state->Debug_Cube->shader->id);
            glBindVertexArray(state->Debug_Cube->vao);
            update_physics_object(state->Camera->physics, state->DeltaTimeS);
            process_collision(state, state->Level, state->Camera->physics);
            //gl_fast_draw_vao(state->Camera, state->Debug_Cube, state->Level->last_collision,
                //glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);


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


            //draw dynamic objects
            for(int i = 0; i < state->ObjectCount; i++) {
                //state->Objects[i].physics->angular_velocity = 30.f;
                //update_physics_object(state->Objects[i].physics, state->DeltaTimeS);
                gl_draw_object(state->Camera, &state->Objects[i]);
            }

            //draw level
            gl_draw_object(state->Camera, state->Level->geometry);
            //octree_debug_draw(state->Level->octree, state);

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

    //ruby_cleanup(0);
    message_log("Allocated-", mem_info.MemoryAllocated);
    message_log("Freed-", mem_info.MemoryFreed);
    message_log("Leaked-", mem_info.MemoryAllocated - mem_info.MemoryFreed);
    SDL_Quit();
    return 0;
}
