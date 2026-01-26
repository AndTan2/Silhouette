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