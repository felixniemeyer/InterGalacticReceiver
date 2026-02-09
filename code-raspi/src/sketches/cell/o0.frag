#version 100
precision mediump float;

uniform float calc01; // Math.sin(time) + 1.5) * 0.05
uniform float calc02; // Math.sin(time * 0.5) + 1.0) * 0.12
uniform float rotate_opt_c; // cos(1 + 0.1 * time)
uniform float rotate_opt_s; // sin(1 + 0.1 * time)
uniform sampler2D tex_o1;
uniform float time;
uniform vec2 resolution;
varying vec2 uv;

float _luminance(vec3 rgb) {
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    return dot(rgb, W);
}

//	Simplex 3D Noise
//	by Ian McEwan, Ashima Arts
vec4 permute(vec4 x) {
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}
vec4 taylorInvSqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}

float _noise(vec3 v) {
    const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    //  x0 = x0 - 0. + 0.0 * C
    vec3 x1 = x0 - i1 + 1.0 * C.xxx;
    vec3 x2 = x0 - i2 + 2.0 * C.xxx;
    vec3 x3 = x0 - 1. + 3.0 * C.xxx;

    // Permutations
    i = mod(i, 289.0);
    vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y + vec4(0.0, i1.y, i2.y, 1.0)) + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    // Gradients
    // ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0 / 7.0; // N=7
    vec3 ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,N*N)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);    // mod(j,N)

    vec4 x = x_ * ns.x + ns.yyyy;
    vec4 y = y_ * ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    vec4 s0 = floor(b0) * 2.0 + 1.0;
    vec4 s1 = floor(b1) * 2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}

vec3 _rgbToHsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 _hsvToRgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 shape(vec2 _st, float sides, float radius, float smoothing) {
    vec2 st = _st * 2. - 1.;
    // Angle and radius from the current pixel
    float a = atan(st.x, st.y) + 3.1416;
    float r = (2. * 3.1416) / sides;
    float d = cos(floor(.5 + a / r) * r - a) * length(st);
    return vec4(vec3(1.0 - smoothstep(radius, radius + smoothing + 0.0000001, d)), 1.0);
}

vec4 sub(vec4 _c0, vec4 _c1, float amount) {
    return (_c0 - _c1) * amount + _c0 * (1.0 - amount);
}

vec4 add(vec4 _c0, vec4 _c1, float amount) {
    return (_c0 + _c1) * amount + _c0 * (1.0 - amount);
}

vec4 mult(vec4 _c0, vec4 _c1, float amount) {
    return _c0 * (1.0 - amount) + (_c0 * _c1) * amount;
}

vec2 modulateScale(vec2 _st, vec4 _c0, float multiple, float offset) {
    vec2 xy = _st - vec2(0.5);
    xy *= (1.0 / vec2(offset + multiple * _c0.r, offset + multiple * _c0.g));
    xy += vec2(0.5);
    return xy;
}

vec2 modulate(vec2 _st, vec4 _c0, float amount) {
    //  return fract(st+(_c0.xy-0.5)*amount);
    return _st + _c0.xy * amount;
}

vec2 scale(vec2 _st, float amount, float xMult, float yMult, float offsetX, float offsetY) {
    vec2 xy = _st - vec2(offsetX, offsetY);
    xy *= (1.0 / vec2(amount * xMult, amount * yMult));
    xy += vec2(offsetX, offsetY);
    return xy;

}

vec4 src(vec2 _st, sampler2D tex) {
    //  vec2 uv = gl_FragCoord.xy/vec2(1280., 720.);
    return texture2D(tex, fract(_st));
}

vec4 osc(vec2 _st, float frequency, float sync, float offset) {
    vec2 st = _st;
    float r = sin((st.x - offset / frequency + time * sync) * frequency) * 0.5 + 0.5;
    float g = sin((st.x + time * sync) * frequency) * 0.5 + 0.5;
    float b = sin((st.x + offset / frequency + time * sync) * frequency) * 0.5 + 0.5;
    return vec4(r, g, b, 1.0);
}

vec4 brightness(vec4 _c0, float amount) {
    return vec4(_c0.rgb + vec3(amount), _c0.a);
}

