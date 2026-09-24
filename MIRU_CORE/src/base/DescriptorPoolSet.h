#pragma once

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
		D3D12_DEPTH_STENCIL_VIEW = 0x1000002,
		D3D12_STRUCTURED_BUFFER = 0x1000003
	};

	constexpr uint32_t DescriptorUnboundedArrayCount = 0xFFFFFFFF;

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
			DeviceRef				device;
			std::vector<PoolSize>	poolSizes;
			uint32_t				maxSets;
			bool					updateAfterBind = false; //Update a descriptor in a set that in bound to a command, but only before execution.
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
			enum class FlagBit : uint32_t
			{
				NONE_BIT						= 0x00000000, //No flags.
				UPDATE_AFTER_BIND_BIT			= 0x00000001, //Update a used descriptor in a set that in bound to a recording command buffer. The lastest updates will be used for the submission.
				UPDATE_UNUSED_WHILE_PENDING_BIT	= 0x00000002, //Update an unused descriptor in a set that in bound to a executing command buffer.
				PARTIALLY_BOUND_BIT				= 0x00000004, //Allow empty descriptor bindings in a set, if they are not used.
				VARIABLE_DESCRIPTOR_COUNT_BIT	= 0x00000008, //Allows unbounded/variable-count arrays of descriptors.
			};

			uint32_t			binding;
			DescriptorType		type;
			uint32_t			descriptorCount; //Number of descriptors in a single binding, accessed as an array. This is the 'upper bound' of an unbounded/variable-count array.
			Shader::StageBit	stage;
			FlagBit				flags = FlagBit::NONE_BIT;
		};
		struct CreateInfo
		{
			std::string				debugName;
			DeviceRef				device;
			std::vector<Binding>	descriptorSetLayoutBinding; //Order by type and then by ascending binding number.
			bool					updateAfterBind = false; //Update a descriptor in a set that in bound to a command, but only before execution.
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
			std::vector<uint32_t>				descriptorCounts = {}; //The actually count of the unbounded/variable-count descriptor array. Only used for the last binding in each descriptor set, value ignored if not an unbounded/variable-count descriptor array.
		};

		//Methods
	public:
		static DescriptorSetRef Create(CreateInfo* pCreateInfo);
		virtual ~DescriptorSet() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t descriptorArrayIndex = 0) = 0; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		virtual void AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t descriptorArrayIndex = 0) = 0; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		virtual void AddAccelerationStructure(uint32_t index, uint32_t bindingIndex, const std::vector<AccelerationStructureRef>& accelerationStructures, uint32_t descriptorArrayIndex = 0) = 0; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		virtual void Update() = 0;
		virtual void Clear() = 0;

	protected:
		inline bool CheckValidIndex(uint32_t index) { return (index < static_cast<uint32_t>(m_CI.descriptorSetLayouts.size())); }
		#define CHECK_VALID_INDEX_RETURN(index) if (!CheckValidIndex(index)) {return;}

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}