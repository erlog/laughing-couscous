//C Standard Library
#include <time.h>
#include <float.h>
#include <math.h>
//C++ Stuff
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;
//Other Libraries
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#include <ruby.h>
#include "lodepng.cpp"
//Local Headers
#include "renderer.h"
//State Struct
State_Struct State;
//Local Includes
#include "wavefront.cpp"
#include "utilities.cpp"
#include "hid_input.cpp"
//#include "ruby_functions.cpp"


uint32_t current_time() {
    return SDL_GetTicks();
}

void send_vertex(Vertex* vertex, GLint t_loc, GLint b_loc, GLint n_loc) {
    Point* v = &vertex->v; Point* uv = &vertex->uv;
    Point* t = &vertex->t; Point* b = &vertex->b; Point* n = &vertex->n;

    glVertexAttrib3f(t_loc, t->x, t->y, t->z);
    glVertexAttrib3f(b_loc, b->x, b->y, b->z);
    glVertexAttrib3f(n_loc, n->x, n->y, n->z);
    glTexCoord2f( uv->x, uv->y );
    glVertex3f( v->x, v->y, v->z );
}

int main() {
    glm::vec4 testpos = glm::vec4(1.f, 1.f, 1.f, 1.f);
    //INITIALIZATION- Failures here cause a hard exit
    State.AssetFolderPath = "objects";
    State.OutputFolderPath = "output";
    State.IsRunning = true;
    State.StartTime = 0;
    State.CurrentTime = 0;
    State.LastUpdateTime = 0;
    State.DeltaTime = 0;

    //Initialize screen struct and buffer for taking screenshots
    Texture screen; screen.asset_path = string("Flamerokz");
    screen.width = 384; screen.height = screen.width; screen.bytes_per_pixel = 3;
    screen.pitch = screen.width * screen.bytes_per_pixel;
    screen.buffer_size = screen.pitch * screen.height;
    screen.buffer = (uint8_t*)malloc(screen.buffer_size);
    State.screen = &screen;

    //Start Ruby
    //ruby_setup_render_environment();
    //VALUE rb_update_func = rb_intern("ruby_update");

    //Initialize SDL and OpenGL
    SDL_Event event;
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        //TODO: spit out actual SDL error code
        message_log("Couldn't initialize-","SDL"); return 0;
    }
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 ); //Use OpenGL 2.1
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

    SDL_Window* window = SDL_CreateWindow( screen.asset_path.c_str(), SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, screen.width, screen.height, SDL_WINDOW_OPENGL);
    if(window == NULL) {
        message_log("Couldn't initialize-", "SDL OpenGL window"); return 0;
    }
    SDL_GLContext context = SDL_GL_CreateContext( window );
    if(context == NULL) {
        message_log("Couldn't get-", "OpenGL Context for window"); return 0;
    }
    SDL_GL_SetSwapInterval( 1 ); //use vsync
    if(glewInit() != GLEW_OK) {
        message_log("Couldn't initialize-", "GLEW"); return 0;
    }
    if(!GLEW_VERSION_2_1) {
        message_log("OpenGL 2.1 not supported by GLEW", ""); return 0;
    }

    //Set up simple OpenGL environment for rendering
    glMatrixMode( GL_PROJECTION ); glLoadIdentity();
    glMatrixMode( GL_MODELVIEW ); glLoadIdentity();
    glClearColor( 0.f, 0.f, 0.f, 1.f );
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW); //Default is CCW, counter-clockwise
    glDepthRange(1.0, -1.0); //change the handedness of the z axis
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //Intialize Shaders

    //GAME INIT- Failures here may cause a proper smooth exit when necessary
    Object object; object.object_name = "african_head";
    if(!load_object(&object)) { State.IsRunning = false; };

    State.StartTime = current_time();
    int frames_drawn = 0;

    //MAIN LOOP- Failures here may cause a proper smooth exit when necessary
    message_log("Starting update loop.", "");

    while(State.IsRunning) {
        State.CurrentTime = current_time();
        State.DeltaTime = State.CurrentTime - State.LastUpdateTime;

        while(SDL_PollEvent(&event)) { switch(event.type) {
            case SDL_WINDOWEVENT:
                break;

            case SDL_KEYDOWN:
                handle_keyboard(event.key);
                break;

            case SDL_QUIT:
                State.IsRunning = false;
                break;
        } }

        //TODO: is there a better way to control our framerate?
        if( State.DeltaTime > 32 ) {
            //rb_funcall(rb_cObject, rb_update_func, 0, NULL);
            glRotatef(0.2f,0.1f,-0.1f,0.0f);
            //draw stuff
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(object.shader_program);

            //Bind diffuse texture
            GLint uniform_location = glGetUniformLocation(object.shader_program,
                "diffuse");
            glUniform1i(uniform_location, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, object.texture->id);

            //Bind normal map
            uniform_location = glGetUniformLocation(object.shader_program,
                "normal");
            glUniform1i(uniform_location, 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, object.normal_map->id);

            //Bind specular map
            uniform_location = glGetUniformLocation(object.shader_program,
                "specular");
            glUniform1i(uniform_location, 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, object.specular_map->id);

            //Bind Attributes
            GLint tangent_location = glGetAttribLocation(object.shader_program,
                "surface_tangent");
            GLint bitangent_location = glGetAttribLocation(object.shader_program,
                "surface_bitangent");
            GLint normal_location = glGetAttribLocation(object.shader_program,
                "surface_normal");


            glBegin( GL_TRIANGLES );
            int face_i; Face* face; Point* v; Point* uv; Point* n; Point* t;
            for(face_i = 0; face_i < object.model->face_count; face_i++) {
                face = &object.model->faces[face_i];
                send_vertex(&face->a, tangent_location, bitangent_location,
                    normal_location);
                send_vertex(&face->b, tangent_location, bitangent_location,
                    normal_location);
                send_vertex(&face->c, tangent_location, bitangent_location,
                    normal_location);
            }
            glEnd();

            //Debug Grid Lines
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

            SDL_GL_SwapWindow(window);
            State.LastUpdateTime = State.CurrentTime;
            frames_drawn++;
        }
    }

    take_screenshot();
    float fps = 1000.0/((float)(State.CurrentTime-State.StartTime)/frames_drawn);
    printf("%f FPS", fps); //TODO:use message log for this
    //ruby_cleanup(0);
    SDL_Quit();
    return 0;
}
