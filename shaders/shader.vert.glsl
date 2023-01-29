#version 450

layout(set = 0, binding = 0) uniform  CameraUBO {
    mat4 view_proj;
} camera_data;

layout( push_constant ) uniform constants
{
    mat4 model;
} model_data;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;

void main() {
    gl_Position = camera_data.view_proj * model_data.model * vec4(in_position, 1.0);
    frag_color = in_color;
}