vec4 saturate(vec4 _c0, float amount) {
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec3 intensity = vec3(dot(_c0.rgb, W));
    return vec4(mix(intensity, _c0.rgb, amount), _c0.a);
}

vec2 kaleid(vec2 _st, float nSides) {
    vec2 st = _st;
    st -= 0.5;
    float r = length(st);
    float a = atan(st.y, st.x);
    float pi = 2. * 3.1416;
    a = mod(a, pi / nSides);
    a = abs(a - pi / nSides / 2.);
    return r * vec2(cos(a), sin(a));
}

vec4 voronoi(vec2 _st, float scale, float speed, float blending) {
    vec3 color = vec3(.0);
    // Scale
    _st *= scale;
    // Tile the space
    vec2 i_st = floor(_st);
    vec2 f_st = fract(_st);
    float m_dist = 10.;  // minimun distance
    vec2 m_point;        // minimum point
    for(int j = -1; j <= 1; j++) {
        for(int i = -1; i <= 1; i++) {
            vec2 neighbor = vec2(float(i), float(j));
            vec2 p = i_st + neighbor;
            vec2 point = fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
            point = 0.5 + 0.5 * sin(time * speed + 6.2831 * point);
            vec2 diff = neighbor + point - f_st;
            float dist = length(diff);
            if(dist < m_dist) {
                m_dist = dist;
                m_point = point;
            }
        }
    }
    // Assign a color using the closest point position
    color += dot(m_point, vec2(.3, .6));
    color *= 1.0 - blending * m_dist;
    return vec4(color, 1.0);
}

vec4 invert(vec4 _c0, float amount) {
    return vec4((1.0 - _c0.rgb) * amount + _c0.rgb * (1.0 - amount), _c0.a);
}

vec2 rotate_opt(vec2 _st) {
    vec2 xy = _st - vec2(0.5);
    xy = mat2(rotate_opt_c, -rotate_opt_s, rotate_opt_s, rotate_opt_c) * xy;
    xy += 0.5;
    return xy;
}

void main() {
    vec2 st = gl_FragCoord.xy / resolution.xy;

    st = scale(st, 1., 1., 1.5, 0.5, 0.5);
    vec2 st_c6_i0 = st;

    vec4 c6_i0 = src(st_c6_i0, tex_o1);
    st = modulate(st, c6_i0, 0.05);

    st = rotate_opt(st);
    vec2 st_c4_i0 = st;

    vec4 c4_i0 = src(st_c4_i0, tex_o1);
    st = modulateScale(st, c4_i0, 0.2, 1.);
    vec2 st_c3_i0 = st;

    st_c3_i0 = kaleid(st_c3_i0, 50.);

    vec4 c3_i0 = osc(st_c3_i0, 23., -0.05, 1000.);
    c3_i0 = brightness(c3_i0, 0.3);
    c3_i0 = saturate(c3_i0, 0.6);
    vec2 st_c2_i0 = st;
    vec2 st_c2_i0_c2_i01_i0 = st_c2_i0;

    vec4 c2_i01_i0 = shape(st_c2_i0_c2_i01_i0, 50., 0.4, 0.2);
    c2_i01_i0 = invert(c2_i01_i0, 1.);

    vec4 c2_i0 = voronoi(st_c2_i0, 60., 1., 5.);
    c2_i0 = mult(c2_i0, c2_i01_i0, 1.);
    vec2 st_c1_i0 = st;
    vec2 st_c1_i0_c1_i01_i0 = st_c1_i0;

    vec4 c1_i01_i0 = src(st_c1_i0_c1_i01_i0, tex_o1);
    st_c1_i0 = modulateScale(st_c1_i0, c1_i01_i0, 0.5, 1.);

    vec4 c1_i0 = shape(st_c1_i0, 50., calc02, 0.2);

    vec4 c = shape(st, 50., 0.4, calc01);
    c = sub(c, c1_i0, 1.);
    c = add(c, c2_i0, 1.);
    c = mult(c, c3_i0, 1.);
    gl_FragColor = c;
}
