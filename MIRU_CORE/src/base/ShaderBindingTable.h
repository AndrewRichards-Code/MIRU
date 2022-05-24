#pragma once
#include "miru_core_common.h"
#include "PipelineHelper.h"
#include "Buffer.h"

namespace miru
{
namespace base
{
	struct StridedDeviceAddressRegion;
	struct ShaderRecord
	{
		ShaderGroupHandleType	type;
		std::vector<uint8_t>	handle;
		std::vector<uint64_t>	parameters;
	};

	class MIRU_API ShaderBindingTable final
	{
		//enum/structs
	public:
		struct CreateInfo
		{
			std::string					debugName;
			void*						device;
			std::vector<ShaderRecord>	shaderRecords;
			AllocatorRef				allocator;
		};

		//Methods
	public:
		static ShaderBindingTableRef Create(ShaderBindingTable::CreateInfo* pCreateInfo);
		~ShaderBindingTable() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		ShaderBindingTable(ShaderBindingTable::CreateInfo* pCreateInfo);

		const StridedDeviceAddressRegion& GetStridedDeviceAddressRegion(const ShaderGroupHandleType& type) { return m_SDARs[type]; }

		//Members
	protected:
		CreateInfo m_CI = {};

	private:
		std::map<ShaderGroupHandleType, BufferRef> m_SBTs;
		std::map<ShaderGroupHandleType, StridedDeviceAddressRegion> m_SDARs;
	};
}
}