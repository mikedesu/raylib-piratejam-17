precision mediump float;

varying mediump vec2 fragTexCoord;
uniform sampler2D texture0;
uniform mediump float durability_fraction; // For that "rustic" durability loss

void main() {
    mediump vec4 color = texture2D(texture0, fragTexCoord);
    // black 
    vec3 glowColor = vec3(0.0, 0.0, 0.0); 
    // A lower durability_fraction should result in a more intense black
    // 1.0 should result in all-black
    float glowIntensity = 1.0 - durability_fraction; // Inverse relationship
    vec3 glowed = mix(color.rgb, glowColor, glowIntensity);
    gl_FragColor = vec4(glowed, color.a);
}
