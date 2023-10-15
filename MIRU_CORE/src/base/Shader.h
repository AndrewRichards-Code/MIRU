#pragma once
#include "miru_core_common.h"
#include <filesystem>

namespace miru
{
namespace base
{
	enum class VertexType : uint32_t;
	enum class DescriptorType : uint32_t;

	class MIRU_API Shader
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
			
			TASK_BIT_EXT = 0x00000040,
			MESH_BIT_EXT = 0x00000080,

			RAYGEN_BIT = 0x00000100,
			ANY_HIT_BIT = 0x00000200,
			CLOSEST_HIT_BIT = 0x00000400,
			MISS_BIT = 0x00000800,
			INTERSECTION_BIT = 0x00001000,
			CALLABLE_BIT = 0x00002000,

			PIXEL_BIT = FRAGMENT_BIT,
			HULL_BIT = TESSELLATION_CONTROL_BIT,
			DOMAIN_BIT = TESSELLATION_EVALUATION_BIT,

			AMPLIFICATION_BIT_EXT = TASK_BIT_EXT,
		};

		struct VertexShaderInputAttributeDescription
		{
			uint32_t	location;
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

			uint32_t			dimension;
			bool				cubemap;
			bool				array_;
			bool				multisample;
			bool				readwrite;
		};

		//See MSCDocumentation.h for correct usage.
		struct CompileArguments
		{
			std::string					hlslFilepath;
			std::string					outputDirectory;	//Optional
			std::vector<std::string>	includeDirectories;	//Optional
			std::string					entryPoint;			//Required for non lib shaders.
			std::string					shaderModel;		//Optional
			std::vector<std::string>	macros;				//Optional
			bool						cso;				//Either cso or spv must be true
			bool						spv;				//Either cso or spv must be true
			std::vector<std::string>	dxcArguments;		//Optional
		};

		struct CreateInfo
		{
			std::string										debugName;
			void*											device;
			std::vector<std::pair<StageBit, std::string>>	stageAndEntryPoints;
			std::string										binaryFilepath;
			std::vector<char>								binaryCode;
			CompileArguments								recompileArguments;
		};

		//Methods
	public:
		static ShaderRef Create(CreateInfo* pCreateInfo);
		virtual ~Shader();
		const CreateInfo& GetCreateInfo() { return m_CI; }

		void Recompile();

		virtual void GetShaderResources() = 0;
		const std::vector<VertexShaderInputAttributeDescription>& GetVSIADs() const { return m_VSIADs; };
		const std::vector<PixelShaderOutputAttributeDescription>& GetPSOADs() const { return m_PSOADs; };
		const std::array<uint32_t, 3>& GetGroupCountXYZ() const { return m_GroupCountXYZ; };
		const std::map<uint32_t, std::map<uint32_t, ResourceBindingDescription>>& GetRBDs() const { return m_RBDs; };

	public:
		static std::vector<CompileArguments> LoadCompileArgumentsFromFile(std::filesystem::path filepath, const std::unordered_map<std::string, std::string>& environmentVariables = {});
		static void CompileShaderFromSource(const CompileArguments& arguments);

	protected:
		void GetShaderByteCode();
		virtual void Reconstruct() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
		std::vector<char> m_ShaderBinary;

		std::vector<VertexShaderInputAttributeDescription> m_VSIADs;
		std::vector<PixelShaderOutputAttributeDescription> m_PSOADs;
		std::array<uint32_t, 3> m_GroupCountXYZ;

		//Key is the set number
		std::map<uint32_t, std::map<uint32_t, ResourceBindingDescription>> m_RBDs;
	};
}
}