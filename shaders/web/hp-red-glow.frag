precision mediump float;

varying mediump vec2 fragTexCoord;
uniform sampler2D texture0;
uniform mediump float hp_frac; // For pulsing effect

void main() {
    mediump vec4 color = texture2D(texture0, fragTexCoord);
    // red glow
    vec3 glowColor = vec3(1.0, 0.0, 0.0); 
    // A lower hp_frac should result in a more intense glow
    // 1.0 should result in no glow
    float glowIntensity = 1.0 - hp_frac; // Inverse relationship
    vec3 glowed = mix(color.rgb, glowColor, glowIntensity);
    gl_FragColor = vec4(glowed, color.a);
}
