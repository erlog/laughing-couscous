void point_print(Point* point) {
    printf("Point: %f, %f, %f\n", point->x, point->y, point->z);
    return;
}

void set_point(Point* point, GLfloat x, GLfloat y, GLfloat z) {
    point->x = x; point->y = y; point->z = z;
}

void point_add(Point* result, Point* left, Point* right) {
    result->x = left->x + right->x;
    result->y = left->y + right->y;
    result->z = left->z + right->z;
    return;
}

void point_subtract(Point* result, Point* left, Point* right) {
    result->x = left->x - right->x;
    result->y = left->y - right->y;
    result->z = left->z - right->z;
    return;
}

void normalize(Point* point) {
    float length = sqrtf( pow(point->x,2) + pow(point->y,2) + pow(point->z,2) );
    point->x /= length; point->y /= length; point->z /= length;
    return;
}

bool points_are_equal(Point* a, Point* b) {
    //This is unsafe
    //https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
    Point c; point_subtract(&c, a, b);
    if(c.x != 0.0) { return false; }
    if(c.y != 0.0) { return false; }
    if(c.z != 0.0) { return false; }
    return true;
}

Point parse_point_string(char* line) {
    Point point;
    int i = 0; while(line[i] == 0x20) { i++; }
    float x; float y; float z;
    sscanf(&line[i], "%f %f %f", &x, &y, &z);
    point.x = (GLfloat)x; point.y = (GLfloat)y; point.z = (GLfloat)z;
    return point;
}

int parse_face_triplet(char* line, int* result) {
    int i = 0; while(line[i] == 0x20) { i++; }
    sscanf(&line[i], "%i/%i/%i", &result[0], &result[1], &result[2]);
    result[0]--; result[1]--; result[2]--; //wavefront files are 1-indexed
    return i;
}

