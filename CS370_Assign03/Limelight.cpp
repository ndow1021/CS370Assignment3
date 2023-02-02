// CS370 Assignment 3 - Limelight
// Fall 2022

#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#include<iostream>
#include<cstdlib>
#include<random>
#include<Math.h>
using namespace std;
#define DEG2RAD (M_PI/180.0)
#define RAD2DEG (180.0f/3.14159f)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Sphere, Octahedron, Circle, Cylinder, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Wood, RedPlastic, WhitePlastic, Brass};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
const char *objFiles[NumVAOs] = {"../models/unitcube.obj", "../models/sphere.obj", "../models/octahedron.obj", "../models/half_circle.obj", "../models/cylinder.obj"};

// Camera
vec3 eye = {1.0f, 1.0f, 1.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

// Shader variables
// Light shader program reference
GLuint lighting_program;
GLuint lighting_vPos;
GLuint lighting_vNorm;
GLuint lighting_camera_mat_loc;
GLuint lighting_model_mat_loc;
GLuint lighting_proj_mat_loc;
GLuint lighting_norm_mat_loc;
GLuint lighting_lights_block_idx;
GLuint lighting_materials_block_idx;
GLuint lighting_material_loc;
GLuint lighting_num_lights_loc;
GLuint lighting_light_on_loc;
GLuint lighting_eye_loc;
const char *lighting_vertex_shader = "../lighting.vert";
const char *lighting_frag_shader = "../lighting.frag";

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;

// Global light and material variables
vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0,0,0,0,0,0,0,0};

// Global variables
GLfloat cube_angle = 0.0;
GLfloat i = 0.0;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
vec3 axis = {0.0f, 1.0f, 0.0f};
vec3 cube_pos = { -6.0f, 01.0f, 0.0f };
GLboolean cube_slide = true;
GLfloat cube_dir = 1.0f;
GLfloat cube_slps = 3.0f;
vec3 sphere_pos = { 0.0f, 1.0f, -0.5f };
GLboolean sphere_bounce = true;
GLfloat sphere_dir = 1.0f;
GLfloat sphere_bps = 3.0f;
GLfloat SPHERE_MIN = 1.0f;
GLfloat SPHERE_MAX = 3.0f;
GLfloat CUBE_MIN = -1.0f;
GLfloat CUBE_MAX = 1.0f;
vec3 pyr_pos = { 6.0f, 1.0f, -0.5f };
GLboolean pyr_spin = true;
GLfloat pyr_ang = 0.0f;
GLfloat pyr_dps = 360.0f;
GLfloat red;
GLfloat blue;
GLfloat green;


// Global spherical camera variables
GLfloat azimuth = 45.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 53.5f;
GLfloat del = 2.0f;
GLfloat radius = 2.0f;

// Global screen dimensions
GLint ww,hh;

