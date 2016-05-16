glm::vec3 parse_point_string(char* line) {
    //TODO: make this more resilient against instances of multiple characters
    //of whitespace, do this string parsing properly
    int i = 0; while(line[i] == 0x20) { i++; }
    GLfloat x; GLfloat y; GLfloat z;
    sscanf(&line[i], "%f %f %f", &x, &y, &z);
    return glm::vec3(x, y, z);
}

glm::vec4 convert_vec3_vec4(glm::vec3 vertex) {
    return glm::vec4(vertex.x, vertex.y, vertex.z, 1.f);
}

int parse_face_triplet(char* line, int* result) {
    int i = 0; while(line[i] == 0x20) { i++; }

    result[0] = 0; result[1] = 0; result[2] = 0;
    int results = sscanf(&line[i], "%i/%i/%i", &result[0], &result[1], &result[2]);

    result[0]--; result[1]--; result[2]--; //wavefront files are 1-indexed
    return i;
}

void compute_face_tb(Face* face, glm::vec3* tangent, glm::vec3* bitangent) {
    glm::vec4 q1; glm::vec4 q2;
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
    glm::normalize(tangent);

    bitangent->x = ((s1t1.x * q2.x) - (s2t2.x*q1.x)) / divisor;
    bitangent->y = ((s1t1.x * q2.y) - (s2t2.x*q1.y)) / divisor;
    bitangent->z = ((s1t1.x * q2.z) - (s2t2.x*q1.z)) / divisor;
    glm::normalize(bitangent);

    return;
}

