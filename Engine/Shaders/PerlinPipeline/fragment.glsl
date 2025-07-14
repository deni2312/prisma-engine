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
    vec2 uv = gl_FragCoord.xy / vec2(WIDTH, HEIGHT); // Replace WIDTH and HEIGHT with your actual resolution
    float scale = 8.0;

    // Different offsets for each color channel
    float r = noise(uv * scale + vec2(0.0, 0.0));
    float g = noise(uv * scale + vec2(10.0, 5.0));
    float b = noise(uv * scale + vec2(-5.0, 15.0));

    // Normalize to [0, 1]
    r = 0.5 + 0.5 * r;
    g = 0.5 + 0.5 * g;
    b = 0.5 + 0.5 * b;

    FragColor = vec4(r, g, b, 1.0);
}