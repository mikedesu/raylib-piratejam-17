// durability-black.frag
#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;
uniform float durability_fraction; // For that "rustic" durability loss

void main() {
    vec4 color = texture2D(texture0, fragTexCoord);
    // black 
    vec3 glowColor = vec3(0.0, 0.0, 0.0); 
    // A lower durability_fraction should result in a more intense black
    // 1.0 should result in all-black
    float glowIntensity = 1.0 - durability_fraction; // Inverse relationship
    vec3 glowed = mix(color.rgb, glowColor, glowIntensity);
    finalColor = vec4(glowed, color.a);
}
