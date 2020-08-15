//MIRU_PRE_COMPILED HEADER
#include "miru_core_common.h"

using namespace miru;

//MIRU CPU Heap Allocation tracker
#if defined(MIRU_CPU_HEAP_ALLOCATION_TRACKER)
size_t CPU_HEAP_ALLOC_TRACKER::current_allocated_bytes = 0;
//std::map<void*, size_t> CPU_HEAP_ALLOC_TRACKER::current_allocation_map = {};

void* operator new(size_t size)
{
	void* ptr = (void*)malloc(size);
	CPU_HEAP_ALLOC_TRACKER::current_allocated_bytes += size;
	//CPU_HEAP_ALLOC_TRACKER::current_allocation_map[ptr] = size;
	return ptr;
}
void operator delete(void* ptr, size_t size)
{
	CPU_HEAP_ALLOC_TRACKER::current_allocated_bytes -= size;
	//CPU_HEAP_ALLOC_TRACKER::current_allocation_map.erase(ptr);
	free(ptr);
}
#endif

//MIRU CPU Profiler
#if defined(MIRU_CPU_PROFILER)
uint64_t Timer::m_ScopeCount = 0;
std::vector<Timer::ProfileDatum> Timer::m_ProfileData = {};
std::fstream Timer::m_File = {};

Timer::Timer(const std::string& name)
	:m_Name(name)
{
	m_StartTP = std::chrono::high_resolution_clock::now();
	m_ScopeCount++;
}

Timer::~Timer()
{
	m_ScopeCount--;
	if (!m_Stopped)
		Timer::Stop();
}

void Timer::Stop()
{
	m_EndTP = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds duration = 
		  std::chrono::time_point_cast<std::chrono::nanoseconds>(m_EndTP).time_since_epoch() 
		- std::chrono::time_point_cast<std::chrono::nanoseconds>(m_StartTP).time_since_epoch();
	m_Stopped = true;

	if (m_File.is_open())
	{
		for (uint32_t i = 0; i < m_ScopeCount; i++)
			m_File << "\t";

		m_File << "Scope " << m_ScopeCount << " : " << m_Name << " : " << duration.count() << "ns. (" << (double)duration.count() / 1e9 << "s" << ")\n";
	}
}

void Timer::BeginSession(const std::string& filepath)
{
	EndSession();

	m_File = std::fstream(filepath, std::ios::out | std::ios::trunc);
	if (!m_File.is_open())
		MIRU_WARN(true, "WARN: Unable to open file for profiling.");
}
void Timer::EndSession()
{
	if (m_File.is_open())
		m_File.close();
}
#endif