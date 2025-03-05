#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 eye_position;

// Output
out vec4 clip_space;
out vec2 tex_coords;
out vec3 to_camera_vector;
out vec3 from_light_vector;

const float tiling = 10.0;

void main()
{
    vec4 world_position = Model * vec4(v_position, 1);
    clip_space = Projection * View * world_position;
    tex_coords = v_texture_coord * tiling;
    to_camera_vector = eye_position - world_position.xyz;
    from_light_vector = world_position.xyz - vec3(-25, 40, -100);
    gl_Position = clip_space; 
}
