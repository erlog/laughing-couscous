void handle_keyboard(SDL_KeyboardEvent key) {
    switch(key.keysym.sym) {
        case SDLK_ESCAPE:
            State.IsRunning = false;
            break;
        case SDLK_F12:
            take_screenshot();
            break;
        case SDLK_UP:
            State.Camera->position += 0.05f * State.Camera->facing;
            break;
        case SDLK_DOWN:
            State.Camera->position -= 0.05f * State.Camera->facing;
            break;
        case SDLK_LEFT:
            State.Camera->facing -= glm::vec3(0.1f, 0.0f, 0.f);
            State.Camera->facing = glm::normalize(State.Camera->facing);
            if(State.Camera->facing.x < -0.9f) {
                State.Camera->facing.x = 1.f + (1 - State.Camera->facing.x);
                State.Camera->facing.z -= -1.f;
                State.Camera->facing = glm::normalize(State.Camera->facing);
            }
            message_log("Camera facing-", State.Camera->facing);
            break;
        case SDLK_RIGHT:
            State.Camera->facing += glm::vec3(0.1f, 0.0f, 0.f);
            State.Camera->facing = glm::normalize(State.Camera->facing);
            if(State.Camera->facing.x > 0.9f) {
                State.Camera->facing.x = -1.f - (1 - State.Camera->facing.x);
                State.Camera->facing.z *= -1.f;
                State.Camera->facing = glm::normalize(State.Camera->facing);
            }
            message_log("Camera facing-", State.Camera->facing);
            break;
    }
}
