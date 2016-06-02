inline glm::vec3 calculate_child_position(Octree_Node* node,
    uint8_t child_index) {

    /** OLD POSITION TABLE
        glm::vec3(-x, -y, -z); glm::vec3(+x, -y, -z);
        glm::vec3(-x, +y, -z); glm::vec3(+x, +y, -z);
        glm::vec3(-x, -y, +z); glm::vec3(+x, -y, +z);
        glm::vec3(-x, +y, +z); glm::vec3(+x, +y, +z); **/

    glm::vec3 result = node->position;
    GLfloat step = node->hard_radius/2.0f;

    if(child_index >= 4) { result.z += step; child_index -= 4; }
    else { result.z -= step; }

    if(child_index >= 2) { result.y += step; child_index -= 2; }
    else { result.y -= step; }

    if(child_index > 0) { result.x += step; }
    else { result.x -= step; }

    return result;
}

inline Octree_Node* child_from_position(Octree_Node* node, glm::vec3 position) {
    uint8_t child_index = 0;

    if(position.z > node->position.z) { child_index += 4; }
    if(position.y > node->position.y) { child_index += 2; }
    if(position.x > node->position.x) { child_index += 1; }

    return &node->children[child_index];
}

bool test_collision(Octree_Node* node, glm::vec3 position) {
    if(node->filled) { return true; }

    if(node->children != NULL) {
        return test_collision(child_from_position(node, position), position);
    }

    return false;
}

inline void initialize_octree_node_children(Octree_Node* node) {
    if(node->children == NULL) {
        node->children = (Octree_Node*)walloc(sizeof(Octree_Node)*8);
    }
    GLfloat child_radius = node->hard_radius/2.0f;
    uint8_t child_depth = node->depth + 1;

    for(int i = 0; i < 8; i++) {
        node->children[i].parent = node;
        node->children[i].depth = child_depth;
        node->children[i].children = NULL;
        node->children[i].filled = 0;
        node->children[i].hard_radius = child_radius;
        node->children[i].soft_radius = child_radius;
        node->children[i].position = calculate_child_position(node, i);
    }
}

Octree_Node* find_specific_node(Octree* octree, glm::vec3 position) {
    Octree_Node* node = &octree->root;
    while(node->position != position) {
        if(node->children == NULL) { initialize_octree_node_children(node); }
        node = child_from_position(node, position);
    }
    return node;
}

Octree_Node* find_appropriate_node(Octree* octree, glm::vec3 position,
    GLfloat radius) {

    Octree_Node* node = &octree->root;

    if(radius >= node->hard_radius) {
        return node;
    }

    while(node->hard_radius > radius) {
        if(node->children == NULL) { initialize_octree_node_children(node); }
        node = child_from_position(node, position);
    }

    return node;
}

void put_bounding_box_in_octree(Octree* octree, Object* object) {
    glm::mat4 model_matrix = build_model_matrix(object);
    glm::vec3 minimum = mat4_multiply_vec3(model_matrix,
        object->model->bounding_minimum);
    glm::vec3 maximum = mat4_multiply_vec3(model_matrix,
        object->model->bounding_maximum);

    glm::vec3 size = absolute_difference(maximum, minimum);
    GLfloat radius = vector_max_component(size)/2.0f;

    find_appropriate_node(octree, maximum-minimum, radius)->filled += 1;

    return;
}

void octree_from_level(Level* level) {
    //Find the size of our root node
    QuadModel* model = level->collision_model;
    glm::vec3 model_size = absolute_difference(
        level->collision_model->bounding_maximum,
        level->collision_model->bounding_minimum);
    GLfloat largest_radius = ceilf(vector_max_component(model_size))/2.0f;
    DEBUG_LOG(model_size);
    DEBUG_LOG(largest_radius);

    Octree* octree = (Octree*)walloc(sizeof(Octree));
    level->octree = octree;
    octree->max_depth = 32;
    octree->root.depth = 0;
    octree->root.hard_radius = largest_radius;
    octree->root.soft_radius = largest_radius;
    octree->root.position = glm::vec3(0.0f, 0.0f, 0.0f);
    octree->root.filled = 0;
    octree->root.children = NULL;

    Face* face;
    glm::mat4 model_matrix = build_model_matrix(level->geometry);

    glm::vec3 face_center; GLfloat radius;
    glm::vec3 a; glm::vec3 b; glm::vec3 c; glm::vec3 d;

    for(GLuint i = 0; i < model->face_count; i++) {
        //Project face into world space;
        a = mat4_multiply_vec3(model_matrix, model->faces[i].a.v);
        b = mat4_multiply_vec3(model_matrix, model->faces[i].b.v);
        c = mat4_multiply_vec3(model_matrix, model->faces[i].c.v);
        d = mat4_multiply_vec3(model_matrix, model->faces[i].d.v);
        face_center = (a * 0.25f) + (b * 0.25f) + (c * 0.25f) + (d * 0.25f);

        radius = glm::distance(face_center, a);
        find_appropriate_node(octree, face_center, radius)->filled += 1;
    }
    return;
}

void octree_debug_draw_node(Octree_Node* node, Scene_Camera* camera,
    Object* marker) {

    if( node->depth == 0 ) {
        gl_fast_draw_vao(camera, marker, node->position,
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), node->soft_radius*2.0f*0.99f);
    }

    if( (node->depth > 0) & (node->filled > 0) ) {
        glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        gl_fast_draw_vao(camera, marker, node->position, color, node->hard_radius*2.0f);
    }
    if(node->children != NULL) { for(int i = 0; i < 8; i++) {
            octree_debug_draw_node(&node->children[i], camera, marker);
    } }
}

void octree_debug_draw(Octree* octree, State* state) {
    gl_toggle_wireframe(true);
    glUseProgram(state->Debug_Cube->shader->id);
    glBindVertexArray(state->Debug_Cube->vao);
    octree_debug_draw_node(&octree->root, state->Camera, state->Debug_Cube);
    gl_toggle_wireframe(false);
}

void octree_print(Octree_Node* node) {
    if(node->filled > 3) {
        message_log("Node Position-", node->position);
        message_log("  Radius", node->hard_radius);
        message_log("  Soft Radius", node->soft_radius);
        //message_log("  Depth-", node->depth);
        message_log("  Filled-", node->filled);
    }
    if(node->children != NULL) {
        for(int i = 0; i < 8; i++) { octree_print(&node->children[i]); }
    }
    return;
}

