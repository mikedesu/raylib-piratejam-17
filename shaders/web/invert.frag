precision mediump float;

varying mediump vec2 fragTexCoord;
uniform sampler2D texture0;

void main() {
    mediump vec4 color = texture2D(texture0, fragTexCoord);
    // Invert the color
    gl_FragColor = vec4(1.0 - color.rgb, color.a);
}
// This shader inverts the color of the texture sampled at fragTexCoord
