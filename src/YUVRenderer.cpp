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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // YUV textures
    glGenTextures(1, &yTexture);
    glBindTexture(GL_TEXTURE_2D, yTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0,
        GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glGenTextures(1, &uTexture);
    glBindTexture(GL_TEXTURE_2D, uTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0,
        GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glGenTextures(1, &vTexture);
    glBindTexture(GL_TEXTURE_2D, vTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0,
        GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create shaders
    yuvShader = createShaderProgram();
    if (!yuvShader) {
        std::cerr << "Failed to create YUV shader\n";
        return false;
    }


    const char* rgbaVert = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 texCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            texCoord = aTexCoord;
        }
    )";

    const char* rgbaFrag = R"(
        #version 330 core
        in vec2 texCoord;
        out vec4 FragColor;
        uniform sampler2D rgbaTexture;
        void main() {
            FragColor = texture(rgbaTexture, texCoord);
        }
    )";

    GLuint rgbaVertShader = compileShader(GL_VERTEX_SHADER, rgbaVert);
    GLuint rgbaFragShader = compileShader(GL_FRAGMENT_SHADER, rgbaFrag);

    if (rgbaVertShader && rgbaFragShader) {
        rgbaShader = glCreateProgram();
        glAttachShader(rgbaShader, rgbaVertShader);
        glAttachShader(rgbaShader, rgbaFragShader);
        glLinkProgram(rgbaShader);
        glDeleteShader(rgbaVertShader);
        glDeleteShader(rgbaFragShader);
    }

    // Create fullscreen quad
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

    glBindTexture(GL_TEXTURE_2D, rgbaTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
        GL_RGBA, GL_UNSIGNED_BYTE, rgbaData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void YUVRenderer::render(float x0, float x1, float y0, float y1) {
    if (!initialized) return;


    renderFullscreen();
}

void YUVRenderer::renderFullscreen() {
    if (!initialized) return;

    if (renderingYUV && yuvShader) {
        glUseProgram(yuvShader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, yTexture);
        glUniform1i(glGetUniformLocation(yuvShader, "yTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, uTexture);
        glUniform1i(glGetUniformLocation(yuvShader, "uTexture"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, vTexture);
        glUniform1i(glGetUniformLocation(yuvShader, "vTexture"), 2);
    }
    else if (rgbaShader) {
        glUseProgram(rgbaShader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rgbaTexture);
        glUniform1i(glGetUniformLocation(rgbaShader, "rgbaTexture"), 0);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


GLuint YUVRenderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint YUVRenderer::createShaderProgram() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 texCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            texCoord = aTexCoord;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 texCoord;
        out vec4 FragColor;
        
        uniform sampler2D yTexture;
        uniform sampler2D uTexture;
        uniform sampler2D vTexture;
        
        void main() {
            // Sample YUV textures
            float y = texture(yTexture, texCoord).r;
            vec2 uvCoord = texCoord * 0.5;  // UV planes are half resolution
            float u = texture(uTexture, uvCoord).r - 0.5;
            float v = texture(vTexture, uvCoord).r - 0.5;
            
            // YUV to RGB conversion (BT.601)
            float r = y + 1.402 * v;
            float g = y - 0.344136 * u - 0.714136 * v;
            float b = y + 1.772 * u;
            
            // Clamp and output
            FragColor = vec4(
                clamp(r, 0.0, 1.0),
                clamp(g, 0.0, 1.0),
                clamp(b, 0.0, 1.0),
                1.0
            );
        }
    )";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (!vertexShader || !fragmentShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
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