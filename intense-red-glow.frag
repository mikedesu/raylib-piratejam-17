// intense-red-glow.frag
#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;
uniform float time; // For pulsing effect

void main() {
    vec4 color = texture2D(texture0, fragTexCoord);
    // red glow
    vec3 glowColor = vec3(1.0, 0.0, 0.0); 
    // Pulse between 0.5 and 1.0
    float glowIntensity = 0.5 + 0.5 * sin(time * 2.0);
    vec3 glowed = mix(color.rgb, glowColor, glowIntensity);
    finalColor = vec4(glowed, color.a);
}
