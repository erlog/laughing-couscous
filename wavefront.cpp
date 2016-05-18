void vec_print(glm::vec3 vector) {
    printf("%f, %f, %f\n", vector.x, vector.y, vector.z);
}

int parse_indexed_face(const char* line, int* triplet) {
    triplet[0] = 1; triplet[1] = 1; triplet[2] = 1;
    int pos = 0;
    while(line[pos] == 0x20) { pos++; }
    sscanf(&line[pos], "%i", &triplet[0]);
    //forward to just after next slash
    while(line[pos] != 0x2F) { pos++; } pos++;
    if(line[pos] != 0x2F) {
        //if the character isn't a slash then get the value
        sscanf(&line[pos], "%i", &triplet[1]);
        //forward to just after next slash
        while(line[pos] != 0x2F) { pos++; } pos++;
    }
    else {
        //go past the slash
        pos++;
    }
    if(line[pos] != 0x20) {
        sscanf(&line[pos], "%i", &triplet[2]);
    }
    triplet[0]--; triplet[1]--; triplet[2]--; //wavefront obj is 1-indexed
    return pos;
}

glm::vec3 parse_point_string(char* line) {
    //TODO: make this more resilient against instances of multiple characters
    //of whitespace, do this string parsing properly
    int i = 0; while(line[i] == 0x20) { i++; }
    GLfloat x = 0.f; GLfloat y=0.f; GLfloat z=0.f;
    sscanf(&line[i], "%f %f %f", &x, &y, &z);
    return glm::vec3(x, y, z);
}

void compute_face_tb(Face* face, glm::vec3* tangent, glm::vec3* bitangent) {
    glm::vec3 q1; glm::vec3 q2;
    glm::vec3 s1t1; glm::vec3 s2t2;
    Vertex* a = &face->a; Vertex* b = &face->b; Vertex* c = &face->c;
    //q1 = b.v - a.v; q2 = c.v - a.v
    q1 = b->v - a->v; q2 = c->v - a->v;
    //s1t1 = b.uv - a.uv; s2t2 = c.uv - a.uv
    s1t1 = b->uv - a->uv; s2t2 = c->uv - a->uv;
    //if s1t1 == s2t2 #otherwise we get NaN trying to divide infinity
    if(s1t1 == s2t2) {
        //I should probably just do the calculation then re-do it
        //with these inputs if I get NaN
        s1t1 = glm::vec3(1.f, 0.f, 0.f);
        s2t2 = glm::vec3(0.f, 1.f, 0.f);
    }

    GLfloat divisor = (s1t1.x*s2t2.y) - (s1t1.y*s2t2.x);

    tangent->x  = ((s2t2.y * q1.x) - (s1t1.y * q2.x)) / divisor;
    tangent->y  = ((s2t2.y * q1.y) - (s1t1.y * q2.y)) / divisor;
    tangent->z  = ((s2t2.y * q1.z) - (s1t1.y * q2.z)) / divisor;
    normalize(tangent);

    bitangent->x = ((s1t1.x * q2.x) - (s2t2.x*q1.x)) / divisor;
    bitangent->y = ((s1t1.x * q2.y) - (s2t2.x*q1.y)) / divisor;
    bitangent->z = ((s1t1.x * q2.z) - (s2t2.x*q1.z)) / divisor;
    normalize(bitangent);

    return;
}

int load_face_array(FILE* file, const char* label, Indexed_Face** faces) {
    char buffer[255]; int item_count = 0;
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        if(strncmp(buffer, label, strlen(label)) == 0) { item_count++; }
    }

    *faces = (Indexed_Face*)malloc(sizeof(Indexed_Face)*item_count);

    int i = 0; fseek(file, 0, SEEK_SET);
    int pos; int* triplet = (int*)malloc(sizeof(int)*3);
    Indexed_Face face;
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        if(strncmp(buffer, label, strlen(label)) == 0) {
            pos = strlen(label);
            pos += parse_indexed_face(&buffer[pos], triplet);
            face.a_v_index = triplet[0];
            face.a_uv_index = triplet[1];
            face.a_n_index = triplet[2];
            while(buffer[pos] != 0x20) { pos++; }
            pos += parse_indexed_face(&buffer[pos], triplet);
            face.b_v_index = triplet[0];
            face.b_uv_index = triplet[1];
            face.b_n_index = triplet[2];
            while(buffer[pos] != 0x20) { pos++; }
            pos += parse_indexed_face(&buffer[pos], triplet);
            face.c_v_index = triplet[0];
            face.c_uv_index = triplet[1];
            face.c_n_index = triplet[2];
            (*faces)[i] = face;
            i++;
        }
    }

    fseek(file, 0, SEEK_SET);
    return item_count;
}

int load_vec3_array(FILE* file, const char* label, glm::vec3** items) {
    char buffer[255]; int item_count = 0;
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        if(strncmp(buffer, label, strlen(label)) == 0) { item_count++; }
    }

    if(item_count == 0) {
        item_count = 1;
        *items = (glm::vec3*)malloc(sizeof(glm::vec3)*item_count);
        (*items)[0] = glm::vec3(0.f, 0.f, 0.f);
        fseek(file, 0, SEEK_SET);
        return 0;
    }

    *items = (glm::vec3*)malloc(sizeof(glm::vec3)*item_count);

    int i = 0; fseek(file, 0, SEEK_SET);
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        if(strncmp(buffer, label, strlen(label)) == 0) {
            (*items)[i] = parse_point_string(buffer+strlen(label));
            i++;
        }
    }

    fseek(file, 0, SEEK_SET);
    return item_count;
}

