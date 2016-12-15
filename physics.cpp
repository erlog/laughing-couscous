void physics_face_movement_direction(Object* object) {
    //bail if we haven't moved
    if(!object->physics->moved) { return; }

    GLfloat distance = glm::distance(object->physics->old_position,
        object->physics->position);
    //checking distance prevents jitter
    if(distance > 0.001) {
        glm::vec3 movement_vector;
        //face the direction of movement
        movement_vector = object->physics->position -
            object->physics->old_position;
        //look vectors need to be inverted to become rotation vectors
        //TODO: convert this back to quaternions and lerp this properly over time
        normalize(&movement_vector);
        object->model->rotation = glm::inverse(glm::lookAt(
            glm::vec3(0.0f, 0.0f, 0.0f), movement_vector,
            glm::vec3(0.000001f, 1.0f, 0.0000001f)));
        object->physics->quaternion = glm::quat_cast(object->model->rotation);
    } else {
        return;
        //face forward
        //movement_vector = glm::vec3(0.0f, 0.0f, -1.0f) * object->physics->quaternion;
        //movement_vector.y = 0.0f;
    }

    return;
}

void physics_process_movement(Physics_Object* physics) {
    glm::vec3 direction_vector;
    physics->old_position = physics->position;
    physics->moved = false;

    if( (physics->velocity > 0) ) {
        physics->moved = true;
        direction_vector = physics->movement_vector * physics->quaternion;
        direction_vector.y = 0;
        normalize(&direction_vector);

        physics->position += physics->time_remaining * physics->velocity *
           direction_vector;
        physics->velocity -= physics->time_remaining *
            physics->velocity  * physics->deceleration_factor;

    } else {
        physics->movement_vector = glm::vec3(0.0f, 0.0f, 0.0f);
        physics->velocity = 0;
    }

    //TODO: break gravity out into 2 separate processes
    //physics->fall_speed += 9.81 * physics->time_remaining;
    //physics->position.y -= physics->fall_speed * physics->time_remaining;
}

inline bool same_side(glm::vec3 p_a, glm::vec3 p_b, glm::vec3 a, glm::vec3 b) {
    //TODO: study enough to understand this
    glm::vec3 cp_a = glm::cross(b - a, p_a - a);
    glm::vec3 cp_b = glm::cross(b - a, p_b - a);
    if(glm::dot(cp_a, cp_b) >= 0) { return true; } else { return false; }
}

inline bool point_in_triangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    //Implemented using as reference: http://www.blackpawn.com/texts/pointinpoly/
    bool test;
    test = same_side(p, a, b, c); if(!test) { return false; }
    test = same_side(p, b, a, c); if(!test) { return false; }
    test = same_side(p, c, a, b); if(!test) { return false; }
    return true;
}

inline bool same_side_of_plane(glm::vec3 point_a, glm::vec3 point_b,
    glm::vec3 plane_normal, GLfloat planar_distance) {
    bool side_a = ((glm::dot(point_a, plane_normal) + planar_distance) >= 0);
    bool side_b = ((glm::dot(point_b, plane_normal) + planar_distance) >= 0);
    bool result = (side_a == side_b);
    return result;
}

bool process_collision(Game_Level* level, Physics_Object* physics) {
    Collision_Face* face;

    glm::vec3 point_a; glm::vec3 point_b; glm::vec3 intersection_point;
    GLfloat t; GLfloat t_denominator; GLfloat t_numerator;

    //TODO: re-implement this with octree
    for(unsigned int i = 0; i < level->collision_model->face_count; i++) {
        //implemented using as reference:
        //http://www.peroxide.dk/download/tutorials/tut10/pxdtut10.html
        //ALSO: scratchapixel.com minimal ray tracer rendering tutorial

        face = &level->collision_model->faces[i];

        //STEP 1: Take 2 points. Test which side of the plane they're on.
        point_a = physics->position - (face->normal * physics->radii);
        point_b = physics->position + (face->normal * physics->radii);

        if(!same_side_of_plane(point_a, point_b, face->normal, face->distance)){

            //STEP 2: If they're on opposite sides then figure out where the
            //line between them intersects with the plane
            t_denominator =
                glm::dot(face->normal, point_b - point_a);
            t_numerator =
                glm::dot(face->center - point_b, face->normal);

            t = t_numerator/t_denominator;
            lerp(&intersection_point, physics->position, point_a, t);


            //STEP 3: We have the point of intersection with the plane, but
            //we need to know if it's actually within the boundaries of the
            //triangle
            if(!point_in_triangle(intersection_point, face->a.v, face->b.v,
                face->c.v)) {
                    continue;
            }

            //STEP 4: Now that we know we've collided we need to react.
            GLfloat rebound_distance_a = glm::distance(intersection_point, point_a);
            GLfloat rebound_distance_b = glm::distance(intersection_point, point_b);
            GLfloat rebound_distance = min(rebound_distance_a, rebound_distance_b);
            rebound_distance += 0.001f; //to make sure we're clear of the obstruction
            physics->position += (face->normal * rebound_distance);

            return true;
        }
    }

    return false;
}
