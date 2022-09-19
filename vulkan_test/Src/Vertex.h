#pragma once
#ifndef VERTEX_H
#define VERTEX_H

#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#include<glm/glm.hpp>
#include<vector>
#include<array>

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;


	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		{
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			/*
			VK_VERTEX_INPUT_RATE_VERTEX: �ƶ���ÿ����������һ��������Ŀ
			VK_VERTEX_INPUT_RATE_INSTANCE : ��ÿ��instance֮���ƶ�����һ��������Ŀ
				���ǲ���ʹ��instancing��Ⱦ�����Լ��ʹ��per - vertex data��ʽ��
			*/
		}

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
	{
		// ��������
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
		{
			// ����� 2 floats vec2
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);
			/*
			format�������������Ե����͡��ø�ʽʹ������ɫ��ʽһ����ö�٣��������е��ҡ����е���ɫ�����ͺ͸�ʽ�ǱȽϳ��õĴ��䡣
				float: VK_FORMAT_R32_SFLOAT
				vec2 : VK_FORMAT_R32G32_SFLOAT
				vec3 : VK_FORMAT_R32G32B32_SFLOAT
				vec4 : V_FORMAT_R32G32B32A32_SFLOAT
			��ɫ����(SFLOAT, UINT, SINT) ��λ���Ӧ������ɫ����������Ͷ�Ӧƥ�䡣����ʾ����
				ivec2 : VK_FORMAT_R32G32_SINT, ������32λ�з�������������ɵ�����
				uvec4 : VK_FORMAT_R32G32B32A32_UINT, ���ĸ�32λ�޷�����ʽ������ɵ�����
				double : VK_FORMAT_R64_SFLOAT, ˫���ȸ�����(64 - bit)
			*/

			// ��ɫ�� 3 floats vec3
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);
		}

		return attributeDescriptions;

	}
};




#endif // !Vertex_H

