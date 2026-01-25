#pragma once

#include <iostream>
#include <cassert>
#include<vector>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class YUVRenderer {

public:
    YUVRenderer();
    ~YUVRenderer();

    
    bool init(int videoWidth, int videoHeight);

    
    void uploadFrame(const std::vector<uint8_t>& yPlane,
        const std::vector<uint8_t>& uPlane,
        const std::vector<uint8_t>& vPlane);

   
    void uploadRGBA(const std::vector<uint8_t>& rgbaData);

 
    void render(float x0, float x1, float y0, float y1);  
    void renderFullscreen();  

    
    void useYUV(bool useYUV) { renderingYUV = useYUV; }

private:
   
    GLuint compileShader(GLenum type, const char* source);
    GLuint createShaderProgram();
    void createFullscreenQuad();

    
    GLuint rgbaTexture = 0;
    GLuint yTexture = 0, uTexture = 0, vTexture = 0;

    
    GLuint rgbaShader = 0;
    GLuint yuvShader = 0;

 
    GLuint VAO = 0, VBO = 0, EBO = 0;

   
    int width = 0;
    int height = 0;
    bool initialized = false;
    bool renderingYUV = true;  
};