//Strings
void message_log(const char* message) {
    printf("%i: %s\n", SDL_GetTicks(), message);
}
void message_log(const char* message, const char* predicate) {
    printf("%i: %s %s\n", SDL_GetTicks(), message, predicate);
}

//Vectors, etc.
void message_log(const char* message, glm::vec3 vec) {
    printf("%i: %s (%f, %f, %f)\n",
        SDL_GetTicks(), message, vec.x, vec.y, vec.z);
}

void message_log(const char* message, glm::vec4 vec) {
    printf("%i: %s (%f, %f, %f, %f)\n",
        SDL_GetTicks(), message, vec.x, vec.y, vec.z, vec.w);
}

void message_log(const char* message, glm::quat vec) {
    printf("%i: %s (%f, %f, %f, %f)\n",
        SDL_GetTicks(), message, vec.x, vec.y, vec.z, vec.w);
}

void message_log(const char* message, glm::mat4 mat) {
    printf("%s\n", message);
    message_log("   ", mat[0]); message_log("   ", mat[1]);
    message_log("   ", mat[2]); message_log("   ", mat[3]); puts("---");
}

void message_log(const char* message, glm::mat3 mat) {
    printf("%s\n", message);
    message_log("   ", mat[0]); message_log("   ", mat[1]);
    message_log("   ", mat[2]); puts("---");
}

//Numbers
void message_log(const char* message, float number) {
    printf("%i: %s %f\n", SDL_GetTicks(), message, number);
}

void message_log(const char* message, unsigned int number) {
    printf("%i: %s %u\n", SDL_GetTicks(), message, number);
}

void message_log(const char* message, int number) {
    printf("%i: %s %i\n", SDL_GetTicks(), message, number);
}

void message_log(const char* message, size_t number) {
    printf("%i: %s %lu Bytes\n", SDL_GetTicks(), message, number);
}

//MACROS
#define DEBUG_LOG(VARIABLE) \
do { message_log("Variable, " #VARIABLE "-", VARIABLE); } \
while(0)
