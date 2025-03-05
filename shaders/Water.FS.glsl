#version 330

// Input
in vec4 clip_space;
in vec2 tex_coords;
in vec3 to_camera_vector;
in vec3 from_light_vector;

// Uniform properties
uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;
uniform float move_factor;

// Output
layout(location = 0) out vec4 out_color;

void main()
{
    vec2 ndc = (clip_space.xy / clip_space.w) / 2.0 + 0.5;
    vec2 reflection_coords = vec2(ndc.x, -ndc.y);
    vec2 refraction_coords = vec2(ndc.x, ndc.y);

    vec2 distortedTexCoords = texture(dudvMap, vec2(tex_coords.x + move_factor, tex_coords.y)).rg*0.1;
	distortedTexCoords = tex_coords + vec2(distortedTexCoords.x, distortedTexCoords.y+move_factor);
	vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * 0.03;

    reflection_coords += totalDistortion;

    reflection_coords.x = clamp(reflection_coords.x, 0.001, 0.999);
    reflection_coords.y = clamp(reflection_coords.y, -0.999, -0.001);
    refraction_coords = clamp(refraction_coords, 0.001, 0.999);

    vec4 reflect_color = texture(reflection_texture, reflection_coords);
    vec4 refract_color = texture(refraction_texture, refraction_coords);

    vec3 view_vector = normalize(to_camera_vector);
    float refractive_factor = dot(view_vector, vec3(0.0, 1.0, 0.0));
    refractive_factor = pow(refractive_factor, 1.5);

    vec4 normal_color = texture(normalMap, distortedTexCoords);
    vec3 normal = normalize(vec3(normal_color.r * 2.0 - 1.0, normal_color.b, normal_color.g * 2.0 - 1.0));
    normal = normalize(normal);

    vec3 reflect_light = reflect(normalize(from_light_vector), normal);
    float splecular = pow(max(dot(reflect_light, view_vector), 0.0), 32.0);
    vec3 specular_highlights = vec3(1, 1, 1) * splecular * 0.5;

    out_color = mix(reflect_color, refract_color, refractive_factor);
    out_color = mix(out_color, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(specular_highlights, 1.0);
}
