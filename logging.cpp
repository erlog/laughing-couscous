//Strings
void message_log(const char* message, const char* predicate) {
    printf("%i: %s %s\n", SDL_GetTicks(), message, predicate);
}

//Vectors
void message_log(const char* message, glm::vec3 vec) {
    printf("%i: %s (%f, %f, %f)\n",
        SDL_GetTicks(), message, vec.x, vec.y, vec.z);
}

void message_log(const char* message, glm::vec4 vec) {
    printf("%i: %s (%f, %f, %f, %f)\n",
        SDL_GetTicks(), message, vec.x, vec.y, vec.z, vec.w);
}

//Numbers
void message_log(const char* message, float number) {
    printf("%i: %s %f\n", SDL_GetTicks(), message, number);
}

void message_log(const char* message, int number) {
    printf("%i: %s %i\n", SDL_GetTicks(), message, number);
}

void message_log(const char* message, size_t number) {
    printf("%i: %s %.2f MB\n", SDL_GetTicks(), message, number/(1024.f*1024.f));
}
