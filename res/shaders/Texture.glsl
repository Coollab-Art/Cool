#include "_COOL_RES_/shaders/shader-utils.glsl" // For unnormalize_uv_with_aspect_ratio()

struct Cool_Texture {
    sampler2D tex;
    float     aspect_ratio;
    bool      flip_y;
};

vec4 sample_cool_texture(Cool_Texture tex, vec2 uv)
{
    if (tex.flip_y)
        uv.y *= -1.;
    return texture(tex.tex, unnormalize_uv_with_aspect_ratio(uv, tex.aspect_ratio));
}