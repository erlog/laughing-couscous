//String Logging
void message_log(const char* message) {
    u_printf("%i: %s\n", SDL_GetTicks(), message);
}
void message_log(const char* message, const char* predicate) {
    u_printf("%i: %s %s\n", SDL_GetTicks(), message, predicate);
}
void message_log(UChar* message) {
    u_printf("%i: %S\n", SDL_GetTicks(), message);
}
void message_log(UChar* message, const char* predicate) {
    u_printf("%i: %S %s\n", SDL_GetTicks(), message, predicate);
}
void message_log(const char* message, UChar* predicate) {
    u_printf("%i: %s %S\n", SDL_GetTicks(), message, predicate);
}
void message_log(UChar* message, UChar* predicate) {
    u_printf("%i: %S %S\n", SDL_GetTicks(), message, predicate);
}

//NOTE: snprintf is not counting the extra characters from our format string
//so we're adding on the length of the format string manually plus one extra
//for the null terminator

//String Conversions - Vectors
UChar* to_str(glm::vec2 vec) {
    const char* format_str = "(%f, %f)";
    int32_t length = u_snprintf(NULL, 0, format_str, vec.x, vec.y);
    length += strlen(format_str) + 1;
    UChar* icu_str = (UChar*)walloc(sizeof(UChar) * length);
    u_snprintf(icu_str, length, format_str, vec.x, vec.y);
    return icu_str;
}
UChar* to_str(glm::vec3 vec) {
    const char* format_str = "(%f, %f, %f)";
    int32_t length = u_snprintf(NULL, 0, format_str, vec.x, vec.y, vec.z);
    length += strlen(format_str) + 1;
    UChar* icu_str = (UChar*)walloc(sizeof(UChar) * length);
    u_snprintf(icu_str, length, format_str, vec.x, vec.y, vec.z);
    return icu_str;
}
UChar* to_str(glm::vec4 vec) {
    const char* format_str = "(%f, %f, %f, %f)";
    int32_t length = u_snprintf(NULL, 0, format_str, vec.x, vec.y, vec.z, vec.w);
    length += strlen(format_str) + 1;
    UChar* icu_str = (UChar*)walloc(sizeof(UChar) * length);
    u_snprintf(icu_str, length, format_str, vec.x, vec.y, vec.z, vec.w);
    return icu_str;
}
UChar* to_str(glm::quat vec) {
    const char* format_str = "(%f, %f, %f, %f)";
    int32_t length = u_snprintf(NULL, 0, format_str, vec.x, vec.y, vec.z, vec.w);
    length += strlen(format_str) + 1;
    UChar* icu_str = (UChar*)walloc(sizeof(UChar) * length);
    u_snprintf(icu_str, length, format_str, vec.x, vec.y, vec.z, vec.w);
    return icu_str;
}

//String Conversions - Numbers
UChar* to_str(float number) {
    const char* format_str = "%f";
    int32_t length = u_snprintf(NULL, 0, format_str, number);
    length += strlen(format_str) + 1;
    UChar* icu_str = (UChar*)walloc(sizeof(UChar) * length);
    u_snprintf(icu_str, length, format_str, number); return icu_str;
}
UChar* to_str(unsigned int number) {
    const char* format_str = "%u";
    int32_t length = u_snprintf(NULL, 0, format_str, number);
    length += strlen(format_str) + 1;
    UChar* icu_str = (UChar*)walloc(sizeof(UChar) * length);
    u_snprintf(icu_str, length, format_str, number); return icu_str;
}
UChar* to_str(long int number) {
    const char* format_str = "%ld";
    int32_t length = u_snprintf(NULL, 0, format_str, number);
    length += strlen(format_str) + 1;
    UChar* icu_str = (UChar*)walloc(sizeof(UChar) * length);
    u_snprintf(icu_str, length, format_str, number); return icu_str;
}
UChar* to_str(int number) {
    const char* format_str = "%i";
    int32_t length = u_snprintf(NULL, 0, format_str, number);
    length += strlen(format_str) + 1;
    UChar* icu_str = (UChar*)walloc(sizeof(UChar) * length);
    u_snprintf(icu_str, length, format_str, number); return icu_str;
}
UChar* to_str(size_t number) {
    const char* format_str = "%lu Bytes";
    int32_t length = u_snprintf(NULL, 0, format_str, number);
    length += strlen(format_str) + 1;
    UChar* icu_str = (UChar*)walloc(sizeof(UChar) * length);
    u_snprintf(icu_str, length, format_str, number); return icu_str;
}

//Vector Logging
void message_log(const char* message, glm::vec2 vec) {
    UChar* icu_str = to_str(vec); message_log(message, icu_str); wfree(icu_str); }

void message_log(const char* message, glm::vec3 vec) {
    UChar* icu_str = to_str(vec); message_log(message, icu_str); wfree(icu_str); }

void message_log(const char* message, glm::vec4 vec) {
    UChar* icu_str = to_str(vec); message_log(message, icu_str); wfree(icu_str); }

void message_log(const char* message, glm::quat quat) {
    UChar* icu_str = to_str(quat); message_log(message, icu_str); wfree(icu_str); }

//Number Logging
void message_log(const char* message, float number) {
    UChar* icu_str = to_str(number); message_log(message, icu_str); wfree(icu_str); }

void message_log(const char* message, unsigned int number) {
    UChar* icu_str = to_str(number); message_log(message, icu_str); wfree(icu_str); }

void message_log(const char* message, long int number) {
    UChar* icu_str = to_str(number); message_log(message, icu_str); wfree(icu_str); }

void message_log(const char* message, int number) {
    UChar* icu_str = to_str(number); message_log(message, icu_str); wfree(icu_str); }

void message_log(const char* message, size_t number) {
    UChar* icu_str = to_str(number); message_log(message, icu_str); wfree(icu_str); }

//MACROS
#define DEBUG_LOG(VARIABLE) \
do { message_log("Variable, " #VARIABLE "-", VARIABLE); } \
while(0)
