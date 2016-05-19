void wfree_texture(Texture* object) {
    wfree(object->asset_path);
    wfree(object->buffer); return;
}

void wfree_model(Model* object) {
    wfree(object->asset_path);
    wfree(object->faces); return;
}

void wfree_shader(Shader* object) {
    wfree(object->asset_path_vert);
    wfree(object->asset_path_frag);
    return;
}

void wfree_object(Object* object) {
    wfree_model(object->model);
    wfree_texture(object->texture);
    wfree_texture(object->normal_map);
    wfree_texture(object->specular_map);
    wfree_shader(object->shader);
    wfree(object->physics);
}

void wfree_camera(Scene_Camera* object) {
    wfree(object->physics);
}

void wfree_state(State* object) {
    wfree_texture(object->Screen);
    for(int i = 0; i < object->ObjectCount; i++) {
        wfree_object(&object->Objects[i]);
    }
    wfree(object->Objects);
    wfree_camera(object->Camera);
    wfree(object->Camera);
    return;
}
