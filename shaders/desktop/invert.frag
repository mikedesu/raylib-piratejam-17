// invert.frag
#version 330 core
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;
void main() {
    vec4 color = texture(texture0, fragTexCoord);
    // Invert the color
    finalColor = vec4(1.0 - color.rgb, color.a);
}
// This shader inverts the color of the texture sampled at fragTexCoord