bool load_model(string object_name, Model* model) {
    //Load file
    model->asset_path = construct_asset_path(object_name, "object.obj");
    char buffer[255]; FILE* file = fopen(model->asset_path.c_str(), "r");
    if(file == NULL) {
        //message_log("Error loading file-", model->asset_path); return false;
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

    model->vertex_count = vertex_count;
    Point* vertices = (Point*)malloc(sizeof(Point)*vertex_count); vertex_count = 0;
    Point* uvs = (Point*)malloc(sizeof(Point)*uv_count); uv_count = 0;
    Point* normals = (Point*)malloc(sizeof(Point)*normal_count); normal_count = 0;

    model->face_count = face_count;
    Face* faces = (Face*)malloc(sizeof(Face)*face_count); face_count = 0;
    model->faces = faces;

    //Read data
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        if(strncmp(buffer, uv_label, strlen(uv_label)) == 0) {
            uvs[uv_count] = parse_point_string(buffer+strlen(uv_label));
            uvs[uv_count].id = uv_count;
            uv_count++;
        }
        else if(strncmp(buffer, normal_label, strlen(normal_label)) == 0) {
            normals[normal_count] =
                parse_point_string(buffer+strlen(normal_label));
            normals[normal_count].id = normal_count;
            normal_count++;
        }
        else if(strncmp(buffer, vertex_label, strlen(vertex_label)) == 0) {
            vertices[vertex_count] =
                parse_point_string(buffer+strlen(vertex_label));
            vertices[vertex_count].id = vertex_count;
            vertex_count++;
        }
        else if(strncmp(buffer, face_label, strlen(face_label)) == 0) {
            int triplet[3]; int i = strlen(face_label);
            i += parse_face_triplet(&buffer[i], &triplet[0]);
            faces[face_count].a.v = vertices[triplet[0]];
            faces[face_count].a.uv = uvs[triplet[1]];
            faces[face_count].a.n = normals[triplet[2]];
            set_point(&faces[face_count].a.t, 0.0, 0.0, 0.0);
            while(buffer[i] != 0x20) { i++; }
            i += parse_face_triplet(&buffer[i], &triplet[0]);
            faces[face_count].b.v = vertices[triplet[0]];
            faces[face_count].b.uv = uvs[triplet[1]];
            faces[face_count].b.n = normals[triplet[2]];
            set_point(&faces[face_count].b.t, 0.0, 0.0, 0.0);
            while(buffer[i] != 0x20) { i++; }
            i += parse_face_triplet(&buffer[i], &triplet[0]);
            faces[face_count].c.v = vertices[triplet[0]];
            faces[face_count].c.uv = uvs[triplet[1]];
            faces[face_count].c.n = normals[triplet[2]];
            set_point(&faces[face_count].c.t, 0.0, 0.0, 0.0);

            face_count++;
        }
    }

    //compute tangent vectors
    Vertex* a; Vertex* b; Vertex* c; int face_i;
    Point q1; Point q2;
    Point s1t1; Point s2t2;
    Point tangent;
    Point bitangent;
    Point* tangents = (Point*)malloc(sizeof(Point)*model->vertex_count);
    Point* bitangents = (Point*)malloc(sizeof(Point)*model->vertex_count);
    int* tangent_uses = (int*)malloc(sizeof(int)*model->vertex_count);

    //initialize tangents
    int tangent_i;
    for(tangent_i = 0; tangent_i < model->vertex_count; tangent_i++) {
        set_point(&tangents[tangent_i], 0.0, 0.0, 0.0);
        set_point(&bitangents[tangent_i], 0.0, 0.0, 0.0);
        tangent_uses[tangent_i] = 0;
    }

    for(face_i = 0; face_i < model->face_count; face_i++) {
        a = &faces[face_i].a; b = &faces[face_i].b; c = &faces[face_i].c;
        //q1 = b.v - a.v; q2 = c.v - a.v
        point_subtract(&q1, &b->v, &a->v);
        point_subtract(&q2, &c->v, &a->v);
        //s1t1 = b.uv - a.uv; s2t2 = c.uv - a.uv
        point_subtract(&s1t1, &b->uv, &a->uv);
        point_subtract(&s2t2, &c->uv, &a->uv);
        //if s1t1 == s2t2 #otherwise we get NaN trying to divide infinity
        if(points_are_equal(&s1t1, &s2t2)) {
            //I should probably just do the calculation then re-do it
            //with these inputs if I get NaN
            set_point(&s1t1, 1.0, 0.0, 0.0);
            set_point(&s2t2, 0.0, 1.0, 0.0);
        }

        GLfloat divisor = (s1t1.x*s2t2.y) - (s1t1.y*s2t2.x);

        tangent.x  = ((s2t2.y * q1.x) - (s1t1.y * q2.x)) / divisor;
        tangent.y  = ((s2t2.y * q1.y) - (s1t1.y * q2.y)) / divisor;
        tangent.z  = ((s2t2.y * q1.z) - (s1t1.y * q2.z)) / divisor;
        normalize(&tangent);

        bitangent.x = ((s1t1.x * q2.x) - (s2t2.x*q1.x)) / divisor;
        bitangent.y = ((s1t1.x * q2.y) - (s2t2.x*q1.y)) / divisor;
        bitangent.z = ((s1t1.x * q2.z) - (s2t2.x*q1.z)) / divisor;
        normalize(&bitangent);

        point_add(&tangents[a->v.id], &tangents[a->v.id], &tangent);
        point_add(&tangents[b->v.id], &tangents[b->v.id], &tangent);
        point_add(&tangents[c->v.id], &tangents[c->v.id], &tangent);

        point_add(&bitangents[a->v.id], &bitangents[a->v.id], &bitangent);
        point_add(&bitangents[b->v.id], &bitangents[b->v.id], &bitangent);
        point_add(&bitangents[c->v.id], &bitangents[c->v.id], &bitangent);

        tangent_uses[a->v.id]++;
        tangent_uses[b->v.id]++;
        tangent_uses[c->v.id]++;
    }

    //compute average tangent for each use
    int uses;
    for(tangent_i = 0; tangent_i < model->vertex_count; tangent_i++) {
        uses = tangent_uses[tangent_i];
        tangent = tangents[tangent_i]; bitangent = bitangents[tangent_i];

        tangent.x /= uses; tangent.y /= uses; tangent.z /= uses;
        bitangent.x /= uses; bitangent.y /= uses; bitangent.z /= uses;

        normalize(&tangent); normalize(&bitangent);
        tangents[tangent_i] = tangent; bitangents[tangent_i] = bitangent;
    }

    //assign tangents to vertices
    for(face_i = 0; face_i < model->face_count; face_i++) {
        a = &faces[face_i].a; b = &faces[face_i].b; c = &faces[face_i].c;
        a->t = tangents[a->v.id]; a->b = bitangents[a->v.id];
        b->t = tangents[b->v.id]; b->b = bitangents[b->v.id];
        c->t = tangents[c->v.id]; c->b = bitangents[c->v.id];
    }

    free(vertices); free(uvs); free(normals); free(tangents); free(tangent_uses);
    free(bitangents);
    return true;
}
