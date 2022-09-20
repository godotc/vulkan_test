#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject{ // 统一变量，（运行时统一变化）
	mat4 model;
	mat4 view;
	mat4 proj;
}ubo;

layout(location =0) in vec2 inPosition; // 传入 顶点
layout(location =1) in vec3 inColor;  // 传入颜色
layout(location =2) in vec2 inTexCoord; // 纹理贴图坐标

layout(location =0) out vec3 fragColor;	// 传出颜色
layout(location =1) out vec2 fragTexCoord;  // 传出纹理坐标到 fragmentShader



out gl_PerVertex{
	vec4 gl_Position;
};


void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);

	fragColor = inColor;

	fragTexCoord =  inTexCoord;
}