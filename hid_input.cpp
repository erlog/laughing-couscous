
void process_input(State* state) {
    Game_Input* input = state->Input;
    //TODO: make velocity part of the Player or abstract it some other way
    GLfloat velocity_factor = 9.50f;
    if(input->relative_movement) {
        //read the input values into a vector
        glm::vec3 input_movement_vector =
            glm::vec3(input->relative_horizontal_movement, 0.0f,
                -1.0f * input->relative_vertical_movement);

        //divide by the number of polling events to normalize
        input_movement_vector /= input->times_polled;

        //do a weighted average of the new movement vector with the old one
        //by multiplying both by their velocity then adding them
        input_movement_vector *= velocity_factor;
        input_movement_vector += (state->Player->physics->movement_vector *
            state->Player->physics->velocity);
        normalize(&input_movement_vector);

        //set the new movement vector and velocity
        //TODO: make velocity analog based on times_polled info, etc.
        state->Player->physics->velocity = velocity_factor;
        state->Player->physics->movement_vector = input_movement_vector;
    }

    if(input->relative_camera_movement) {
        Physics_Object* physics = state->Camera->physics;
        glm::quat rotation_x = glm::quat(
            glm::vec3(0.f, glm::radians(input->relative_camera_x*0.25f), 0.f));
        glm::quat rotation_y = glm::quat(
            glm::vec3(glm::radians(input->relative_camera_y*0.25f), 0.f, 0.f));
        //this kills rolling on Z
        physics->quaternion = rotation_y * physics->quaternion * rotation_x;
    }
    return;
}




void poll_input(State* state, const uint8_t* keystate) {
    Game_Input* input = state->Input;
    state->Input->times_polled += 1;

    if(keystate[SDL_SCANCODE_TAB] & keystate[SDL_SCANCODE_LALT]) {
        //TODO: figure out why this gets eaten
        message_log("ALT-TAB");
    }
    if(keystate[SDL_SCANCODE_W]) {
        input->relative_movement = true;
        input->relative_vertical_movement += 1.0f;
    }
    if(keystate[SDL_SCANCODE_S]) {
        input->relative_movement = true;
        input->relative_vertical_movement -= 1.0f;
    }
    if(keystate[SDL_SCANCODE_A]) {
        input->relative_movement = true;
        input->relative_horizontal_movement -= 1.0f;
    }
    if(keystate[SDL_SCANCODE_D]) {
        input->relative_movement = true;
        input->relative_horizontal_movement += 1.0f;
    }

    int x=0; int y=0;
    uint32_t bitmask = SDL_GetRelativeMouseState(&x, &y);

    if( (x != 0) | (y != 0) ) {
        input->relative_camera_movement = true;
        input->relative_camera_x += x;
        input->relative_camera_y += y;
    }

    return;
}

#if 0
#endif

void handle_keyboard(State* state, SDL_Event event) {
    SDL_KeyboardEvent key = event.key;

    switch(key.keysym.sym) {
        case SDLK_ESCAPE:
            state->IsRunning = false;
            break;
        case SDLK_SPACE:
            message_log("Player Position", state->Player->physics->position);
            break;
        case SDLK_TAB:
            message_log("TAB");
            break;
        case SDLK_p:
            toggle_pause(state);
            break;
        case SDLK_F11:
            reload_shaders(state);
            break;
        case SDLK_F12:
            take_screenshot(state);
            break;
    }
}
