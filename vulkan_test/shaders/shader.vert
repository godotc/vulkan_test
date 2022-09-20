#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject{ // ͳһ������������ʱͳһ�仯��
	mat4 model;
	mat4 view;
	mat4 proj;
}ubo;

layout(location =0) in vec3 inPosition; // ���� ����
layout(location =1) in vec3 inColor;  // ������ɫ
layout(location =2) in vec2 inTexCoord; // ������ͼ����

layout(location =0) out vec3 fragColor;	// ������ɫ
layout(location =1) out vec2 fragTexCoord;  // �����������굽 fragmentShader



out gl_PerVertex{
	vec4 gl_Position;
};


void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	fragColor = inColor;

	fragTexCoord =  inTexCoord;
}