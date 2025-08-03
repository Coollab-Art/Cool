const float TAU     = 6.283185307180;
const float PI      = 3.141592653590;
const float sqrt_3  = 1.73205;
const float FLT_MAX = 3.402823466e+38;
const float FLT_MIN = 1.175494351e-38;

// See https://www.iquilezles.org/www/articles/smin/smin.htm
float smooth_min_polynomial(float a, float b, float smoothing)
{
    float h = max(smoothing - abs(a - b), 0.0) / smoothing;
    return min(a, b) - h * h * smoothing * 0.25;
}

// We use the fact that `smax(a, b) = - smin(-a, -b)` and reuse the formula above for smin
float smooth_max_polynomial(float a, float b, float smoothing)
{
    float h = max(smoothing - abs(a - b), 0.0) / smoothing;
    return max(a, b) + h * h * smoothing * 0.25;
}

float ndot(vec2 a, vec2 b)
{
    return a.x * b.x - a.y * b.y;
}

float length_squared(vec2 p)
{
    return dot(p, p);
}

float length_squared(vec3 p)
{
    return dot(p, p);
}

float length_squared(vec4 p)
{
    return dot(p, p);
}

mat2 rotation_2D(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return mat2(
        +c, -s,
        +s, +c
    );
}

vec3 rotation_around_axis(vec3 p, vec3 ax, float ro)
{
    // from https://suricrasia.online/demoscene/functions/
    return mix(dot(ax, p) * ax, p, cos(ro)) + cross(ax, p) * sin(ro);
}

vec2 vec2_from_polar(float radius, float angle)
{
    return radius * vec2(cos(angle), sin(angle));
}

float Cool_angle(vec2 v)
{
    return v.x != 0.
               ? atan(v.y, v.x)
           : v.y > 0.
               ? PI / 2.
               : -PI / 2.;
}

vec2 Cool_polar_coordinates(vec2 v)
{
    return vec2(length(v), Cool_angle(v));
}

vec2 complex_product(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 complex_division(vec2 n, vec2 d)
{
    return vec2(n.x * d.x + n.y * d.y, n.y * d.x - n.x * d.y) / (d.x * d.x + d.y * d.y);
}

vec2 complex_pow(vec2 z, float n)
{
    float theta = atan(z.y, z.x);
    float log_r = log2(dot(z, z)) * 0.5; // log2(r) = 0.5 * log2(x^2 + y^2)
    float rn    = exp2(log_r * n);
    float angle = n * theta;
    return vec2(rn * cos(angle), rn * sin(angle));
}

vec2 Cool_apply_matrix_to_position_2D(mat3 matrix, vec2 pos)
{
    vec3 tmp = matrix * vec3(pos, 1.);
    return tmp.xy / tmp.z;
}
vec2 Cool_apply_matrix_to_normalized_direction_2D(mat3 matrix, vec2 dir)
{
    vec3 tmp = matrix * vec3(dir, 0.);
    return normalize(tmp.xy);
}

vec3 Cool_apply_matrix_to_position_3D(mat4 matrix, vec3 pos)
{
    vec4 tmp = matrix * vec4(pos, 1.);
    return tmp.xyz / tmp.w;
}
vec3 Cool_apply_matrix_to_normalized_direction_2D(mat4 matrix, vec3 dir)
{
    vec4 tmp = matrix * vec4(dir, 0.);
    return normalize(tmp.xyz);
}

#define saturate(v) clamp(v, 0., 1.)