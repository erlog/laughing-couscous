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
            leaf_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.00f);
            gl_fast_draw_vao(camera, marker, node->children[i].position,
                leaf_color, node->radius);
            octree_debug_draw(octree, &node->children[i], camera, marker);
        }
    }
}
