//Vector methods
void normalize(glm::vec3* vector) {
    *vector = glm::normalize(*vector); return;
}
void normalize(glm::vec4* vector) {
    *vector = glm::normalize(*vector); return;
}

inline GLfloat distance_squared(glm::vec3 a, glm::vec3 b) {
    glm::vec3 c = b - a;
    return (c.x*c.x) + (c.y*c.y) + (c.z*c.z);
}

inline glm::vec3 absolute_difference(glm::vec3 a, glm::vec3 b) {
    glm::vec3 c = a - b;
    c.x = fabs(c.x); c.y = fabs(c.y); c.z = fabs(c.z);
    return c;
}

inline glm::vec3 floor_vector(glm::vec3 vec, GLfloat min) {
    if(vec.x < min) { vec.x = min; }
    if(vec.y < min) { vec.y = min; }
    if(vec.z < min) { vec.z = min; }
    return vec;
}

inline glm::vec3 mat4_multiply_vec3(glm::mat4 mat, glm::vec3 vec) {
    return glm::vec3( mat * glm::vec4(vec, 1.0f));
}

inline GLfloat vector_min_component(glm::vec3 vector) {
    GLfloat output = vector.x;
    if(vector.y < output) { output = vector.y; }
    if(vector.z < output) { output = vector.z; }
    return output;
}
inline GLfloat vector_max_component(glm::vec3 vector) {
    GLfloat output = vector.x;
    if(vector.y > output) { output = vector.y; }
    if(vector.z > output) { output = vector.z; }
    return output;
}

inline void vector_set_if_lower(glm::vec3* input, glm::vec3* output) {
    if(input->x < output->x) { output->x = input->x; }
    if(input->y < output->y) { output->y = input->y; }
    if(input->z < output->z) { output->z = input->z; }
}
inline void vector_set_if_higher(glm::vec3* input, glm::vec3* output) {
    if(input->x > output->x) { output->x = input->x; }
    if(input->y > output->y) { output->y = input->y; }
    if(input->z > output->z) { output->z = input->z; }
}

inline void cartesian_to_barycentric(glm::vec3* result, glm::vec3 cartesian,
    glm::vec3 a, glm::vec3 b, glm::vec3 c) {

    glm::vec3 vec_one;
    vec_one.x = c.x - a.x; vec_one.y = b.x - a.x; vec_one.z = a.x - cartesian.x;

    glm::vec3 vec_two;
    vec_two.x = c.y - a.y; vec_two.y = b.y - a.y; vec_two.z = a.y - cartesian.y;

    glm::vec3 vec_u = glm::cross(vec_one, vec_two);

    GLfloat x = 1.0 - ((vec_u.x + vec_u.y) / vec_u.z);
    GLfloat y = vec_u.y / vec_u.z;
    GLfloat z = vec_u.x / vec_u.z;
    GLfloat total = x + y + z;

    result->x = x/total;
    result->y = y/total;
    result->z = z/total;
    return;
}

//GL methods
void gl_toggle_wireframe(bool on) {
    if(on) { glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); }
    else { glPolygonMode( GL_FRONT_AND_BACK, GL_FILL); }
}

void gl_bind_mat(GLuint shader, glm::mat4 matrix, const char* variable) {
    GLint loc = glGetUniformLocation(shader, variable);
    if(loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
    }
    return;
}
void gl_bind_mat(GLuint shader, glm::mat3 matrix, const char* variable) {
    GLint loc = glGetUniformLocation(shader, variable);
    if(loc != -1) {
        glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
    }
    return;
}

void gl_bind_vec(GLuint shader, glm::vec3 vector, const char* variable) {
    GLint loc = glGetUniformLocation(shader, variable);
    if(loc != -1) {
        glUniform3fv(loc, 1, glm::value_ptr(vector));
    }
    return;
}

void gl_bind_vec(GLuint shader, glm::vec4 vector, const char* variable) {
    GLint loc = glGetUniformLocation(shader, variable);
    if(loc != -1) {
        glUniform4fv(loc, 1, glm::value_ptr(vector));
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
    return;
}


void gl_bind_texture(GLuint shader, GLuint texture_id, GLuint slot,
    const char* variable) {
    GLint loc = glGetUniformLocation(shader, variable);
    if(loc != -1) {
        glUniform1i(loc, slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture_id);
    }
    return;
}

void gl_fast_draw_vao(Scene_Camera* camera, Object* object, glm::vec3 position,
    glm::vec4 color, GLfloat scale) {
    //Build model matrix
    glm::mat4 model_matrix;
    model_matrix = glm::scale(glm::translate(model_matrix, position),
        glm::vec3(scale, scale, scale) );

    glm::mat4 model_view_projection = camera->projection * camera->view * model_matrix;
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(
        camera->view * model_matrix)));

    gl_bind_vec(object->shader->id, color, "matte_color");
    gl_bind_mat(object->shader->id, model_view_projection, "model_view_projection");
    gl_bind_mat(object->shader->id, normal_matrix, "normal_matrix");

    //Render VAO
    glDrawArrays(GL_TRIANGLES, 0, object->model->face_count*3);
}

