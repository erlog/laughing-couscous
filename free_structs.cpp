//TODO: work out if asset paths should actually be const or not
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
}

void wfree_state(State* object) {
    wfree_texture(object->Screen);
    for(int i = 0; i < object->ObjectCount; i++) {
        wfree_object(&object->Objects[i]);
    }
    wfree(object->Objects);
    wfree(object->Camera);
    return;
}
