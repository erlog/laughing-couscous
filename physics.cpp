void physics_face_movement_direction(Object* object) {

    GLfloat distance = glm::distance(object->physics->old_position,
        object->physics->position);
    glm::vec3 movement_vector;

    //checking distance prevents jitter
    if(distance > 0.002) {
        //face the direction of movement
        movement_vector = object->physics->position - object->physics->old_position;
    } else {
        //face forward
        movement_vector = glm::vec3(0.0f, 0.0f, -1.0f) * object->physics->quaternion;
        movement_vector.y = 0.0f;
    }

    //look vectors need to be inverted to become rotation vectors
    //TODO: convert this back to quaternions and lerp this properly over time
    normalize(&movement_vector);
    object->model->rotation = glm::inverse(glm::lookAt(
        glm::vec3(0.0f, 0.0f, 0.0f), movement_vector,
        glm::vec3(0.000001f, 1.0f, 0.0000001f)));

    return;
}

void physics_process_movement(Physics_Object* physics) {
    glm::vec3 direction_vector;
    physics->old_position = physics->position;
    if(physics->velocity > 0) {
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
    physics->fall_speed += 9.81 * physics->time_remaining;
    physics->position.y -= physics->fall_speed * physics->time_remaining;
}

bool process_collision(Game_Level* level, Physics_Object* physics) {

    QuadFace* face;
    glm::vec3 edge_point;
    GLfloat center_p; bool center_positive;
    GLfloat edge_p; bool edge_positive;

    //TODO: re-implement this with octree
    for(unsigned int i = 0; i < level->collision_model->face_count; i++) {
        //implemented using as reference:
        //http://www.peroxide.dk/download/tutorials/tut10/pxdtut10.html

        //Check if our sphere crosses a plane by shooting a ray from
        //the center of our sphere to the closest point facing the polygon
        face = &level->collision_model->faces[i];
        edge_point = physics->position - (physics->radii * face->normal);

        center_p = glm::dot(physics->position, face->normal) + face->distance;
        center_positive = (center_p >= 0);
        edge_p = glm::dot(edge_point, face->normal) + face->distance;
        edge_positive = (edge_p >= 0);

        //If signs don't match then it means center is on one side while
        //the closest edge point is on the other
        if(center_positive != edge_positive) {
            glm::vec3 intersection_point;
            GLfloat t; GLfloat t_denominator;

            //Get the point of intersection
            t_denominator = glm::dot(face->normal, edge_point - physics->position);

            //make sure the ray isn't parallel to the plane
            if(t_denominator != 0.0f) {
                //find the point of intersection
                t = -1.0f * center_p / t_denominator;
                lerp(&intersection_point, physics->position, edge_point, t);

                //check if point in quad
                //TODO: there's probably a more mathy way to do this
                if(intersection_point.x < face->minimum.x) { continue; }
                if(intersection_point.y < face->minimum.y) { continue; }
                if(intersection_point.z < face->minimum.z) { continue; }
                if(intersection_point.x > face->maximum.x) { continue; }
                if(intersection_point.y > face->maximum.y) { continue; }
                if(intersection_point.z > face->maximum.z) { continue; }

                //collided, we need to react
                GLfloat rebound_distance = glm::distance(intersection_point, edge_point);
                rebound_distance += 0.001f; //to make sure we're clear of the obstruction
                physics->position += (face->normal * rebound_distance);

                if(edge_point.y < physics->position.y) { physics->fall_speed = 0; }

                return true;
            }
        }
    }

    return false;
}
