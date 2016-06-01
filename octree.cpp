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

inline void initialize_octree_node_children(Octree_Node* node) {
    node->children = (Octree_Node*)walloc(sizeof(Octree_Node)*8);
    GLfloat child_radius = node->radius/2.0f;
    node->filled_children = 0x00;
    uint8_t child_depth = node->depth + 1;

    for(int i = 0; i < 8; i++) {
        node->children[i].parent = node;
        node->children[i].depth = child_depth;
        node->children[i].children = NULL;
        node->children[i].filled_children = 0x00;
        node->children[i].radius = child_radius;
        node->children[i].position = calculate_child_position(node, i);
    }
}

Octree_Node* find_specific_node(Octree* octree, glm::vec3 position) {
    //TODO: write a hash function to make these lookups not traverse the tree
    Octree_Node* node = &octree->root;
    while(node->position != position) {
        node = child_from_position(node, position);
    }
    return node;
}

Octree_Node* find_appropriate_node(Octree* octree, glm::vec3 position,
    GLfloat radius) {
    Octree_Node* node = &octree->root;
    GLfloat node_radius = node->radius;

    while(node_radius > radius) {
        if(node->children == NULL) { initialize_octree_node_children(node); }
        node = child_from_position(node, position);
        node_radius = node->radius;
    }
    return node->parent;
}

void put_bounding_box_in_octree(Octree* octree, Object* object) {
    Octree_Node model_node;
    glm::vec3 model_size = object->model->bounding_maximum -
        object->model->bounding_minimum;
    model_size *= object->model->local_scale;
    model_size *= object->physics->scale;
    model_node.radius = vector_max_component(model_size)/2.0f;
    model_node.position = object->physics->position;

    glm::vec3 model_node_child_position;
    Octree_Node* real_node;
    for(int i = 0; i < 8; i++) {
        model_node_child_position = calculate_child_position(&model_node, i);
        real_node = find_appropriate_node(octree, model_node_child_position, model_node.radius);
        real_node->filled_children = 0xFF;
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
    GLfloat radius;
    Octree_Node* best_node;
    glm::vec3 a; glm::vec3 b; glm::vec3 c;
    for(GLuint i = 0; i < model->face_count; i++) {
        a = model->faces[i].a.v;
        b = model->faces[i].b.v;
        c = model->faces[i].c.v;
        a = glm::vec3(model_matrix * glm::vec4(a, 1.0f));
        b = glm::vec3(model_matrix * glm::vec4(b, 1.0f));
        c = glm::vec3(model_matrix * glm::vec4(c, 1.0f));
        minimum = a; maximum = a;
        face_center = (a * 0.333f) + (b * 0.333f) + (c * 0.333f);
        vector_set_if_lower(&a, &minimum);
        vector_set_if_lower(&b, &minimum);
        vector_set_if_lower(&c, &minimum);
        vector_set_if_higher(&a, &maximum);
        vector_set_if_higher(&b, &maximum);
        vector_set_if_higher(&c, &maximum);
        radius = vector_max_component(maximum - minimum)/2.0f;
        best_node = find_appropriate_node(octree, face_center, radius);
        best_node->filled_children = 0xFF;
    }
}

Octree initialize_octree(uint32_t max_depth, GLfloat root_radius) {
    Octree octree;
    octree.max_depth = max_depth;
    octree.root.depth = 0;
    octree.root.radius = root_radius;
    octree.root.position = glm::vec3(0.0f, 0.0f, 0.0f);
    octree.root.filled_children = 0x00;

    initialize_octree_node_children(&octree.root);

    return octree;
}

void octree_from_object(Octree* octree, Object* object) {
    //Find the size of our root node
    Model* model = object->model;
    glm::vec3 model_radius = (model->bounding_maximum - model->bounding_minimum)/2.0f;
    GLfloat largest_radius = ceilf(vector_max_component(model_radius));

    *octree = initialize_octree(32, largest_radius);

    //TODO: add automatic loading of object back in when performant
    return;
}

void octree_debug_draw_node(Octree_Node* node, Scene_Camera* camera,
    Object* marker) {
    glm::vec4 leaf_color;
    glm::vec3 leaf_pos;

#if 0
    //DRAW EVERY NODE
    DEBUG_LOG(node->position);
    leaf_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    gl_fast_draw_vao(camera, marker, node->position,
        leaf_color, node->radius*2.0f);
    leaf_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    gl_fast_draw_vao(camera, marker, node->position,
        leaf_color, 0.1f);
#endif

#if 1
    //DRAW ONLY NODES WITH CHILDREN
    if(node->filled_children) {
        leaf_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        gl_fast_draw_vao(camera, marker, node->parent->position,
            leaf_color, node->parent->parent->radius);
        leaf_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        gl_fast_draw_vao(camera, marker, node->position,
            leaf_color, node->parent->radius);
    }
#endif

    if(node->children != NULL) {
        for(int i = 0; i < 8; i++) {
            octree_debug_draw_node(&node->children[i], camera, marker);
        }
    }
}

void octree_debug_draw(Octree* octree, State* state) {
    gl_toggle_wireframe(true);
    glUseProgram(state->Debug_Cube->shader->id);
    glBindVertexArray(state->Debug_Cube->vao);
    octree_debug_draw_node(&octree->root, state->Camera, state->Debug_Cube);
    gl_toggle_wireframe(false);
}

