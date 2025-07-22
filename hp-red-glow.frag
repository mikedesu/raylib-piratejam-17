// hp-red-glow.frag
#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;
uniform float hp_frac; // For pulsing effect

void main() {
    vec4 color = texture2D(texture0, fragTexCoord);
    // red glow
    vec3 glowColor = vec3(1.0, 0.0, 0.0); 
    // A lower hp_frac should result in a more intense glow
    // 1.0 should result in no glow
    float glowIntensity = 1.0 - hp_frac; // Inverse relationship
    vec3 glowed = mix(color.rgb, glowColor, glowIntensity);
    finalColor = vec4(glowed, color.a);
}