bool load_model(const char* model_name, Model* model) {
    //Load file
    model->asset_path = construct_asset_path("models", model_name);
    char buffer[255]; FILE* file = fopen(model->asset_path, "r");

    if(file == NULL) {
        message_log("Error loading file-", model->asset_path); return false;
    }

    const char* vertex_label = "v"; const char* uv_label = "vt";
    const char* normal_label = "vn"; const char* face_label = "f";

    //Count number of items
    int vertex_count = 0; int uv_count = 0;
    int normal_count = 0; int face_count = 0;

    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        if(strncmp(buffer, uv_label, strlen(uv_label)) == 0) {
            uv_count++;
        }
        else if(strncmp(buffer, normal_label, strlen(normal_label)) == 0) {
            normal_count++;
        }
        else if(strncmp(buffer, vertex_label, strlen(vertex_label)) == 0) {
            vertex_count++;
        }
        else if(strncmp(buffer, face_label, strlen(face_label)) == 0) {
            face_count++;
        }
    }
    fseek(file, 0, SEEK_SET);

    glm::vec4* vertices = (glm::vec4*)malloc(sizeof(glm::vec4)*vertex_count);
    glm::vec3* uvs = (glm::vec3*)malloc(sizeof(glm::vec3)*uv_count);
    glm::vec3* normals = (glm::vec3*)malloc(sizeof(glm::vec3)*normal_count);
    glm::vec3* tangents = (glm::vec3*)malloc(sizeof(glm::vec3)*vertex_count);
    glm::vec3* bitangents = (glm::vec3*)malloc(sizeof(glm::vec3)*vertex_count);


    Indexed_Face* indexed_faces =
        (Indexed_Face*)malloc(sizeof(Indexed_Face)*face_count);
    Face* faces = (Face*)malloc(sizeof(Face)*face_count);
    model->face_count = face_count;
    model->faces = faces;

    //initialize tangents/bitangents
    int tangent_i;
    float* tangent_uses = (float*)malloc(sizeof(float)*vertex_count);
    for(tangent_i = 0; tangent_i < vertex_count; tangent_i++) {
        tangents[tangent_i] = glm::vec3(0.f, 0.f, 0.f);
        bitangents[tangent_i] = glm::vec3(0.f, 0.f, 0.f);
        tangent_uses[tangent_i] = 0.f;
    }

    //Read data
    int triplet_a[3]; int triplet_b[3]; int triplet_c[3];
    vertex_count = 0; uv_count = 0; normal_count = 0; face_count = 0;
    glm::vec3 tangent; glm::vec3 bitangent;
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        if(strncmp(buffer, uv_label, strlen(uv_label)) == 0) {
            uvs[uv_count] = parse_point_string(buffer+strlen(uv_label));
            uv_count++;
        }
        else if(strncmp(buffer, normal_label, strlen(normal_label)) == 0) {
            normals[normal_count] =
                parse_point_string(buffer+strlen(normal_label));
            normal_count++;
        }
        else if(strncmp(buffer, vertex_label, strlen(vertex_label)) == 0) {
            vertices[vertex_count] =
                convert_vec3_vec4(parse_point_string(buffer+strlen(vertex_label)));
            vertex_count++;
        }
        else if(strncmp(buffer, face_label, strlen(face_label)) == 0) {
            int i = strlen(face_label);
            i += parse_face_triplet(&buffer[i], triplet_a);
            faces[face_count].a.v = vertices[triplet_a[0]];
            faces[face_count].a.uv = uvs[triplet_a[1]];
            faces[face_count].a.n = normals[triplet_a[2]];
            while(buffer[i] != 0x20) { i++; }
            i += parse_face_triplet(&buffer[i], triplet_b);
            faces[face_count].b.v = vertices[triplet_b[0]];
            faces[face_count].b.uv = uvs[triplet_b[1]];
            faces[face_count].b.n = normals[triplet_b[2]];
            while(buffer[i] != 0x20) { i++; }
            i += parse_face_triplet(&buffer[i], triplet_c);
            faces[face_count].c.v = vertices[triplet_c[0]];
            faces[face_count].c.uv = uvs[triplet_c[1]];
            faces[face_count].c.n = normals[triplet_c[2]];

            //Compute tangent/bitangent, prep for averaging
            compute_face_tb(&faces[face_count], &tangent, &bitangent);
            tangents[triplet_a[0]] += tangent;
            bitangents[triplet_a[0]] += bitangent;
            tangent_uses[triplet_a[0]] += 1.f;

            tangents[triplet_b[0]] += tangent;
            bitangents[triplet_b[0]] += bitangent;
            tangent_uses[triplet_b[0]] += 1.f;

            tangents[triplet_c[0]] += tangent;
            bitangents[triplet_c[0]] += bitangent;
            tangent_uses[triplet_c[0]] += 1.f;

            //Record face vertex indices
            indexed_faces[face_count].a_index = triplet_a[0];
            indexed_faces[face_count].b_index = triplet_b[0];
            indexed_faces[face_count].c_index = triplet_c[0];

            face_count++;
        }
    }


    //compute average tangent/bitangent for each vertex
    float uses;
    for(tangent_i = 0; tangent_i < vertex_count; tangent_i++) {
        uses = tangent_uses[tangent_i];
        tangents[tangent_i] = tangents[tangent_i] / uses;
        bitangents[tangent_i] = bitangents[tangent_i] / uses;

        glm::normalize(&tangents[tangent_i]);
        glm::normalize(&bitangents[tangent_i]);
    }

    //attach tangents/bitangents to faces
    int vertex_index;
    for(face_count = 0; face_count < model->face_count; face_count++) {
        vertex_index = indexed_faces[face_count].a_index;
        faces[face_count].a.t = tangents[vertex_index];
        faces[face_count].a.b = bitangents[vertex_index];
        vertex_index = indexed_faces[face_count].b_index;
        faces[face_count].b.t = tangents[vertex_index];
        faces[face_count].b.b = bitangents[vertex_index];
        vertex_index = indexed_faces[face_count].c_index;
        faces[face_count].c.t = tangents[vertex_index];
        faces[face_count].c.b = bitangents[vertex_index];
    }

    free(vertices); free(uvs); free(normals); free(tangents); free(bitangents);
    free(indexed_faces);
    free(tangent_uses);
    return true;
}
