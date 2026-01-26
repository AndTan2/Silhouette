#include "YUVRenderer.hpp"



YUVRenderer::YUVRenderer() {}

YUVRenderer::~YUVRenderer() {
    if (rgbaTexture) glDeleteTextures(1, &rgbaTexture);
    if (yTexture) glDeleteTextures(1, &yTexture);
    if (uTexture) glDeleteTextures(1, &uTexture);
    if (vTexture) glDeleteTextures(1, &vTexture);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (rgbaShader) glDeleteProgram(rgbaShader);
    if (yuvShader) glDeleteProgram(yuvShader);
}

bool YUVRenderer::init(int videoWidth, int videoHeight) {
    if (initialized) return true;

    this->width = videoWidth;
    this->height = videoHeight;

    // Create textures
    glGenTextures(1, &rgbaTexture);
    glBindTexture(GL_TEXTURE_2D, rgbaTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // YUV textures
    glGenTextures(1, &yTexture);
    glBindTexture(GL_TEXTURE_2D, yTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0,
        GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glGenTextures(1, &uTexture);
    glBindTexture(GL_TEXTURE_2D, uTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0,
        GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glGenTextures(1, &vTexture);
    glBindTexture(GL_TEXTURE_2D, vTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0,
        GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create shaders
    Shader yuvShaderObj("C:/Users/eu/source/repos/AndTan2/Silhouette/src/yuv.vert", "C:/Users/eu/source/repos/AndTan2/Silhouette/src/yuv.frag");
    yuvShader = yuvShaderObj.ID; 

    Shader rgbaShaderObj("C:/Users/eu/source/repos/AndTan2/Silhouette/src/rgba.vert", "C:/Users/eu/source/repos/AndTan2/Silhouette/src/rgba.frag");
    rgbaShader = rgbaShaderObj.ID; 

    yTextureLoc = glGetUniformLocation(yuvShader, "yTexture");
    uTextureLoc = glGetUniformLocation(yuvShader, "uTexture");
    vTextureLoc = glGetUniformLocation(yuvShader, "vTexture");
    rgbaTextureLoc = glGetUniformLocation(rgbaShader, "rgbaTexture");

    createFullscreenQuad();

    initialized = true;
    return true;
}

void YUVRenderer::uploadFrame(const std::vector<uint8_t>& yPlane,
    const std::vector<uint8_t>& uPlane,
    const std::vector<uint8_t>& vPlane) {
    if (!initialized) return;

    glBindTexture(GL_TEXTURE_2D, yTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
        GL_RED, GL_UNSIGNED_BYTE, yPlane.data());

    glBindTexture(GL_TEXTURE_2D, uTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2,
        GL_RED, GL_UNSIGNED_BYTE, uPlane.data());

    glBindTexture(GL_TEXTURE_2D, vTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2,
        GL_RED, GL_UNSIGNED_BYTE, vPlane.data());

    glBindTexture(GL_TEXTURE_2D, 0);
}

void YUVRenderer::uploadRGBA(const std::vector<uint8_t>& rgbaData) {
    if (!initialized) return;

    std::cout << "Uploading RGBA data: " << rgbaData.size() << " bytes" << std::endl;
    std::cout << "  Expected size: " << (width * height * 4) << " bytes" << std::endl;
    std::cout << "  Texture ID: " << rgbaTexture << std::endl;

    if (rgbaData.size() < width * height * 4) {
        std::cerr << "ERROR: RGBA data too small!" << std::endl;
        return;
    }

    glBindTexture(GL_TEXTURE_2D, rgbaTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
        GL_RGBA, GL_UNSIGNED_BYTE, rgbaData.data());

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error in uploadRGBA: " << err << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}




void YUVRenderer::renderRect(
    float x0, float x1,
    float y0, float y1,
    int screenWidth, int screenHeight
) {
    if (!initialized) return;

    // Convert pixel coordinates to normalized device coordinates [-1, 1]
    float ndc_x0 = (2.0f * x0 / screenWidth) - 1.0f;
    float ndc_x1 = (2.0f * x1 / screenWidth) - 1.0f;

    // Screen Y (down) -> OpenGL Y (up)
    float ndc_y_top = 1.0f - (2.0f * y0 / screenHeight);
    float ndc_y_bottom = 1.0f - (2.0f * y1 / screenHeight);

    // Update vertex positions for the rectangle
    float vertices[] = {
    ndc_x0, ndc_y_top,    0.0f, 0.0f,  // top-left
    ndc_x0, ndc_y_bottom, 0.0f, 1.0f,  // bottom-left
    ndc_x1, ndc_y_bottom, 1.0f, 1.0f,  // bottom-right
    ndc_x1, ndc_y_top,    1.0f, 0.0f   // top-right
    };

    // Update VBO with new vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Select shader + bind textures
    if (!renderingYUV && rgbaShader) {
        glUseProgram(rgbaShader);
        glUniform1i(rgbaTextureLoc, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rgbaTexture);
    }
    else if (renderingYUV && yuvShader) {
        glUseProgram(yuvShader);
        glUniform1i(yTextureLoc, 0);
        glUniform1i(uTextureLoc, 1);
        glUniform1i(vTextureLoc, 2);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, yTexture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, uTexture);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, vTexture);
    }

    // Draw
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void YUVRenderer::createFullscreenQuad() {
    float vertices[] = {
      
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}