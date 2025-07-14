
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

vec4 hash43(vec3 p) {
    p = fract(p * vec3(443.8975, 441.423, 437.195));
    p += dot(p, p.yzx + 19.19);
    return fract(vec4(p.x * p.y, p.y * p.z, p.z * p.x, p.x + p.y + p.z));
}

void main() {
    vec2 resolution = vec2(WIDTH,HEIGHT);  // replace with your WIDTH and HEIGHT
    vec2 uv = gl_FragCoord.xy / resolution;

    vec3 seed = vec3(gl_FragCoord.xy, 0.0);
    FragColor = hash43(seed);
}
