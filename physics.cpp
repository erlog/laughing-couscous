void physics_process_movement(Physics_Object* physics) {
    if(physics->velocity > 0) {
        physics->movement_vector.y = 0;
        normalize(&physics->movement_vector);
        physics->position += physics->time_remaining * physics->velocity *
            physics->movement_vector * physics->quaternion;
        physics->velocity -= physics->time_remaining *
            physics->velocity  * physics->deceleration_factor;
    } else {
        physics->velocity += 0;
    }

    physics->fall_speed += 9.81 * physics->time_remaining;
    physics->position.y -= physics->fall_speed * physics->time_remaining;
}
