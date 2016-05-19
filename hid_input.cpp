void handle_keyboard(State* state, SDL_KeyboardEvent key) {
    switch(key.keysym.sym) {
        case SDLK_ESCAPE:
            state->IsRunning = false;
            break;
        case SDLK_F12:
            take_screenshot(state);
            break;
        case SDLK_w:
            state->Camera->physics->velocity = 3.50f;
            break;
        case SDLK_s:
            state->Camera->physics->velocity = -3.50f;
            break;
        case SDLK_a:
            //state->Camera->angular_velocity -= 0.05f;
            break;
        case SDLK_d:
            //state->Camera->angular_velocity += 0.05f;
            break;
    }
}
