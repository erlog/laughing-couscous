void process_keyboard(State* state, const uint8_t* keystate) {
    if(keystate[SDL_SCANCODE_W]) {
        state->Camera->physics->velocity = 4.50f;
    }
    else if(keystate[SDL_SCANCODE_S]) {
        state->Camera->physics->velocity = -4.50f;
    }

    if(keystate[SDL_SCANCODE_A]) {
        state->Camera->physics->angular_velocity = -75.0f;
    }
    else if(keystate[SDL_SCANCODE_D]) {
        state->Camera->physics->angular_velocity = 75.0f;
    }
}

void handle_keyboard(State* state, SDL_KeyboardEvent key) {
    switch(key.keysym.sym) {
        case SDLK_ESCAPE:
            state->IsRunning = false;
            break;
        case SDLK_F12:
            take_screenshot(state);
            break;
    }
}
