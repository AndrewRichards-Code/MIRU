#pragma once
#include "miru_core_common.h"
#include "PipelineHelper.h"
#include "Buffer.h"

namespace miru
{
namespace crossplatform
{
	struct StridedDeviceAddressRegion;
	struct ShaderRecord
	{
		ShaderGroupHandleType	type;
		std::vector<uint8_t>	handle;
		std::vector<uint64_t>	parameters;
	};

	class ShaderBindingTable final
	{
		//enum/structs
	public:
		struct CreateInfo
		{
			std::string					debugName;
			void*						device;
			std::vector<ShaderRecord>	shaderRecords;
			Ref<Allocator>				pAllocator;
		};

		//Methods
	public:
		static Ref<ShaderBindingTable> Create(ShaderBindingTable::CreateInfo* pCreateInfo);
		~ShaderBindingTable() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		ShaderBindingTable(ShaderBindingTable::CreateInfo* pCreateInfo);

		const StridedDeviceAddressRegion& GetStridedDeviceAddressRegion(const ShaderGroupHandleType& type) { return m_SDARs[type]; }

		//Members
	protected:
		CreateInfo m_CI = {};

	private:
		std::map<ShaderGroupHandleType, Ref<Buffer>> m_SBTs;
		std::map<ShaderGroupHandleType, StridedDeviceAddressRegion> m_SDARs;
	};
}
}