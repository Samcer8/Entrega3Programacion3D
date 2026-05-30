#ifdef VERTEX_SHADER

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec4 a_color;

layout(std140, binding = 0) uniform Shader_Data {
  mat4 u_projection;
  mat4 u_transform;
  vec2 u_uv_size;
  vec2 u_uv_offset;
  vec4 u_tint;
  int u_tex_unit;
};

out vec2 v_uv;
out vec3 v_normal;
out vec4 v_color;
out vec3 v_frag_pos;
out flat int v_tex_unit; 

void main() {
    gl_Position = u_projection * u_transform * vec4(a_pos.xyz, 1.0);
    v_frag_pos = vec3(u_transform * vec4(a_pos.xyz, 1.0f));
    v_tex_unit = u_tex_unit;
    v_uv = a_uv;
    v_normal = normalize(mat3(transpose(inverse(u_transform))) * a_normal);
    v_color = a_color;
}
#endif

#ifdef FRAGMENT_SHADER

#define MAX_LIGHTS 8

layout(std140, binding = 1) uniform Light_Data {
    vec4 u_light_color[MAX_LIGHTS];
    vec4 u_light_pos[MAX_LIGHTS];
    vec4 u_view_pos;
};

in vec2 v_uv;
in vec3 v_normal;
in vec4 v_color;
in vec3 v_frag_pos;
in flat int v_tex_unit; 

layout(location = 0) out vec4 o_col;

#define MAX_TEXTURES 32

uniform sampler2D u_samplers[MAX_TEXTURES];

void main() {
  vec3 total_light = vec3(0.0);

    for(int i = 0; i < MAX_LIGHTS; i++)
    {
        vec3 light_dir =
            normalize(u_light_pos[i].xyz - v_frag_pos);

        float diffuse =
            max(dot(normalize(v_normal), light_dir), 0.0);

        vec3 ambient = u_light_color[i].rgb * 0.1;
        vec3 diffuseColor = u_light_color[i].rgb * diffuse;

        total_light += ambient + diffuseColor;
    }

    vec4 tex = texture(u_samplers[v_tex_unit], v_uv);

    o_col = vec4(total_light, 1.0) * tex;
}

#endif