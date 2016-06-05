inline glm::vec3 process_vector3D(aiVector3D vector) {
    glm::vec3 result = glm::vec3(vector.x, vector.y, vector.z);
    return result;
}

inline Vertex process_vertex(aiMesh* mesh, GLuint index) {
    Vertex vertex;
    vertex.v = process_vector3D(mesh->mVertices[index]);
    vertex.n = process_vector3D(mesh->mNormals[index]);
    if(mesh->mTextureCoords[0] != NULL) {
        vertex.uv = process_vector3D(mesh->mTextureCoords[0][index]);
        vertex.t = process_vector3D(mesh->mTangents[index]);
        vertex.b = process_vector3D(mesh->mBitangents[index]);
    }
    else {
        vertex.uv = glm::vec3(); vertex.t = glm::vec3(); vertex.b = glm::vec3();
    }

    return vertex;
}

void process_node(Model* model, const aiScene* scene, aiNode* node) {
    if(node->mNumMeshes == 0) { return; }
    aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];
    model->face_count = mesh->mNumFaces;
    model->faces = (Face*)walloc(sizeof(Face)*model->face_count);

    for(GLuint i = 0; i < model->face_count; i++) {
        Face face;
        face.a = process_vertex(mesh, mesh->mFaces[i].mIndices[0]);
        face.b = process_vertex(mesh, mesh->mFaces[i].mIndices[1]);
        face.c = process_vertex(mesh, mesh->mFaces[i].mIndices[2]);
        model->faces[i] = face;
    }

    return;
}

void process_quad_node(QuadModel* model, const aiScene* scene, aiNode* node) {
    if(node->mNumMeshes == 0) { return; }
    aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];
    model->face_count = mesh->mNumFaces;
    model->faces = (QuadFace*)walloc(sizeof(QuadFace)*model->face_count);

    glm::vec3 minimum; glm::vec3 maximum; glm::vec3 radii;
    for(GLuint i = 0; i < model->face_count; i++) {
        QuadFace face;
        face.a = process_vertex(mesh, mesh->mFaces[i].mIndices[0]);
        face.b = process_vertex(mesh, mesh->mFaces[i].mIndices[1]);
        face.c = process_vertex(mesh, mesh->mFaces[i].mIndices[2]);
        face.d = process_vertex(mesh, mesh->mFaces[i].mIndices[3]);
        face.center = (0.25f * face.a.v) + (0.25f * face.b.v) +
            (0.25f * face.c.v) + (0.25f * face.d.v);
        face.normal = (0.25f * face.a.n) + (0.25f * face.b.n) +
            (0.25f * face.c.n) + (0.25f * face.d.n);

        minimum = face.a.v; maximum = face.a.v;
        vector_set_if_lower(&face.a.v, &minimum); vector_set_if_lower(&face.b.v, &minimum);
        vector_set_if_lower(&face.c.v, &minimum); vector_set_if_lower(&face.d.v, &minimum);
        vector_set_if_higher(&face.a.v, &maximum); vector_set_if_higher(&face.b.v, &maximum);
        vector_set_if_higher(&face.c.v, &maximum); vector_set_if_higher(&face.d.v, &maximum);

        face.radii = absolute_difference(maximum, minimum)/2.0f;
        model->faces[i] = face;
    }

    return;
}

bool load_quad_mesh(QuadModel* model, const char* model_name) {
    //TODO: make this properly recursive and handle multiple meshes and stuff
    model->asset_path = construct_asset_path("models", model_name, "obj");

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(model->asset_path,
        aiProcess_GenNormals | aiProcess_CalcTangentSpace);

    if(!scene || (scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE) ||
        !scene->mRootNode) { return false; }

    process_quad_node(model, scene, scene->mRootNode->mChildren[0]);

    //Compute bounding box
    glm::vec3 minimum = model->faces[0].a.v;
    glm::vec3 maximum = model->faces[0].a.v;
    for(GLuint i = 0; i < model->face_count; i++) {
        vector_set_if_lower(&model->faces[i].a.v, &minimum);
        vector_set_if_higher(&model->faces[i].a.v, &maximum);
        vector_set_if_lower(&model->faces[i].b.v, &minimum);
        vector_set_if_higher(&model->faces[i].b.v, &maximum);
        vector_set_if_lower(&model->faces[i].c.v, &minimum);
        vector_set_if_higher(&model->faces[i].c.v, &maximum);
        vector_set_if_lower(&model->faces[i].d.v, &minimum);
        vector_set_if_higher(&model->faces[i].d.v, &maximum);
    }
    model->bounding_minimum = minimum;
    model->bounding_maximum = maximum;

    return true;
}
bool load_model(const char* model_name, Model* model) {
    //TODO: make this properly recursive and handle multiple meshes and stuff
    model->asset_path = construct_asset_path("models", model_name, "obj");

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(model->asset_path,
        aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if(!scene || (scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE) ||
        !scene->mRootNode) { return false; }

    process_node(model, scene, scene->mRootNode->mChildren[0]);

    //Compute bounding box
    glm::vec3 minimum = model->faces[0].a.v;
    glm::vec3 maximum = model->faces[0].a.v;
    for(GLuint i = 0; i < model->face_count; i++) {
        vector_set_if_lower(&model->faces[i].a.v, &minimum);
        vector_set_if_higher(&model->faces[i].a.v, &maximum);
        vector_set_if_lower(&model->faces[i].b.v, &minimum);
        vector_set_if_higher(&model->faces[i].b.v, &maximum);
        vector_set_if_lower(&model->faces[i].c.v, &minimum);
        vector_set_if_higher(&model->faces[i].c.v, &maximum);
    }
    model->bounding_minimum = minimum;
    model->bounding_maximum = maximum;

    return true;
}
