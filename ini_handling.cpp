
bool does_label_match(char* input, const char* label) {
    if( strcmp(label, input) == 0 ) { return true; }
    return false;
}

bool ini_line_split(char* input, char* out_a, char* out_b) {
    int length = strlen(input) + 1; //used in for loop
    int i;

    for(i = 0; i < length; i++) { if(input[i] == 0x3D) { break; } }
    if( (i == 0) | (i == length) ) { return false; }

    strncpy(out_a, input, i);
    out_a[i] = 0x00;
    strcpy(out_b, &input[i+1]);
    return true;
}

void load_settings(State* state) {
    FILE* file = fopen(SettingsINIPath, "r");

    //Initialize default settings
    state->Settings = (Settings_Object*)walloc(sizeof(Settings_Object));
    state->Settings->horizontal_resolution = 682;
    state->Settings->vertical_resolution = 384;
    state->Settings->vsync = false;
    state->Settings->fullscreen = false;

    if(file == NULL) {
        message_log("Error loading file-", SettingsINIPath);
        return;
    }

    const char* horizontal_resolution_label = "horizontal_resolution";
    const char* vertical_resolution_label = "vertical_resolution";
    const char* vsync_label = "vsync";
    const char* fullscreen_label = "fullscreen";

    //TODO: this is all going to get ripped out when I do proper string handling
    char buffer[255]; char label[255]; char value[255];
    while(fgets(buffer, 255, file)) {
       if(ini_line_split(buffer, label, value)) {
            if(does_label_match(label, horizontal_resolution_label)) {
                state->Settings->horizontal_resolution = atoi(value);
            } else if(does_label_match(label, vertical_resolution_label)) {
                state->Settings->vertical_resolution = atoi(value);
            } else if(does_label_match(label, vsync_label)) {
                state->Settings->vsync = (bool)atoi(value);
            } else if(does_label_match(label, fullscreen_label)) {
                state->Settings->fullscreen = (bool)atoi(value);
            }
        }
    }

    return;
}
