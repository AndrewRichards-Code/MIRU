#pragma once
#define MIRU_CORE_COMMON_MINDEF
#include "miru_core_common.h"
#include "Image.h"
#include "Shader.h"

namespace miru
{
namespace base
{
	enum class DescriptorType : uint32_t
	{
		SAMPLER = 0,
		COMBINED_IMAGE_SAMPLER = 1,
		SAMPLED_IMAGE = 2,
		STORAGE_IMAGE = 3,
		UNIFORM_TEXEL_BUFFER = 4,
		STORAGE_TEXEL_BUFFER = 5,
		UNIFORM_BUFFER = 6,
		STORAGE_BUFFER = 7,
		UNIFORM_BUFFER_DYNAMIC = 8,
		STORAGE_BUFFER_DYNAMIC = 9,
		INPUT_ATTACHMENT = 10,
		ACCELERATION_STRUCTURE = 1000150000,

		D3D12_RENDER_TARGET_VIEW = 0x1000001,
		D3D12_DEPTH_STENCIL_VIEW = 0x1000002
	};

	class MIRU_API DescriptorPool
	{		
		//enums/structs
	public:
		struct PoolSize
		{
			DescriptorType	type;
			uint32_t		descriptorCount;
		};
		struct CreateInfo
		{
			std::string				debugName;
			void*					device;
			std::vector<PoolSize>	poolSizes;
			uint32_t				maxSets;
		};
		//Methods
	public:
		static DescriptorPoolRef Create(CreateInfo* pCreateInfo);
		virtual ~DescriptorPool() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	class MIRU_API DescriptorSetLayout
	{
		//enums/structs
	public:
		struct Binding
		{
			uint32_t			binding;
			DescriptorType		type;
			uint32_t			descriptorCount; //Number of descriptor in a single binding, accessed as an array.
			Shader::StageBit	stage;
		};
		struct CreateInfo
		{
			std::string				debugName;
			void*					device;
			std::vector<Binding>	descriptorSetLayoutBinding; //Order by type and then by ascending binding number.
		};
		//Methods
	public:
		static DescriptorSetLayoutRef Create(CreateInfo* pCreateInfo);
		virtual ~DescriptorSetLayout() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	class MIRU_API DescriptorSet
	{
		//enums/structs
	public:
		struct DescriptorImageInfo
		{
			SamplerRef		sampler;
			ImageViewRef	imageView;
			Image::Layout	imageLayout;
		};
		struct DescriptorBufferInfo
		{
			BufferViewRef bufferView;
		};
		struct CreateInfo
		{
			std::string							debugName;
			DescriptorPoolRef					descriptorPool;
			std::vector<DescriptorSetLayoutRef>	descriptorSetLayouts; //One set is created for each DescriptorSetLayout provided.
		};

		//Methods
	public:
		static DescriptorSetRef Create(CreateInfo* pCreateInfo);
		virtual ~DescriptorSet() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t desriptorArrayIndex = 0) = 0; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		virtual void AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t desriptorArrayIndex = 0) = 0; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		virtual void AddAccelerationStructure(uint32_t index, uint32_t bindingIndex, const std::vector<AccelerationStructureRef>& accelerationStructures, uint32_t desriptorArrayIndex = 0) = 0; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		virtual void Update() = 0;

	protected:
		inline bool CheckValidIndex(uint32_t index) { return (index < static_cast<uint32_t>(m_CI.descriptorSetLayouts.size())); }
		#define CHECK_VALID_INDEX_RETURN(index) if (!CheckValidIndex(index)) {return;}

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}