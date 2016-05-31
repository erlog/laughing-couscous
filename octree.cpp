inline glm::vec3 get_child_position(Octree* octree, Octree_Node* node,
    uint8_t child_index) {
    return node->position + (octree->position_table[child_index] * node->radius);
}

inline void initialize_octree_node_children(Octree* octree, Octree_Node* node) {
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
        node->children[i].position = get_child_position(octree, node, i);
    }
}

Octree initialize_octree(uint32_t max_depth, GLfloat root_radius) {
    Octree octree;
    octree.max_depth = max_depth;
    octree.root.depth = 0;
    octree.root.radius = root_radius;
    octree.position_table = (glm::vec3*)walloc(sizeof(glm::vec3)*8);
    octree.position_table[0] = glm::vec3(-0.5f, -0.5f, -0.5f);
    octree.position_table[1] = glm::vec3( 0.5f, -0.5f, -0.5f);
    octree.position_table[2] = glm::vec3(-0.5f,  0.5f, -0.5f);
    octree.position_table[3] = glm::vec3( 0.5f,  0.5f, -0.5f);
    octree.position_table[4] = glm::vec3(-0.5f, -0.5f,  0.5f);
    octree.position_table[5] = glm::vec3( 0.5f, -0.5f,  0.5f);
    octree.position_table[6] = glm::vec3(-0.5f,  0.5f,  0.5f);
    octree.position_table[7] = glm::vec3( 0.5f,  0.5f,  0.5f);

    octree.root.position = glm::vec3(0.0f, 0.0f, 0.0f);
    octree.root.filled_children = 0x00;

    initialize_octree_node_children(&octree, &octree.root);

    return octree;
}

inline Octree_Node* child_from_position(Octree* octree, Octree_Node* node,
    glm::vec3 position) {

    if(node->children == NULL) {
        initialize_octree_node_children(octree, node);
    }

    Octree_Node* closest = &node->children[0];
    GLfloat closest_distance = distance_squared(position, node->children[0].position);
    GLfloat distance;
    for(int i = 1; i < 8; i++) {
        distance = distance_squared(position, node->children[i].position);
        if(distance < closest_distance) {
            closest_distance = distance;
            closest = &node->children[i];
        }
    }
    return closest;
}

Octree_Node* find_specific_node(Octree* octree, glm::vec3 position) {
    Octree_Node* node = &octree->root;
    while(node->position != position) {
        node = child_from_position(octree, node, position);
    }
    return node;
}

Octree_Node* find_appropriate_node(Octree* octree, glm::vec3 position,
    GLfloat radius) {
    Octree_Node* node = &octree->root;
    GLfloat node_radius = node->radius;
    while(node_radius > radius) {
        node = child_from_position(octree, node, position);
        node_radius = node->radius;
    }

    return node->parent;
}

GLfloat quantize_radius(GLfloat root_radius, GLfloat radius) {
    GLfloat result = root_radius;
    while(result > radius) { result /= 2.0f; }
    result *= 2.0f;
    return result;
}

void put_bounding_box_in_octree(Octree* octree, Object* object) {

    //anything
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
        model_node_child_position = get_child_position(octree, &model_node, i);
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

void generate_octree(Octree* octree, Object* object) {
    //Find the size of our root node
    Model* model = object->model;
    glm::vec3 model_radius = (model->bounding_maximum - model->bounding_minimum)/2.0f;
    GLfloat largest_radius = ceilf(vector_max_component(model_radius));

    *octree = initialize_octree(32, largest_radius);
    for(int i = 0; i < 8; i++) {
        octree->root.children[i].children = NULL;
        octree->root.children[i].filled_children = 0;
        octree->root.children[i].parent = &octree->root;
        octree->root.children[i].radius = octree->root.radius/2.0f;
        octree->root.children[i].position = octree->root.position +
            (octree->position_table[i] * octree->root.radius);
    }
    put_object_in_octree(octree, object);
}

void octree_debug_draw(Octree* octree, Octree_Node* node, Scene_Camera* camera,
    Object* marker) {
    glm::vec4 leaf_color;
    glm::vec3 leaf_pos;

    if(node->filled_children) {
        //if not subdivided then draw filled pieces of octree
        leaf_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        gl_fast_draw_vao(camera, marker, node->parent->position,
            leaf_color, node->parent->parent->radius);
        leaf_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        gl_fast_draw_vao(camera, marker, node->position,
            leaf_color, node->parent->radius);
    }
    if(node->children != NULL) {
        //if subdivided then check each of the children
        for(int i = 0; i < 8; i++) {
            octree_debug_draw(octree, &node->children[i], camera, marker);
        }
    }
}
