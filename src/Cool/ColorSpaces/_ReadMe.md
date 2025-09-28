
## How to add a color space

- Go in *generator_colors.py* and add it in the `color_spaces()` function.
- Run the *generator_colors.py* script you just modified.
- Go in *Cool/res/shaders/color_conversions.glsl* and add the conversions (from and to) between your new color space and XYZ. (The other conversions with all the color spaces are generated automatically using the XYZ ones. If you want to write one of those functions manually, you can tell the script to ignore it.)
- Go to *conversions.cpp* in this folder and implement the conversion functions with XYZ. They should match the ones you just wrote in glsl and it should just be a matter of copy-pasting what you did and adjusting the syntax.
- If you are working on Coollab: go to *src/Module_Nodes/gen_primitive_types.py* and run the script.
- If you are working on Coollab: edit the website to explain what your color space is and when it makes sense to use it. https://coollab-art.com/Tutorials/Writing%20Nodes/Color%20Spaces

## Alpha spaces, tips ans tricks

- When premultiplying / unpremultiplying, weird colors can arise if your alpha is not between 0 and 1. Always clamp it. In Cool this is done automatically when calling the recommended functions `Cool_premultiply_color(vec3 color, float alpha) -> vec3` and `Cool_unpremultiply_color(vec3 color, float alpha) -> vec3` that are in *Cool/res/shaders/color_conversions.glsl*, or the greyscale alternatives: `Cool_premultiply_greyscale(float greyscale, float alpha) -> float` and `Cool_unpremultiply_greyscale(float greyscale, float alpha) -> float`, or `Cool_apply_premultiplied_alpha_to_color_and_alpha` etc..