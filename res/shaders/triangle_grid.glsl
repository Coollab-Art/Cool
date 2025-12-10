
// Resources about hexagons:
// https://www.youtube.com/watch?v=VmrIDyYiJBA&ab_channel=TheArtofCode
// https://andrewhungblog.wordpress.com/2018/07/28/shader-art-tutorial-hexagonal-grids/

vec2 Cool_tri_uv(vec2 uv, float grid_size)
{
    uv *= grid_size;
    vec2 TriRatio = vec2(1, sqrt(3.)/2.);
    vec2 h = TriRatio*.5;  
    vec2 a = mod(uv, TriRatio);
    a.x -= h.x;
    vec2 b = mod(uv - vec2(h.x, 0.), TriRatio);
    b.x -= h.x;
    (mod(floor(uv.y / TriRatio.y), 2.) == 0.0)?a.y = abs(a.y - TriRatio.y):b.y = abs(b.y - TriRatio.y);

    vec2 gv;
    if (length(vec2(a.x * TriRatio.y * 2., a.y))<length(vec2(b.x * TriRatio.y * 2., b.y)))
    {
        gv=mod(uv, TriRatio)-h;
    }
    else
    {
        gv=mod(uv - vec2(h.x, 0.), TriRatio) - h;
    }

    return gv;
}

vec2 Cool_tri_center_from_tri_id(vec2 id, float grid_size)
{
    return vec2(float(id.x)/2., (float(id.y) + .5) * sqrt(3.)/2.) / grid_size;
}

ivec2 Cool_tri_id(vec2 uv, float grid_size)
{
    uv *=grid_size * 2.;
    ivec2 id1, id2;
    id1.x = int(floor(uv.x * 2.));
    id2.x = int(ceil(uv.x * 2.));
    id1.y = int(floor(uv.y * 2. /sqrt(3.)));
    id2.y = id1.y;
    
    vec2 center1 = Cool_tri_center_from_tri_id(vec2(id1) * grid_size, grid_size);
    vec2 center2 = Cool_tri_center_from_tri_id(vec2(id2) * grid_size, grid_size);
    
    if(((id1.x+id1.y) / 2) * 2 != id1.x+id1.y)
    {
        center1.y += sqrt(3.) * 0.45;
        center2.y -= sqrt(3.)* 0.05;
    }
    else
    {
        center1.y -= sqrt(3.) * 0.05;
        center2.y += sqrt(3.)* 0.45;
    }
    center1.y /= sqrt(3.);
    center2.y /= sqrt(3.);
    uv.y /= sqrt(3.);
    
    return length_squared(uv - center1) < length_squared(uv - center2)
                   ? id1
                   : id2;
}