//C Standard Library
#include <time.h>
#include <float.h>
#include <math.h>
//C++ Stuff

//Other Libraries
#include <GL/glew.h>
#include <GL/glext.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
//#include <SDL_opengl_glext.h>
#include <ruby.h>
#include "lodepng.cpp"
//Namespaces
using namespace std;
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

void bind_mat4(GLuint shader, glm::mat4 matrix, const char* variable) {
    GLint loc = glGetUniformLocation(shader, variable);
    if(loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
    }
    return;
}

void bind_texture(GLuint shader, Texture* texture, GLuint slot,
    const char* variable) {
    GLint loc = glGetUniformLocation(shader, variable);
    if(loc != -1) {
        glUniform1i(loc, slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture->id);
    }
    return;
}

int main() {
    //INITIALIZATION- Failures here cause a hard exit
    State.AssetFolderPath = "objects";
    State.OutputFolderPath = "output";
    State.IsRunning = true;
    State.StartTime = 0;
    State.CurrentTime = 0;
    State.LastUpdateTime = 0;
    State.DeltaTime = 0;

    //Initialize screen struct and buffer for taking screenshots
    Texture screen; screen.asset_path = "Flamerokz";
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
        message_log("Couldn't initialize-", "SDL"); return 0;
    }
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 ); //Use OpenGL 2.1
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

    SDL_Window* window = SDL_CreateWindow( screen.asset_path, SDL_WINDOWPOS_CENTERED,
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
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CCW); //Default is CCW, counter-clockwise
    //glDepthRange(1.0, -1.0); //change the handedness of the z axis
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //GAME INIT- Failures here may cause a proper smooth exit when necessary

    //construct camera matrices
    State.Camera = (Scene_Camera*)malloc(sizeof(Scene_Camera));
    State.Camera->position = glm::vec3(0.f, 0.f, 3.f);
    State.Camera->facing = glm::vec3(0.f, 0.f, -1.f);
    State.Camera->orientation = glm::vec3(0.f, 1.f, 0.f);

    glm::mat4 projection_matrix;
    projection_matrix = glm::perspective(glm::radians(85.f),
        (float)screen.width/screen.height, 0.1f, 100.f);
    State.ProjectionMatrix = projection_matrix;

    //Load Objects
    State.ObjectCount = 3;
    State.Objects = (Object*)malloc(sizeof(Object)*3);
    load_object(&State.Objects[0], "cube.obj", "blank.png", "blank_nm.png",
        "blank_spec.png", "flat.vert", "flat.frag");
    State.Objects[0].model->position = glm::vec3(0.f, 0.f, 0.f);
    State.Objects[0].model->rotation = glm::vec3(1.f, 0.f, 0.f);
    load_object(&State.Objects[1], "wt_teapot.obj", "blank.png", "blank_nm.png",
        "blank_spec.png", "flat.vert", "flat.frag");
    State.Objects[1].model->position = glm::vec3(1.5f, 0.f, 0.f);
    State.Objects[1].model->rotation = glm::vec3(0.f, 1.f, 0.f);
    load_object(&State.Objects[2], "african_head.obj", "blank.png", "blank_nm.png",
        "blank_spec.png", "flat.vert", "flat.frag");
    State.Objects[2].model->position = glm::vec3(-1.5f, 0.f, 0.f);
    State.Objects[2].model->rotation = glm::vec3(0.f, 0.f, 1.f);

    Object* object;

    State.StartTime = current_time();
    int frames_drawn = 0;
    int object_i;

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
            //draw stuff
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for(object_i = 0; object_i < State.ObjectCount; object_i++) {
                object = &State.Objects[object_i];
                object->model->rotation_angle += 2.0f;

                glUseProgram(object->shader_id);
                glBindVertexArray(object->model->vao);

                //Bind diffuse texture
                bind_texture(object->shader_id, object->texture, 0,
                    "diffuse");
                bind_texture(object->shader_id, object->normal_map, 1,
                    "normal");
                bind_texture(object->shader_id, object->specular_map, 2,
                    "specular");

                //Bind matrices
                glm::mat4 model_matrix;
                model_matrix = glm::translate(model_matrix,
                    object->model->position);
                model_matrix = glm::rotate(model_matrix,
                    glm::radians(object->model->rotation_angle),
                    object->model->rotation);
                model_matrix = glm::scale(model_matrix, object->model->scale);
                bind_mat4(object->shader_id, model_matrix, "model");

                glm::mat4 view_matrix;
                view_matrix = glm::lookAt(State.Camera->position,
                    State.Camera->position + State.Camera->facing,
                    State.Camera->orientation);

                bind_mat4(object->shader_id, view_matrix, "view");
                bind_mat4(object->shader_id, State.ProjectionMatrix,
                    "projection");

                //Render VAO
                glDrawArrays(GL_TRIANGLES, 0, object->model->face_count*3);

                //Unbind VAO
                glBindVertexArray(0);
            }

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