void display( );
void render_scene( );
void build_geometry();
void build_lights();
void build_materials();
void load_object(GLuint obj);
void draw_mat_object(GLuint obj, GLuint material);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Limelight");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }
    glfwGetFramebufferSize(window, &ww, &hh);
    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

	// Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();

    // Load shaders
    // Load light shader
    ShaderInfo lighting_shaders[] = { {GL_VERTEX_SHADER, lighting_vertex_shader},{GL_FRAGMENT_SHADER, lighting_frag_shader},{GL_NONE, NULL} };
    lighting_program = LoadShaders(lighting_shaders);
    lighting_vPos = glGetAttribLocation(lighting_program, "vPosition");
    lighting_vNorm = glGetAttribLocation(lighting_program, "vNormal");
    lighting_proj_mat_loc = glGetUniformLocation(lighting_program, "proj_matrix");
    lighting_camera_mat_loc = glGetUniformLocation(lighting_program, "camera_matrix");
    lighting_norm_mat_loc = glGetUniformLocation(lighting_program, "normal_matrix");
    lighting_model_mat_loc = glGetUniformLocation(lighting_program, "model_matrix");
    lighting_lights_block_idx = glGetUniformBlockIndex(lighting_program, "LightBuffer");
    lighting_materials_block_idx = glGetUniformBlockIndex(lighting_program, "MaterialBuffer");
    lighting_material_loc = glGetUniformLocation(lighting_program, "Material");
    lighting_num_lights_loc = glGetUniformLocation(lighting_program, "NumLights");
    lighting_light_on_loc = glGetUniformLocation(lighting_program, "LightOn");
    lighting_eye_loc = glGetUniformLocation(lighting_program, "EyePosition");

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // TODO: Enable alpha blending and set blend factors
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC0_RGB, GL_ONE);

    glDepthMask(GL_FALSE);
    glDepthMask(GL_TRUE);


    // Set background color
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    // Set Initial camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x, y, z);

    // Get initial time
    elTime = glfwGetTime();

	// Start loop
    while ( !glfwWindowShouldClose( window ) ) {
    	// Draw graphics
        display();
        // Update other events like input handling

        glfwPollEvents();

       // red = rand() % 255 / 100;
        //green = rand() % 255 / 100;
       // blue = rand() % 255 / 100;
        GLfloat frequency = .3;

            red   = sin(frequency*i + 0) * 127 + 128;
            green = sin(frequency*i + 2) * 127 + 128;
            blue  = sin(frequency*i + 4) * 127 + 128;
        i++;



        // TODO: Add animations
        GLdouble curTime = glfwGetTime();
        GLdouble dT = curTime - elTime;

        if (sphere_bounce == true)
        {
            sphere_pos[1] += sphere_dir*sphere_bps*dT;
            if (sphere_pos[1] > SPHERE_MAX) {
                sphere_pos[1] = SPHERE_MAX - 0.05f;
                sphere_dir *= -1;
            } else if (sphere_pos[1] < SPHERE_MIN) {
                sphere_pos[1] = SPHERE_MIN + 0.05f;
                sphere_dir *= -1;
            }

        }
        if (cube_slide == true)
        {
            cube_pos[2] += cube_dir*cube_slps*dT;
            if (cube_pos[2] > CUBE_MAX) {
                cube_pos[2] = CUBE_MAX - 0.05f;
                cube_dir *= -1;
            } else if (cube_pos[2] < CUBE_MIN) {
                cube_pos[2] = CUBE_MIN + 0.05f;
                cube_dir *= -1;
            }

        }
        if(pyr_spin == true)
        {
            pyr_ang += sphere_bps*dT*pyr_dps;

        }


        elTime = curTime;
        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;
}

void display( )
{
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

	// Clear window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set projection matrix
    // Set orthographic viewing volume anisotropic
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }
    // Set projection matrix
    proj_matrix = ortho(-12.0f*xratio, 12.0f*xratio, -12.0f*yratio, 12.0f*yratio, -12.0f, 12.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
	render_scene();

	glFlush();
}

void render_scene( ) {
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // TODO: Draw objects
    trans_matrix = translate(sphere_pos);
    scale_matrix = scale(0.5f,0.5f,0.5f);
    model_matrix = trans_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Sphere, WhitePlastic);


    trans_matrix = translate(cube_pos);
    scale_matrix = scale(0.5f,0.5f,0.5f);
    model_matrix = trans_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Cube, RedPlastic);

    scale_matrix = scale(20.0f,1.0f,12.0f);
    model_matrix = scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Cube, Brass);

    scale_matrix = scale(-6.0f,1.0f,-6.0f);
    trans_matrix = translate(0.0f,0.5f,4.0f);
    model_matrix = trans_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Circle, Brass);

    scale_matrix = scale(-10.0f,0.5f,-10.0f);
    trans_matrix = translate(0.0f,0.0f,4.0f);
    model_matrix = trans_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Cylinder, Brass);

    scale_matrix = scale(-6.0f,0.0f,-6.0f);
    trans_matrix = translate(0.0f,-0.55f,4.0f);
    model_matrix = trans_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Circle, Brass);


    rot_matrix = rotate(pyr_ang,0.0f,1.0f,0.0f);
    trans_matrix = translate(pyr_pos);
    scale_matrix = scale(0.5f,0.5f,0.5f);
    model_matrix = trans_matrix*scale_matrix*rot_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Octahedron, Wood);



}

void build_geometry( )
{
    // Generate vertex arrays for objects
    glGenVertexArrays(NumVAOs, VAOs);

    // TODO: Load objects
    load_object(Cylinder);
    load_object(Cube);
    load_object(Octahedron);
    load_object(Sphere);
    load_object(Circle);
}

