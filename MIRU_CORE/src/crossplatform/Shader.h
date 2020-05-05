#pragma once
#include "miru_core_common.h"

namespace miru
{
namespace crossplatform
{
	enum class VertexType : uint32_t;
	enum class DescriptorType : uint32_t;

	class Shader
	{
		//enums/structs
	public:
		enum class StageBit : uint32_t
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

		struct VertexShaderInputAttributeDescription
		{
			uint32_t	location;
			uint32_t	binding;
			VertexType	vertexType;
			uint32_t	offset;
			std::string semanticName;
		};
		typedef enum class VertexType Type;
		struct FragmentShaderOutputAttributeDescription
		{
			uint32_t	location;
			Type		outputType;
			std::string semanticName;
		};
		typedef FragmentShaderOutputAttributeDescription PixelShaderOutputAttributeDescription;
		struct ResourceBindingDescription
		{
			uint32_t			binding;
			DescriptorType		type;
			uint32_t			descriptorCount; //Number of descriptor in a single binding, accessed as an array.
			Shader::StageBit	stage;
		};

		struct CreateInfo
		{
			const char*		debugName;
			void*			device;
			StageBit		stage;
			const char*		filepath;
			const char*		entryPoint;
		};

		//Methods
	public:
		static Ref<Shader> Create(CreateInfo* pCreateInfo);
		virtual ~Shader() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		void Recompile();
		virtual void GetShaderResources() = 0;

		const std::vector<VertexShaderInputAttributeDescription>& GetVSIADs() const { return m_VSIADs; };
		const std::vector<PixelShaderOutputAttributeDescription>& GetPSOADs() const { return m_PSOADs; };
		const std::map<uint32_t, std::vector<ResourceBindingDescription>>& GetRBDs() const { return m_RBDs; };

	protected:
		void GetShaderByteCode();
		virtual void Reconstruct() = 0;
		void LoadAssemblyFile();

		//Members
	protected:
		CreateInfo m_CI = {};
		std::vector<char> m_ShaderBinary;

		std::vector<char> m_ShaderAssembly;
		bool m_AssemblyFileFound = false;

		std::vector<VertexShaderInputAttributeDescription> m_VSIADs;
		std::vector<PixelShaderOutputAttributeDescription> m_PSOADs;

		//Key the set number
		std::map<uint32_t, std::vector<ResourceBindingDescription>> m_RBDs;
	};
}
}