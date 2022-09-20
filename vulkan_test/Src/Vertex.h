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
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;


	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		{
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			/*
			VK_VERTEX_INPUT_RATE_VERTEX: 移动到每个顶点后的下一个数据条目
			VK_VERTEX_INPUT_RATE_INSTANCE : 在每个instance之后移动到下一个数据条目
				我们不会使用instancing渲染，所以坚持使用per - vertex data方式。
			*/
		}

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		// 顶点属性
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		{
			// 顶点段 2 floats vec2
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);
			/*
			format参数描述了属性的类型。该格式使用与颜色格式一样的枚举，看起来有点乱。下列的着色器类型和格式是比较常用的搭配。
				float: VK_FORMAT_R32_SFLOAT
				vec2 : VK_FORMAT_R32G32_SFLOAT
				vec3 : VK_FORMAT_R32G32B32_SFLOAT
				vec4 : V_FORMAT_R32G32B32A32_SFLOAT
			颜色类型(SFLOAT, UINT, SINT) 和位宽度应该与着色器输入的类型对应匹配。如下示例：
				ivec2 : VK_FORMAT_R32G32_SINT, 由两个32位有符号整数分量组成的向量
				uvec4 : VK_FORMAT_R32G32B32A32_UINT, 由四个32位无符号正式分量组成的向量
				double : VK_FORMAT_R64_SFLOAT, 双精度浮点数(64 - bit)
			*/

			// 颜色段 3 floats vec3
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			//  纹理贴图的坐标(四角的一角） floats vec2 
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
		}

		return attributeDescriptions;

	}
};




#endif // !Vertex_H

