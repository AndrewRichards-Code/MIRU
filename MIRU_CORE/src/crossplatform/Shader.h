#pragma once
#include "common.h"

namespace miru
{
namespace crossplatform
{
	enum class ShaderStageBit : uint32_t
	{
		VERTEX_BIT = 0x00000001,
		TESSELLATION_CONTROL_BIT = 0x00000002,
		TESSELLATION_EVALUATION_BIT = 0x00000004,
		GEOMETRY_BIT = 0x00000008,
		FRAGMENT_BIT = 0x00000010,
		COMPUTE_BIT = 0x00000020,
		ALL_GRAPHICS = 0x0000001F,

		PIXEL_BIT = FRAGMENT_BIT,
		HULL_BIT = TESSELLATION_CONTROL_BIT,
		DOMAIN_BIT = TESSELLATION_EVALUATION_BIT
	};

	class Shader
	{
		//enums/structs
	public:
		struct CreateInfo
		{
			const char*		debugName;
			void*			device;
			ShaderStageBit	stage;
			const char*		filepath;
			const char*		entryPoint;
		};

		//Methods
	public:
		static Ref<Shader> Create(CreateInfo* pCreateInfo);
		virtual ~Shader() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

	protected:
		void GetShaderByteCode();

		//Members
	protected:
		CreateInfo m_CI = {};
		std::vector<char> m_ShaderBinary;
	};
}
}