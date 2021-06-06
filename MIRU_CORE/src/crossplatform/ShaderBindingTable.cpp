#include "miru_core_common.h"
#include "ShaderBindingTable.h"
#include "AccelerationStructure.h"

using namespace miru;
using namespace crossplatform;

Ref<ShaderBindingTable> ShaderBindingTable::Create(ShaderBindingTable::CreateInfo* pCreateInfo)
{
	return CreateRef<ShaderBindingTable>(pCreateInfo);
}

ShaderBindingTable::ShaderBindingTable(ShaderBindingTable::CreateInfo* pCreateInfo)
	:m_SBTs({ 
		{ ShaderGroupHandleType::RAYGEN, nullptr },
		{ ShaderGroupHandleType::MISS, nullptr },
		{ ShaderGroupHandleType::HIT_GROUP, nullptr },
		{ ShaderGroupHandleType::CALLABLE, nullptr } 
	})
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	std::map<ShaderGroupHandleType, std::vector<ShaderRecord>> shaderRecordsPerType;
	std::map<ShaderGroupHandleType, size_t> shaderRecordMaxStride = { 
		{ ShaderGroupHandleType::RAYGEN, 0 },
		{ ShaderGroupHandleType::MISS, 0 },
		{ ShaderGroupHandleType::HIT_GROUP, 0 },
		{ ShaderGroupHandleType::CALLABLE, 0 } 
	};

	for (auto& shaderRecord : m_CI.shaderRecords)
	{
		const ShaderGroupHandleType& type = shaderRecord.type;
		shaderRecordsPerType[type].push_back(shaderRecord);
		shaderRecordMaxStride[type] = std::max(shaderRecord.handle.size() + (shaderRecord.parameters.size() * sizeof(uint64_t)), shaderRecordMaxStride[type]);
	}
	
	std::map<ShaderGroupHandleType, std::vector<uint8_t>> shaderBindingTableData;
	for (auto& shaderRecords : shaderRecordsPerType)
	{
		const ShaderGroupHandleType& type = shaderRecords.first;
		const size_t& shaderRecordStride = shaderRecordMaxStride[type];
		size_t sbtDataSize = shaderRecords.second.size() * shaderRecordMaxStride[type];
		shaderBindingTableData[type].resize(sbtDataSize);

		size_t idx = 0;
		for (auto& shaderRecord : shaderRecords.second)
		{
			const size_t& shaderHandleSize = shaderRecord.handle.size();
			uint8_t* dstPtr = shaderBindingTableData[type].data() + (idx * shaderRecordStride);
			memcpy_s(dstPtr, shaderHandleSize, shaderRecord.handle.data(), shaderHandleSize);

			if (!shaderRecord.parameters.empty())
			{
				dstPtr += shaderHandleSize;
				const size_t& shaderParameterSize = shaderRecord.parameters.size() * sizeof(uint64_t);
				memcpy_s(dstPtr, shaderParameterSize, shaderRecord.parameters.data(), shaderParameterSize);
			}
			idx++;
		}
	}

	//Build SBT buffers.
	std::string debugNames[4] = {
		m_CI.debugName + " : SBT - Raygen",
		m_CI.debugName + " : SBT - Miss",
		m_CI.debugName + " : SBT - Hit",
		m_CI.debugName + " : SBT - Callable",
	};
	for (auto& sbt : m_SBTs)
	{
		const ShaderGroupHandleType& type = sbt.first;
		if (!shaderBindingTableData[type].empty())
		{
			crossplatform::Buffer::CreateInfo bufferCI;
			bufferCI.debugName = debugNames[static_cast<size_t>(type)];
			bufferCI.device = m_CI.device;
			bufferCI.usage = crossplatform::Buffer::UsageBit::SHADER_BINDING_TABLE_BIT | crossplatform::Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT;
			bufferCI.size = static_cast<uint32_t>(shaderBindingTableData[type].size());
			bufferCI.data = shaderBindingTableData[type].data();
			bufferCI.pAllocator = m_CI.pAllocator;
			sbt.second = crossplatform::Buffer::Create(&bufferCI);

			m_SDARs[type] = {
				GetBufferDeviceAddress(m_CI.device, sbt.second),
				static_cast<uint64_t>(shaderRecordMaxStride[type]),
				static_cast<uint64_t>(shaderBindingTableData[type].size())
			};
		}
	}
}