inline glm::mat4 build_model_matrix(Object* object) {
    glm::mat4 model_matrix;
    model_matrix = glm::translate(model_matrix, object->physics->position);
    model_matrix *= object->model->rotation;
    model_matrix = glm::scale(model_matrix, object->model->scale);
    return model_matrix;
}

inline Glyph load_glyph(Font* font, char character) {
    //checks if already in cache and generates if it's not
    if(font->glyphs.find(character) == font->glyphs.end()) {
        message_log("Rendering glyph-", character);
        FT_Load_Char(font->face, character, FT_LOAD_RENDER);
        Glyph glyph; FT_Glyph ft_glyph; FT_BBox bbox;

        //Get glyph object and ask for the bounding box
        FT_Get_Glyph(font->face->glyph, &ft_glyph);
        FT_Glyph_Get_CBox(ft_glyph, FT_GLYPH_BBOX_PIXELS, &bbox);

        glGenTextures(1, &glyph.texture_id);
        glBindTexture(GL_TEXTURE_2D, glyph.texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->face->glyph->bitmap.width,
            font->face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
            font->face->glyph->bitmap.buffer);

        //Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Get glyph metrics
        glyph.center = glm::vec2(
            (bbox.xMin + ( (bbox.xMax-bbox.xMin)/2.0f )) / font->size,
            (bbox.yMin + ( (bbox.yMax-bbox.yMin)/2.0f )) / font->size);
        glyph.size = glm::vec2(
            font->face->glyph->bitmap.width/font->size,
            font->face->glyph->bitmap.rows/font->size);
        glyph.advance = glm::vec2(
            font->face->glyph->advance.x/(font->size*64.0f),
            font->face->glyph->advance.y/(font->size*64.0f));
        glyph.advance -= glyph.center; //precalculate advance from center

        font->glyphs[character] = glyph;
        FT_Done_Glyph(ft_glyph);
        return glyph;

    } else {
        return font->glyphs[character];
    }
}

void gl_draw_font_glyph(Scene_Camera* camera, Font* font, char character,
    GLfloat size) {
    Glyph glyph = load_glyph(font, character);
    font->quad->model->scale.x = glyph.size.x * size;
    font->quad->model->scale.y = glyph.size.y * size;
    font->quad->model->scale.z = 1.0f;

    //move from edge of bounding box to center
    font->quad->physics->position.x += (glyph.center.x * size);
    font->quad->physics->position.y += (glyph.center.y * size);

    glm::mat4 model_matrix = build_model_matrix(font->quad);
    glm::mat4 model_view_projection = camera->projection * camera->view * model_matrix;
    gl_bind_mat(font->quad->shader->id, model_view_projection, "model_view_projection");
    gl_bind_texture(font->quad->shader->id, glyph.texture_id, 0, "diffuse");

    glDrawArrays(GL_TRIANGLES, 0, font->quad->model->face_count*3);

    //move from center to right edge of the glyph based on
    //the precalculated advance from the center
    font->quad->physics->position.x += (glyph.advance.x * size);
    font->quad->physics->position.y += (glyph.advance.y * size);
}

void gl_draw_text(Scene_Camera* camera, Font* font, const char* text,
    GLfloat size) {
    glUseProgram(font->quad->shader->id);
    glm::vec3 start_position = font->quad->physics->position;
    glBindVertexArray(font->quad->vao);
    size_t length = strlen(text);
    for(size_t i = 0; i < length; i++) {
        gl_draw_font_glyph(camera, font, text[i], size);
    }
    font->quad->physics->position = start_position;
    glBindVertexArray(0);
}

void gl_draw_object(Scene_Camera* camera, Object* object) {
    glUseProgram(object->shader->id);
    glBindVertexArray(object->vao);

    //Build model matrix
    glm::mat4 model_matrix = build_model_matrix(object);

    //Combine matrices
    glm::mat4 model_view_projection = camera->projection * camera->view * model_matrix;
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(
        camera->view * model_matrix)));

    //Bind uniforms
    gl_bind_vec(object->shader->id, object->model->color, "matte_color");
    gl_bind_vec(object->shader->id, camera->direction, "camera_direction");
    gl_bind_vec(object->shader->id, object->light_direction, "light_direction");
    gl_bind_mat(object->shader->id, model_view_projection, "model_view_projection");
    gl_bind_mat(object->shader->id, normal_matrix, "normal_matrix");

    //bind textures
    gl_bind_texture(object->shader->id, object->texture->id, 0, "diffuse");
    gl_bind_texture(object->shader->id, object->normal_map->id, 1, "normal");
    gl_bind_texture(object->shader->id, object->specular_map->id, 2, "specular");

    //Render VAO
    glDrawArrays(GL_TRIANGLES, 0, object->model->face_count*3);

    //Unbind VAO
    glBindVertexArray(0);
}

void gl_register_object(Object* object) {
    //Pack model data in a Vertex Buffer Object and save it in a Vertex Array
    glGenVertexArrays(1, &object->vao);
    glGenBuffers(1, &object->vbo);

    glBindVertexArray(object->vao);

    glBindBuffer(GL_ARRAY_BUFFER, object->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Face)*object->model->face_count,
        object->model->faces, GL_STATIC_DRAW);

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
