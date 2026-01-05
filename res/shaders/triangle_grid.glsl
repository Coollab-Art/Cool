
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

vec2 center_test(ivec2 id, float cell_size)
{
    return vec2(float(id.x)/2., (float(id.y) + .5) * sqrt(3.)/ 2.) * cell_size;
}


ivec2 id_test(vec2 uv, float cell_size)
{
    ivec2 id1, id2;
    id1.x = int(floor(uv.x * 2. / cell_size));
    id2.x = int(ceil(uv.x * 2. / cell_size));
    id1.y = int(floor(uv.y * 2. /(sqrt(3.) * cell_size)));
    id2.y = id1.y;
    
    vec2 center1 = center_test(id1 , cell_size);
    vec2 center2 = center_test(id2, cell_size);
    // if(length_squared(uv - center1)< .001 || .001 > length_squared(uv - center2))
    // {
    //     return ivec2(-10, -10);
    // }    
    if(((id1.x+id1.y) / 2) * 2 != id1.x+id1.y)
    {
        center1.y += cell_size * sqrt(3.)/4.;
        center2.y -= cell_size * sqrt(3.)/4.;
    }
    else
    {
        center1.y -= cell_size * sqrt(3.)/4.;
        center2.y += cell_size * sqrt(3.)/4.;
    }
    // if(length_squared(uv - center1)< .01 || .01 > length_squared(uv - center2))
    // {
    //     return ivec2(-10, -10);
    // }
    center1.y /= sqrt(3.);
    center2.y /= sqrt(3.);
    uv.y /= sqrt(3.);
    

    ivec2 id;
    if(length(uv - center1) < length(uv - center2))
    {
        id = id1;
    }
    else
    {
        id = id2;
    }
    // if ((id.x == 0 ||id.x == 10) && (id.y == -1 || id.y == 0))//abs(float(id.y) - .5) <= 1.)
    // {
    //     return ivec2(-10, -10);
    // }
    return id;
}

vec2 Cool_tri_center_from_tri_id(ivec2 id, float cell_size)
{
    return vec2(float(id.x)/2., (float(id.y) + .5) * sqrt(3.)/2.) * cell_size;
}

ivec2 Cool_tri_id(vec2 uv, float grid_size)
{
    uv *=grid_size * 2.;
    ivec2 id1, id2;
    id1.x = int(floor(uv.x * 2.));
    id2.x = int(ceil(uv.x * 2.));
    id1.y = int(floor(uv.y * 2. /sqrt(3.)));
    id2.y = id1.y;
    
    vec2 center1 = Cool_tri_center_from_tri_id(id1 , grid_size * 2);
    vec2 center2 = Cool_tri_center_from_tri_id(id2, grid_size * 2);
    if(length_squared(uv - center1)< .01 || .01 > length_squared(uv - center2))
    {
        return ivec2(-10, -10);
    }    
    if(((id1.x+id1.y) / 2) * 2 != id1.x+id1.y)
    {
        center1.y += sqrt(3.)/2.;
        center2.y -= sqrt(3.)/2.;
    }
    else
    {
        center1.y -= sqrt(3.)/2.;
        center2.y += sqrt(3.)/2.;
    }
    // if(length_squared(uv - center1)< .01 || .01 > length_squared(uv - center2))
    // {
    //     return ivec2(-10, -10);
    // }
    center1.y /= sqrt(3.);
    center2.y /= sqrt(3.);
    uv.y /= sqrt(3.);
    

    ivec2 id;
    if(length(uv - center1) < length(uv - center2))
    {
        id = id1;
    }
    else
    {
        id = id2;
    }
    // if ((id.x == 0 ||id.x == 10) && (id.y == -1 || id.y == 0))//abs(float(id.y) - .5) <= 1.)
    // {
    //     return ivec2(-10, -10);
    // }
    return id;
}