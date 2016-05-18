void handle_keyboard(State* state, SDL_KeyboardEvent key) {
    switch(key.keysym.sym) {
        case SDLK_ESCAPE:
            state->IsRunning = false;
            break;
        case SDLK_F12:
            take_screenshot(state);
            break;
        case SDLK_UP:
            state->Camera->velocity += 0.05f;
            break;
        case SDLK_DOWN:
            state->Camera->velocity -= 0.05f;
            break;
        case SDLK_LEFT:
            state->Camera->rotational_velocity -= 0.05f;
            break;
        case SDLK_RIGHT:
            state->Camera->rotational_velocity += 0.05f;
            break;
    }
}
