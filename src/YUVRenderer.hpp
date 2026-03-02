#pragma once

#include <iostream>
#include <cassert>
#include <vector>
#include "shaderClass.hpp"
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class YUVRenderer {

public:
    YUVRenderer();
    ~YUVRenderer();

    
    bool init(int videoWidth, int videoHeight);

    
    void uploadFrame(const uint8_t* yData, const uint8_t* uData, const uint8_t* vData,
        int yStride, int uvStride, int frameWidth, int frameHeight);

   
    void uploadRGBA(const std::vector<uint8_t>& rgbaData);    
    void useYUV(bool useYUV) { renderingYUV = useYUV; }
    void renderRect(float x0, float x1, float y0, float y1, int screenWidth, int screenHeight);

private:
   
    
    void createFullscreenQuad();

    GLint yTextureLoc;
    GLint uTextureLoc;
    GLint vTextureLoc;
    GLint rgbaTextureLoc;

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