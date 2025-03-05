#version 330

struct LightInfo
{
	vec3 position;
	vec3 color;
};


// Input
in vec2 text_coord;
in vec3 w_pos;
in vec3 w_N;

// Uniform properties
uniform sampler2D u_texture_0;
uniform LightInfo lights[40];
uniform vec3 eye_position;
uniform int lights_count;

// Output
layout(location = 0) out vec4 out_color;


// Local variables and functions
const vec3 LA = vec3 (0.004);             // ambient factor
const vec3 LD = vec3 (0.3);             // diffuse factor
const vec3 LS = vec3 (0.3);             // specular factor
const float SHININESS = 40.0;   // specular exponent

// Computes the output color using Phong lighting model.
// w_pos - world space position of the light source.
// w_N   - world space normal vector of the light source.
vec3 PhongLight(vec3 w_pos, vec3 w_N, vec3 light_position, vec3 light_color)
{
    vec3 L = normalize(light_position - w_pos);
	vec3 V = normalize( eye_position - w_pos );
	vec3 H = normalize( L + V );

    vec3 diffuse = LD * light_color * max( dot( w_N, L ), 0.0 );
    vec3 specular = vec3(0.0);
    if (diffuse.x > 0.0 && diffuse.y > 0.0 && diffuse.z > 0.0)
    {
        specular = LS * light_color * pow( max( dot( w_N, H ), 0.0 ), SHININESS );
    }

    float d = length(light_position - w_pos);
    float constant_att = 1.0;
	float linear_att = 0.14;
	float quadratic_att = 0.07;
	float att = 1.0 / (constant_att + linear_att * d + quadratic_att * d * d);
    return att * (diffuse + specular);
}

void main()
{
    vec3 color = texture(u_texture_0, text_coord).xyz;
    vec3 light_color = vec3(0.0);
    for (int i = 0; i < lights_count; i++)
    {
        light_color = light_color + PhongLight(w_pos, w_N, lights[i].position, lights[i].color);
    }
    light_color = light_color + LA;
    out_color = vec4(color * light_color, 1.0);
}
