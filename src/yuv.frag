#version 330 core
in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D yTexture;
uniform sampler2D uTexture;
uniform sampler2D vTexture;

void main() {
    float y = texture(yTexture, texCoord).r;
    float u = texture(uTexture, texCoord).r;
    float v = texture(vTexture, texCoord).r;

    // Limited range expansion
    y = (y - 16.0/255.0)  * (255.0/219.0);
    u = (u - 128.0/255.0) * (255.0/224.0);
    v = (v - 128.0/255.0) * (255.0/224.0);

    vec3 rgb;
    rgb.r = y + 1.5748 * v;
    rgb.g = y - 0.1873 * u - 0.4681 * v;
    rgb.b = y + 1.8556 * u;

    // Optional if not using sRGB framebuffer
    // rgb = pow(rgb, vec3(1.0 / 2.2));

    FragColor = vec4(clamp(rgb, 0.0, 1.0), 1.0);
}
