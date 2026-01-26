#version 330 core
        in vec2 texCoord;
        out vec4 FragColor;
        uniform sampler2D rgbaTexture;
        void main() {
            FragColor = texture(rgbaTexture, texCoord);
        }