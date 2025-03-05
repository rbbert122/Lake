#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec4 plane;

// Output
out vec2 text_coord;
out vec3 w_pos;
out vec3 w_N;

void main()
{
    text_coord = v_texture_coord;
    w_pos = (Model * vec4(v_position,1)).xyz;
    w_N = normalize( mat3(Model) * v_normal );
    gl_ClipDistance[0] = dot(Model * vec4(v_position,1), plane);
    gl_Position = Projection * View * Model * vec4(v_position, 1); 
}
