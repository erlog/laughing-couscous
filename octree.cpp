inline glm::vec3 calculate_child_position(Octree_Node* node,
    uint8_t child_index) {

    /** OLD POSITION TABLE
        glm::vec3(-x, -y, -z); glm::vec3(+x, -y, -z);
        glm::vec3(-x, +y, -z); glm::vec3(+x, +y, -z);
        glm::vec3(-x, -y, +z); glm::vec3(+x, -y, +z);
        glm::vec3(-x, +y, +z); glm::vec3(+x, +y, +z); **/

    glm::vec3 result = node->position;
    GLfloat step = node->radius/2.0f;

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
    GLfloat child_radius = node->radius/2.0f;
    uint8_t child_depth = node->depth + 1;

    for(int i = 0; i < 8; i++) {
        node->children[i].parent = node;
        node->children[i].depth = child_depth;
        node->children[i].children = NULL;
        node->children[i].filled = 0;
        node->children[i].radius = child_radius;
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
    GLfloat node_radius = node->radius;

    if(radius >= octree->root.radius) {
        return &octree->root;
    }

    while(node_radius > radius) {
        if(node->children == NULL) { initialize_octree_node_children(node); }
        node = child_from_position(node, position);
        node_radius = node->radius;
    }
    return node->parent;
}

void put_bounding_box_in_octree(Octree* octree, Object* object) {
    Octree_Node model_node;
    model_node.children = NULL;
    model_node.depth = 0;

    glm::mat4 model_matrix = build_model_matrix(object);
    glm::vec3 minimum = mat4_multiply_vec3(model_matrix,
        object->model->bounding_minimum);
    glm::vec3 maximum = mat4_multiply_vec3(model_matrix,
        object->model->bounding_maximum);
    glm::vec3 size = absolute_difference(maximum, minimum);

    GLfloat radius = vector_max_component(size)/4.0f;
    model_node.position = object->physics->position;

    Octree_Node* real_node;
    real_node = find_appropriate_node(octree, minimum, radius);

    model_node.radius = real_node->parent->radius;
    GLfloat step = model_node.radius/2.0f;
    model_node.position = real_node->position + glm::vec3(step, step, step);
    initialize_octree_node_children(&model_node);
    for(int i = 0; i < 8; i++) {
        real_node = find_specific_node(octree, model_node.children[i].position);
        real_node->filled += 1;
    }

    return;
}

void put_object_in_octree(Octree* octree, Object* object) {
    Face* face;
    Model* model = object->model;
    glm::mat4 model_matrix = build_model_matrix(object);
    glm::vec3 minimum;
    glm::vec3 maximum;
    glm::vec3 face_center;
    glm::vec3 size;
    GLfloat volume;
    GLfloat radius;
    GLfloat diameter;
    Octree_Node* best_node;
    glm::vec3 a; glm::vec3 b; glm::vec3 c;
    Octree_Node node;
    node.children = NULL;
    node.depth = 0;

    for(GLuint i = 0; i < model->face_count; i++) {
        //Project face into world space;
        a = model->faces[i].a.v;
        b = model->faces[i].b.v;
        c = model->faces[i].c.v;
        a = mat4_multiply_vec3(model_matrix, a);
        b = mat4_multiply_vec3(model_matrix, b);
        c = mat4_multiply_vec3(model_matrix, c);
        minimum = a; maximum = a;
        //Find boundaries/center of face
        face_center = (a * 0.33333f) + (b * 0.33333f) + (c * 0.33333f);
        vector_set_if_lower(&a, &minimum);
        vector_set_if_lower(&b, &minimum);
        vector_set_if_lower(&c, &minimum);
        vector_set_if_higher(&a, &maximum);
        vector_set_if_higher(&b, &maximum);
        vector_set_if_higher(&c, &maximum);
        size = absolute_difference(maximum, minimum);
        node.position = face_center;
        node.radius = glm::distance(face_center, maximum);
        initialize_octree_node_children(&node);

        for(int i = 0; i < 8; i++) {
            best_node = find_appropriate_node(octree, node.children[i].position,
                node.children[i].radius);
            best_node->filled += 1;
        }
    }
}

Octree initialize_octree(uint32_t max_depth, GLfloat root_radius) {
    Octree octree;
    octree.max_depth = max_depth;
    octree.root.depth = 0;
    octree.root.radius = root_radius;
    octree.root.position = glm::vec3(0.0f, 0.0f, 0.0f);
    octree.root.filled = 0;
    octree.root.children = NULL;

    initialize_octree_node_children(&octree.root);

    return octree;
}

void octree_from_object(Octree* octree, Object* object) {
    //Find the size of our root node
    Model* model = object->model;
    glm::vec3 model_size = model->bounding_maximum - model->bounding_minimum;
    GLfloat largest_radius = ceilf(vector_max_component(model_size)/2.0f);

    *octree = initialize_octree(32, largest_radius);

    //TODO: add automatic loading of object back in when performant
    return;
}

void octree_debug_draw_node(Octree_Node* node, Scene_Camera* camera,
    Object* marker) {
    glm::vec4 leaf_color;
    glm::vec3 leaf_pos;

#if 0 //DRAW EVERY NODE
    gl_fast_draw_vao(camera, marker, node->position,
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), node->radius*2.0f);
#endif

    if( (node->depth > 0) & (node->filled > 0) ) {
        gl_fast_draw_vao(camera, marker, node->position,
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), node->radius*2.0f);
    }
    if(node->children != NULL) { for(int i = 0; i < 8; i++) {
        //gl_fast_draw_vao(camera, marker, node->position,
            //glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), node->radius*2.0f*0.99f);
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
    if(node->filled > 0) {
        //message_log("Node Position-", node->position);
        //message_log("  Depth-", node->depth);
        message_log("  Filled-", node->filled);
    }
    if(node->children != NULL) {
        for(int i = 0; i < 8; i++) { octree_print(&node->children[i]); }
    }
    return;
}

