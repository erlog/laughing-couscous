inline void initialize_octree_node_children(Octree* octree, Octree_Node* node) {
    node->children = (Octree_Node*)walloc(sizeof(Octree_Node)*8);
    for(int i = 0; i < 8; i++) {
        node->children[i].parent = node;
        node->children[i].children = NULL;
        node->children[i].filled_children = 0x00;
        node->children[i].radius = node->radius/2.0f;
        node->children[i].position = node->position +
            (octree->position_table[i] * node->radius);
    }
}

Octree initialize_octree(uint32_t max_depth, GLfloat root_radius) {
    Octree octree;
    octree.max_depth = max_depth; octree.root.radius = root_radius;
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

void generate_octree(Octree* octree, Model* model) {
    *octree = initialize_octree(32, 2.0f);
    for(int i = 0; i < 8; i++) {
        octree->root.children[i].children = NULL;
        octree->root.children[i].filled_children = (int)(debug_rand()*255);
        message_log("filled-", octree->root.children[i].filled_children);
        octree->root.children[i].parent = &octree->root;
        octree->root.children[i].radius = octree->root.radius/2.0f;
        octree->root.children[i].position = octree->root.position +
            (octree->position_table[i] * octree->root.radius);
    }
}

void octree_debug_draw(Octree* octree, Octree_Node* node, Scene_Camera* camera,
    Object* marker) {
    glm::vec4 leaf_color;
    glm::vec3 leaf_pos;

    if(node->filled_children) {
        //if not subdivided then draw filled pieces of octree
        uint8_t filled_children = node->filled_children;
        for(int i = 0; i < 8; i++) {
            if(filled_children & 1) {
                leaf_pos = node->position + (octree->position_table[i] * node->radius);
                leaf_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
                gl_fast_draw_vao(camera, marker, leaf_pos, leaf_color, node->radius);
            }
            filled_children >>= 1;
        }
    } else if (node->children != NULL) {
        //if subdivided then check each of the children
        for(int i = 0; i < 8; i++) {
            //leaf_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.00f);
            //gl_fast_draw_vao(camera, marker, node->children[i].position,
            //    leaf_color, node->radius*0.99);
            octree_debug_draw(octree, &node->children[i], camera, marker);
        }
    }
}