bool load_model(const char* model_name, Model* model) {
    //Load file
    model->asset_path = construct_asset_path("models", model_name, "obj");
    char buffer[255]; FILE* file = fopen(model->asset_path, "r");

    if(file == NULL) {
        message_log("Error loading file-", model->asset_path); return false;
    }

    glm::vec3* vertices;
    int vertex_count = load_vec3_array(file, "v ", &vertices);
    glm::vec3* uvs;
    int uv_count = load_vec3_array(file, "vt", &uvs);
    glm::vec3* normals;
    int normal_count = load_vec3_array(file, "vn", &normals);
    Indexed_Face* indexed_faces;
    int face_count = load_face_array(file, "f ", &indexed_faces);

    fclose(file);

    //Assemble Faces
    Face* faces = (Face*)malloc(sizeof(Face)*face_count);
    Indexed_Face indexed_face;
    Face face;
    for(int i = 0; i < face_count; i++) {
        indexed_face = indexed_faces[i];
        face.a.v = vertices[indexed_face.a_v_index];
        face.a.uv = uvs[indexed_face.a_uv_index];
        face.a.n = normals[indexed_face.a_n_index];
        face.a.t = glm::vec3(0.f, 0.f, 0.f);
        face.a.b = glm::vec3(0.f, 0.f, 0.f);
        face.b.v = vertices[indexed_face.b_v_index];
        face.b.uv = uvs[indexed_face.b_uv_index];
        face.b.n = normals[indexed_face.b_n_index];
        face.b.t = glm::vec3(0.f, 0.f, 0.f);
        face.b.b = glm::vec3(0.f, 0.f, 0.f);
        face.c.v = vertices[indexed_face.c_v_index];
        face.c.uv = uvs[indexed_face.c_uv_index];
        face.c.n = normals[indexed_face.c_n_index];
        face.c.t = glm::vec3(0.f, 0.f, 0.f);
        face.c.b = glm::vec3(0.f, 0.f, 0.f);
        faces[i] = face;
    }
    model->faces = faces;
    model->face_count = face_count;

    //bail if there's no UV's and thus we don't need tangents/bitangents
    if(uv_count == 0) {
        free(vertices); free(uvs); free(normals); free(indexed_faces);
        return true;
    }

    //initialize tangents/bitangents
    float* tangent_uses = (float*)malloc(sizeof(float)*vertex_count);
    glm::vec3* tangents = (glm::vec3*)malloc(sizeof(glm::vec3)*vertex_count);
    glm::vec3* bitangents = (glm::vec3*)malloc(sizeof(glm::vec3)*vertex_count);

    for(int i = 0; i < vertex_count; i++) {
        tangents[i] = glm::vec3(0.f, 0.f, 0.f);
        bitangents[i] = glm::vec3(0.f, 0.f, 0.f);
        tangent_uses[i] = 0.f;
    }

    glm::vec3 tangent; glm::vec3 bitangent;
    for(int i = 0; i < face_count; i++) {
        //Compute tangent/bitangent, prep for averaging
        compute_face_tb(&faces[i], &tangent, &bitangent);

        tangents[indexed_faces[i].a_v_index] += tangent;
        bitangents[indexed_faces[i].a_v_index] += bitangent;
        tangents[indexed_faces[i].b_v_index] += tangent;
        bitangents[indexed_faces[i].b_v_index] += bitangent;
        tangents[indexed_faces[i].c_v_index] += tangent;
        bitangents[indexed_faces[i].c_v_index] += bitangent;

        tangent_uses[indexed_faces[i].a_v_index] += 1.f;
        tangent_uses[indexed_faces[i].b_v_index] += 1.f;
        tangent_uses[indexed_faces[i].c_v_index] += 1.f;
    }


    //compute average tangent/bitangent for each vertex
    float uses;
    for(int i = 0; i < vertex_count; i++) {
        uses = tangent_uses[i];
        tangents[i] = tangents[i] / uses;
        bitangents[i] = bitangents[i] / uses;

        normalize(&tangents[i]);
        normalize(&bitangents[i]);
    }

    //attach tangents/bitangents to faces
    int vertex_index;
    for(int i = 0; i < face_count; i++) {
        faces[i].a.t = tangents[indexed_faces[i].a_v_index];
        faces[i].a.b = bitangents[indexed_faces[i].a_v_index];
        faces[i].b.t = tangents[indexed_faces[i].b_v_index];
        faces[i].b.b = bitangents[indexed_faces[i].b_v_index];
        faces[i].c.t = tangents[indexed_faces[i].c_v_index];
        faces[i].c.b = bitangents[indexed_faces[i].c_v_index];
    }

    free(vertices); free(uvs); free(normals); free(tangents); free(bitangents);
    free(indexed_faces); free(tangent_uses);
    return true;
}
