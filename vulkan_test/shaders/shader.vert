#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location =0) in vec2 inPosition; // 传入 顶点
layout(location =1) in vec3 inColor;  // 传入颜色

layout (location =0 ) out vec3 fragColor;	// 传出颜色




out gl_PerVertex{
	vec4 gl_Position;
};



void main()
{
	gl_Position = vec4(inPosition, 0.0, 1.0);
	fragColor = inColor;
}