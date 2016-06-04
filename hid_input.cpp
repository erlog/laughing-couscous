void process_keyboard(State* state, const uint8_t* keystate) {
    if(keystate[SDL_SCANCODE_TAB] & keystate[SDL_SCANCODE_LALT]) {
        //TODO: figure out why this gets eaten
        message_log("ALT-TAB");
    }
    if(keystate[SDL_SCANCODE_W]) {
        state->Camera->physics->movement_vector.z = -1.f;
        state->Camera->physics->velocity = 9.50f;
    }
    else if(keystate[SDL_SCANCODE_S]) {
        state->Camera->physics->movement_vector.z = 1.f;
        state->Camera->physics->velocity = 9.50f;
    }

    if(keystate[SDL_SCANCODE_A]) {
        state->Camera->physics->movement_vector.x = -1.f;
        state->Camera->physics->velocity = 9.50f;
    }
    else if(keystate[SDL_SCANCODE_D]) {
        state->Camera->physics->movement_vector.x = 1.f;
        state->Camera->physics->velocity = 9.50f;
    }

    if(keystate[SDL_SCANCODE_SPACE]) {
        message_log("Player Position", state->Camera->physics->position);
    }
}

void process_mouse(State* state) {
    //TODO: configurable mouse sensitivity
    int x=0; int y=0;
    uint32_t bitmask = SDL_GetRelativeMouseState(&x, &y);

    if( (x == 0) & (y == 0) ) { return; }

    Physics_Object* physics = state->Camera->physics;

    glm::quat rotation_x = glm::quat(
        glm::vec3(0.f, glm::radians(x*0.25f), 0.f));
    glm::quat rotation_y = glm::quat(
        glm::vec3(glm::radians(y*0.25f), 0.f, 0.f));
    physics->quaternion = rotation_y * physics->quaternion * rotation_x;
}

void handle_keyboard(State* state, SDL_Event event) {
    SDL_KeyboardEvent key = event.key;

    switch(key.keysym.sym) {
        case SDLK_ESCAPE:
            state->IsRunning = false;
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
