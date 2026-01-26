#include "shaderClass.hpp"

std::string get_file_contents(const char* filename)
{
    std::cout << "Trying to open: " << filename << std::endl;

    std::ifstream in(filename, std::ios::binary);
    if (in) {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return contents;
    }

    std::cerr << "ERROR: Could not open file: " << filename << std::endl;
    return ""; // Return empty string - NO THROW
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
    : Shader(vertexFile, fragmentFile, nullptr) {}


Shader::Shader(const char* vertexFile, const char* fragmentFile, const char* geometryFile)
{
    // Load source code from files
    std::string vertexCode = get_file_contents(vertexFile);
    std::string fragmentCode = get_file_contents(fragmentFile);
    std::string geometryCode = geometryFile ? get_file_contents(geometryFile) : "";

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();
    const char* geometrySource = geometryFile ? geometryCode.c_str() : nullptr;

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    compileErrors(vertexShader, "VERTEX");

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    compileErrors(fragmentShader, "FRAGMENT");

    // Compile geometry shader (optional)
    GLuint geometryShader = 0;
    if (geometryFile)
    {
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geometrySource, NULL);
        glCompileShader(geometryShader);
        compileErrors(geometryShader, "GEOMETRY");
    }

    // Link program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    if (geometryFile)
        glAttachShader(ID, geometryShader);
    glLinkProgram(ID);
    compileErrors(ID, "PROGRAM");

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryFile)
        glDeleteShader(geometryShader);
}


void Shader::Activate()
{

	glUseProgram(ID);

}

void Shader::Delete()
{

	glDeleteProgram(ID);

}

void Shader::compileErrors(unsigned int shader, const char* type)
{
    GLint hasCompiled;
    char infolog[1024];

    if (strcmp(type, "PROGRAM") != 0)
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infolog);
            std::cout << "SHADER_COMPILATION_ERROR for " << type << ":\n" << infolog << std::endl;
        }
        else {
            std::cout << type << " shader compiled successfully!" << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infolog);
            std::cout << "SHADER_LINKING_ERROR:\n" << infolog << std::endl;
        }
        else {
            std::cout << "Shader program linked successfully! ID: " << ID << std::endl;
        }
    }
}
