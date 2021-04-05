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

			RAYGEN_BIT = 0x00000100,
			ANY_HIT_BIT = 0x00000200,
			CLOSEST_HIT_BIT = 0x00000400,
			MISS_BIT = 0x00000800,
			INTERSECTION_BIT = 0x00001000,
			CALLABLE_BIT = 0x00002000,

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
		struct FragmentShaderOutputAttributeDescription
		{
			uint32_t	location;
			VertexType	outputType;
			std::string semanticName;
		};
		typedef FragmentShaderOutputAttributeDescription PixelShaderOutputAttributeDescription;
		struct ResourceBindingDescription
		{
			uint32_t			binding;
			DescriptorType		type;
			uint32_t			descriptorCount; //Number of descriptor in a single binding, accessed as an array.
			Shader::StageBit	stage;
			std::string			name;
			size_t				structSize;
		};

		//See MSCDocumentation.h for correct usage.
		//All filepaths and directories must be relative to the current working directory.
		//All locations must be full paths i.e. dxc.
		struct RecompileArguments
		{
			std::string					mscDirectory;
			std::string					hlslFilepath;
			std::string					outputDirectory;	//Optional
			std::vector<std::string>	includeDirectories;	//Optional
			std::string					entryPoint;			//Required for non lib shaders.
			std::string					shaderModel;		//Optional
			std::vector<std::string>	macros;				//Optional
			bool						cso;				//Either cso or spv must be true
			bool						spv;				//Either cso or spv must be true
			std::string					dxcArguments;		//Optional. Example: ""\-Zi -Od"\".
			std::string					dxcLocation;		//Optional
			bool						nologo;				//Optional
			bool						nooutput;			//Optional
		};

		struct CreateInfo
		{
			std::string										debugName;
			void*											device;
			std::vector<std::pair<StageBit, std::string>>	stageAndEntryPoints;
			std::string										binaryFilepath;
			std::vector<char>								binaryCode;
			RecompileArguments								recompileArguments;
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
		const std::map<uint32_t, std::map<uint32_t, ResourceBindingDescription>>& GetRBDs() const { return m_RBDs; };

	protected:
		void GetShaderByteCode();
		int Call_MIRU_SHADER_COMPILER();
		virtual void Reconstruct() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
		std::vector<char> m_ShaderBinary;

		std::vector<VertexShaderInputAttributeDescription> m_VSIADs;
		std::vector<PixelShaderOutputAttributeDescription> m_PSOADs;

		//Key is the set number
		std::map<uint32_t, std::map<uint32_t, ResourceBindingDescription>> m_RBDs;
	};
}
}