void handle_keyboard(State* state, SDL_KeyboardEvent key) {
    switch(key.keysym.sym) {
        case SDLK_ESCAPE:
            state->IsRunning = false;
            break;
        case SDLK_F12:
            take_screenshot(state);
            break;
        case SDLK_UP:
            state->Camera->position += 0.05f * state->Camera->facing;
            break;
        case SDLK_DOWN:
            state->Camera->position -= 0.05f * state->Camera->facing;
            break;
        case SDLK_LEFT:
            state->Camera->yaw -= 2.0f;
            gl_recompute_camera_vector(state->Camera);
            break;
        case SDLK_RIGHT:
            state->Camera->yaw += 2.0f;
            gl_recompute_camera_vector(state->Camera);
            break;
    }
}
