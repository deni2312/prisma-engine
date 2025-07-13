#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

// 2D Perlin noise implementation

vec2 fade(vec2 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float grad(int hash, vec2 p) {
    vec2 g;
    hash = hash & 7;
    if (hash == 0) g = vec2(1,1);
    else if (hash == 1) g = vec2(-1,1);
    else if (hash == 2) g = vec2(1,-1);
    else if (hash == 3) g = vec2(-1,-1);
    else if (hash == 4) g = vec2(1,0);
    else if (hash == 5) g = vec2(-1,0);
    else if (hash == 6) g = vec2(0,1);
    else g = vec2(0,-1);
    return dot(g, p);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = fade(f);

    int a = int(i.x) + int(i.y) * 57;
    int b = a + 1;

    float n00 = grad(a, f);
    float n10 = grad(a + 1, f - vec2(1, 0));
    float n01 = grad(a + 57, f - vec2(0, 1));
    float n11 = grad(a + 58, f - vec2(1, 1));

    float nx0 = mix(n00, n10, u.x);
    float nx1 = mix(n01, n11, u.x);
    return mix(nx0, nx1, u.y);
}

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(WIDTH, HEIGHT); // adjust to your resolution
    float n = noise(uv * 8.0); // adjust scale here
    n = 0.5 + 0.5 * n; // normalize to [0, 1]
    FragColor = vec4(vec3(n), 1.0);
}
