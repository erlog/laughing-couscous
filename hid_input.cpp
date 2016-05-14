void handle_keyboard(SDL_KeyboardEvent key) {
    switch(key.keysym.sym) {
        case SDLK_ESCAPE:
            State.IsRunning = false;
            break;
        case SDLK_F12:
            take_screenshot();
            break;
    }
}
