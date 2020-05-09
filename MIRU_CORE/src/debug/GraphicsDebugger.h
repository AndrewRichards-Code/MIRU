#pragma once

namespace miru
{
namespace debug
{
	class GraphicsDebugger
	{
	public:
		enum class DebuggerType : uint32_t
		{
			UNKNOWN = 0,
			PIX,
			RENDER_DOC
		};

	public:
		~GraphicsDebugger() = default;

		const DebuggerType& GetDebuggerType() { return m_Debugger; }

	protected:
		DebuggerType m_Debugger = DebuggerType::UNKNOWN;
	};
}
}