void build_materials( ) {
    // TODO: Add materials

    MaterialProperties Brass = {
            vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
            vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
            vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
            27.8f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // Create red plastic material
    MaterialProperties RedPlastic = {
            vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.6f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties Wood = {
            vec4(0.5f, 0.35f, 0.05f, 0.55f), //ambient
            vec4(0.70f, 0.57f, 0.2f, 0.55f), //diffuse
            vec4(0.85f, 0.68f, 0.5f, 0.55f), //specular
            14.8f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };


    MaterialProperties WhitePlastic = {
            vec4(0.3f, 0.3f, 0.3f, 1.0f), //ambient
            vec4(0.6f, 0.6f, 0.6f, 1.0f), //diffuse
            vec4(0.8f, 0.8f, 0.8f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

/*
    MaterialProperties WhitePlastic = {
            vec4(0.3f, 0.3f, 0.3f, 1.0f), //ambient
            vec4(red, green, blue, 1.0f), //diffuse
            vec4(red + .2f, green + .2f, blue + .2f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    }; */

    Materials.push_back(Wood);
    Materials.push_back(RedPlastic);
    Materials.push_back(WhitePlastic);
    Materials.push_back(Brass);





    // Create uniform buffer for materials
    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    // TODO: Add lights
    LightProperties whitePointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(0.0f, 4.0f, -1.5f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };


    LightProperties greenSpotLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.0f, 1.0f, 0.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(6.0f,4.0f,-0.5f, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            30.0f,   //cutoff
            30.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    LightProperties redSpotLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(-6.0f,4.0f,0.0f, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            30.0f,   //cutoff
            30.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };




    LightProperties yellowSpotLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 0.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(0.0f, 4.0f, 0.0f, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            30.0f,   //cutoff
            30.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };


    Lights.push_back(whitePointLight);
    Lights.push_back(greenSpotLight);
    Lights.push_back(redSpotLight);
    Lights.push_back(yellowSpotLight);



    // Set numLights
    numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Esc exits program
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // TODO: Add key bindings

    if(key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        if(cube_slide == true)
        {
            cube_slide = false;
        }
        else if(cube_slide == false)
        {
            cube_slide = true;
        }
    }
    if(key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        if(sphere_bounce == true)
        {
            sphere_bounce = false;
        }
        else if(sphere_bounce == false)
        {
            sphere_bounce = true;
        }
    }
    if(key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        if(pyr_spin == true)
        {
            pyr_spin = false;
        }
        else if(pyr_spin == false)
        {
            pyr_spin = true;
        }
    }

    if(key == GLFW_KEY_0 && action == GLFW_PRESS)
    {
        if(lightOn[0] == 0)
        {
            lightOn[0] = 1;
        }
        else if(lightOn[0] == 1)
        {
            lightOn[0] = 0;
        }
    }

    if(key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        if(lightOn[1] == 0)
        {
            lightOn[1] = 1;
        }
        else if(lightOn[1] == 1)
        {
            lightOn[1] = 0;
        }
    }

    if(key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        if(lightOn[2] == 0)
        {
            lightOn[2] = 1;
        }
        else if(lightOn[2] == 1)
        {
            lightOn[2] = 0;
        }
    }

    if(key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        if(lightOn[3] == 0)
        {
            lightOn[3] = 1;
        }
        else if(lightOn[3] == 1)
        {
            lightOn[3] = 0;
        }
    }




    // Adjust azimuth
    if (key == GLFW_KEY_A) {
        azimuth += daz;
        if (azimuth > 360.0) {
            azimuth -= 360.0;
        }
    } else if (key == GLFW_KEY_D) {
        azimuth -= daz;
        if (azimuth < 0.0)
        {
            azimuth += 360.0;
        }
    }

    // Adjust elevation angle
    if (key == GLFW_KEY_W)
    {
        elevation += del;
        if (elevation > 180.0)
        {
            elevation = 179.0;
        }
    }
    else if (key == GLFW_KEY_S)
    {
        elevation -= del;
        if (elevation < 0.0)
        {
            elevation = 1.0;
        }
    }

    // Compute updated camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x,y,z);

}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

#include "utilfuncs.cpp"