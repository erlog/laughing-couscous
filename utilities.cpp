//Small functions that don't fit anywhere else with minimal dependencies
inline void clamp(GLfloat* number, GLfloat min, GLfloat max) {
    if(*number > max) { *number = max; return; }
    if(*number < min) { *number = min; return; }
}
inline void clamp(int* number, int min, int max) {
    if(*number > max) { *number = max; return; }
    if(*number < min) { *number = min; return; }
}
inline void clamp(glm::vec3* input_vector, glm::vec3 min, glm::vec3 max) {
    clamp(&input_vector->x, min.x, max.x);
    clamp(&input_vector->y, min.y, max.y);
    clamp(&input_vector->z, min.z, max.z);
    return;
}

inline void lerp(glm::vec3* result, glm::vec3 src, glm::vec3 dest, GLfloat amt) {
    *result = src + (amt * (dest - src));
    return;
}

float debug_rand() {
    int output = rand();
    return (float)output/RAND_MAX;
}

glm::vec4 rgb_to_vector(uint8_t r, uint8_t g, uint8_t b) {
    return glm::vec4(r/255.f, g/255.f, b/255.f, 1.f);
}

inline UChar* UChar_convert(char* string) {
    //TODO: error handling
    //ask how long buffer should be
    int32_t buffer_length = 0;
    UErrorCode icu_error = U_ZERO_ERROR;
    u_strFromUTF8(NULL, 0, &buffer_length, string, -1, &icu_error);
    buffer_length += 1; //for null termination

    //convert string
    icu_error = U_ZERO_ERROR;
    UChar* icu_string = (UChar*)walloc(sizeof(UChar)*buffer_length);
    u_strFromUTF8(icu_string, buffer_length, NULL, string, -1, &icu_error);
    return icu_string;
}

inline char* str_lit(const char* string) {
    int length = strlen(string) + 1;
    char* result = (char *)walloc(sizeof(char)*length);
    strcpy(result, string);
    return result;
}

char* construct_asset_path(const char* folder, const char* filename) {
    char* format_string = "%s/%s/%s";
    //compute appropriate length
    size_t length = strlen(format_string);
    length += strlen(AssetFolderPath);
    length += strlen(folder);
    length += strlen(filename);
    length += 1;
    //build string
    char* buffer = (char*)walloc(sizeof(char)*length);
    snprintf(buffer, length, format_string, AssetFolderPath, folder,
        filename);
    return buffer;
}
char* construct_asset_path(const char* folder, const char* filename,
        const char* file_extension) {
    char* format_string = "%s/%s/%s.%s";
    //compute appropriate length
    size_t length = strlen(format_string);
    length += strlen(AssetFolderPath);
    length += strlen(folder);
    length += strlen(filename);
    length += strlen(file_extension);
    length += 1;
    //build string
    char* buffer = (char*)walloc(sizeof(char)*length);
    snprintf(buffer, length, format_string, AssetFolderPath, folder,
        filename, file_extension);
    return buffer;
}

char* get_datetime_string() {
    time_t rawtime; struct tm *info;
    time( &rawtime ); info = localtime( &rawtime );

    int output_length = 16;
    char* output = (char*)walloc(sizeof(char)*output_length);

    char* date_format = "%Y-%m-%d %H:%M:%S";
    while(strftime(output, output_length, date_format, info) == 0) {
        wfree(output);
        output_length *= 2;
        output = (char*)walloc(sizeof(char)*output_length);
    }
    return output;
}

void flip_texture(Texture* texture) {
    uint8_t* buffer = (uint8_t*)walloc(texture->buffer_size);
    int dest_i; int y; int x;
    for(int src_i = 0; src_i < texture->buffer_size; src_i++) {
        y = src_i / texture->pitch; x = src_i % texture->pitch;
        dest_i = ((texture->height - y - 1) * texture->pitch) + x;
        buffer[dest_i] = texture->buffer[src_i];
    }
    wfree(texture->buffer);
    texture->buffer = buffer;
    return;
}

void take_screenshot(State* state) {
    //construct path TODO: better way to do string nonsense?
    char* datetime_string = get_datetime_string();
    char* output_path = (char*)walloc(sizeof(char)*255);

    sprintf(output_path, "%s/renderer - %s.png", OutputFolderPath,
        datetime_string);
    message_log("Taking screenshot-", output_path);

    //write screenshot
    glReadPixels(0, 0, state->Screen->width, state->Screen->height,
        GL_RGB, GL_UNSIGNED_BYTE, state->Screen->buffer);
    flip_texture(state->Screen);
    lodepng_encode24_file(output_path, (const unsigned char*)state->Screen->buffer,
        state->Screen->width, state->Screen->height);

    wfree(datetime_string);
    wfree(output_path);
}

void update_time(State* state) {
    state->WallTime = SDL_GetTicks();
    state->GameTime = state->WallTime - state->TimeDifference;
    state->DeltaTimeMS = state->GameTime - state->LastUpdateTime;
    state->DeltaTimeS = state->DeltaTimeMS / 1000.f;
    return;
}

void toggle_pause(State* state) {
    if(state->IsPaused) {
        state->IsPaused = false;
        state->TimeDifference = SDL_GetTicks() - state->PauseStartTime;
        update_time(state);
        message_log("Unpaused-", state->GameTime);
    } else {
        state->IsPaused = true;
        state->PauseStartTime = state->GameTime;
        message_log("Paused-", state->GameTime);
    }
    return;
}
