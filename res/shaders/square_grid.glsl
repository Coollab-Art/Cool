
// Resources about hexagons:
// https://www.youtube.com/watch?v=VmrIDyYiJBA&ab_channel=TheArtofCode
// https://andrewhungblog.wordpress.com/2018/07/28/shader-art-tutorial-hexagonal-grids/

vec2 Cool_sqa_uv(vec2 uv, float cell_size, float content_size)
{
    cell_size *= 2.;
    vec2  uv2 = uv / cell_size - 0.5;

    vec2 gid = floor(uv2);

    vec2 ret = normalize_uv_with_aspect_ratio(fract(uv2), 1.);

    vec2 center = uv - ret * cell_size;

    return ret / content_size;
}

vec2 Cool_sqa_id_center(vec2 uv, int number_of_tile)
{
    return floor(uv * number_of_tile) / number_of_tile + 1. / (2. * number_of_tile);
}

ivec2 Cool_sqa_id(vec2 uv, float cell_size)
{
    return ivec2(floor(uv.x/cell_size), floor(uv.y/cell_size));
}

