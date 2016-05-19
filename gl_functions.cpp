void normalize(glm::vec3* vector) {
    *vector = glm::normalize(*vector); return;
}
void normalize(glm::vec4* vector) {
    *vector = glm::normalize(*vector); return;
}

void gl_bind_mat4(GLuint shader, glm::mat4 matrix, const char* variable) {
    GLint loc = glGetUniformLocation(shader, variable);
    if(loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
    }
    return;
}

void gl_register_texture(Texture* texture) {
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0,
         GL_RGBA, GL_UNSIGNED_BYTE, texture->buffer);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
    glBindTexture(GL_TEXTURE_2D, 0); //unbind the texture
    return;
}

void gl_bind_texture(GLuint shader, Texture* texture, GLuint slot,
    const char* variable) {
    GLint loc = glGetUniformLocation(shader, variable);
    if(loc != -1) {
        glUniform1i(loc, slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture->id);
    }
    return;
}

void gl_recompute_camera_vector(Scene_Camera* camera) {
    camera->facing.x = cos(glm::radians(camera->pitch)) *
        cos(glm::radians(camera->yaw));
    camera->facing.y = sin(glm::radians(camera->pitch));
    camera->facing.z = cos(glm::radians(camera->pitch)) *
        sin(glm::radians(camera->yaw));
    normalize(&camera->facing);
}

void gl_draw_object(Scene_Camera* camera, Object* object) {
    glUseProgram(object->shader->id);
    glBindVertexArray(object->model->vao);

    //Bind diffuse texture
    gl_bind_texture(object->shader->id, object->texture, 0,
        "diffuse");
    gl_bind_texture(object->shader->id, object->normal_map, 1,
        "normal");
    gl_bind_texture(object->shader->id, object->specular_map, 2,
        "specular");

    //Bind matrices
    glm::mat4 model_matrix;
    model_matrix = glm::translate(model_matrix,
        object->physics->position + object->model->local_position);
    model_matrix = glm::rotate(model_matrix,
        glm::radians(object->model->local_rotation_angle),
        object->model->local_rotation);
    model_matrix = glm::rotate(model_matrix,
        glm::radians(object->physics->rotation_angle),
        object->physics->rotation);
    model_matrix = glm::scale(model_matrix, object->model->local_scale);
    gl_bind_mat4(object->shader->id, model_matrix, "model");

    glm::mat4 view_matrix;
    view_matrix = glm::lookAt(camera->position,
        camera->position + camera->facing, camera->orientation);

    gl_bind_mat4(object->shader->id, view_matrix, "view");
    gl_bind_mat4(object->shader->id, camera->projection,
        "projection");

    //Render VAO
    glDrawArrays(GL_TRIANGLES, 0, object->model->face_count*3);

    //Unbind VAO
    glBindVertexArray(0);
}

void gl_draw_debug_grid_lines() {
    glUseProgram(0);

    glBegin( GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, -1.0);
    glEnd();

    glBegin(GL_POINTS);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, -1.0);
    glEnd();
}

void gl_register_model(Model* model) {
    //Pack model data in a Vertex Buffer Object and save it in a Vertex Array
    glGenVertexArrays(1, &model->vao);
    glGenBuffers(1, &model->vbo);

    glBindVertexArray(model->vao);

    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Face)*model->face_count,
        model->faces, GL_STATIC_DRAW);

    //Bind vertices, uvs, normals, tangents, and bitangents
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 15*sizeof(GLfloat),
        (const GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 15*sizeof(GLfloat),
        (const GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 15*sizeof(GLfloat),
        (const GLvoid*)(6*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 15*sizeof(GLfloat),
        (const GLvoid*)(9*sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 15*sizeof(GLfloat),
        (const GLvoid*)(12*sizeof(GLfloat)));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
}

bool gl_load_shader(const char* shader_path, GLuint* shader_id, GLenum shader_type) {
    //Read in shader source
    FILE* file = fopen(shader_path, "r");
    if(file == NULL) {
        return false;
        message_log("Error loading file-", shader_path);
    }
    fseek(file, 0, SEEK_END); int length = ftell(file); fseek(file, 0, SEEK_SET);
    GLchar* shader_source = (GLchar*)walloc(sizeof(GLchar)*length+1);
    fread(shader_source, sizeof(GLchar), length, file);
    shader_source[length] = (GLchar)0;
    fclose(file);

    //Compile shader
    *shader_id = glCreateShader(shader_type);
    glShaderSource(*shader_id, 1, (const GLchar**)&shader_source, NULL);
    glCompileShader(*shader_id);

    GLint compiled = GL_FALSE;
    glGetShaderiv(*shader_id, GL_COMPILE_STATUS, &compiled);
    if(compiled != GL_TRUE ) {
        message_log("Open GL-", "error compiling shader");
        message_log("Source:", shader_source);
        GLchar buffer[2048];
        glGetShaderInfoLog(*shader_id, 2048, NULL, buffer);
        message_log("Open GL-", buffer);
        return false;
    }

    wfree(shader_source);
    return true;
}
