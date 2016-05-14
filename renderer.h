#ifndef RENDERER_H
#define RENDERER_H

//Structs
typedef struct c_texture {
    string asset_path;
    GLuint id;  //ID assigned to us by OpenGL
    GLsizei width;
    GLsizei height;
    int bytes_per_pixel;
    int pitch;  //width in bytes of each row of the image
    int buffer_size;
    uint8_t* buffer;
} Texture;

typedef struct c_point {
    int id;
    GLfloat x;
    GLfloat y;
    GLfloat z;
} Point;

typedef struct c_vertex {
    Point v;    //Vertex
    Point uv;   //Texture Coordinate
    Point n;    //Normal Vector
    Point t;    //Tangent Vector
    Point b;    //Bitangent Vector
} Vertex;

typedef struct c_face {
    Vertex a;
    Vertex b;
    Vertex c;
} Face;

typedef struct c_model {
    string asset_path;
    int face_count;
    int vertex_count;
    Face* faces;
} Model;

typedef struct c_object {
    //TODO: write code to free an object from memory
    string object_name;
    Model* model;
    Texture* texture;
    Texture* normal_map;
    Texture* specular_map;
    Point* location;
    GLuint shader_program; //ID assigned to our compiled shader by OpenGL
} Object;

typedef struct c_state {
    std::string AssetFolderPath;
    std::string OutputFolderPath;
    bool IsRunning;
    bool IsPaused;
    Texture* screen;
    uint32_t StartTime;
    uint32_t CurrentTime;
    uint32_t LastUpdateTime;
    uint32_t DeltaTime;
} State_Struct;

//generic utility functions
string construct_asset_path(string object_name, string filename);
//void message_log(const char* message, const char* predicate);

#endif

