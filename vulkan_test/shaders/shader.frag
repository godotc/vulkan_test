#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding =1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor; // 从vert传过来的颜色
layout(location = 1) in vec2 fragTexCoord; // 纹理顶点坐标

layout(location = 0) out vec4 outColor; // 传出渲染颜色


void main(){
	outColor = vec4(texture(texSampler, fragTexCoord * 1.0)); // rgba
}