#pragma once

namespace miru
{
namespace shader_compiler
{
	//Return values from the main function
	enum class ErrorCode : int
	{
		MIRU_SC_OK = 0,
		MIRU_SC_ERROR = 1,
		MIRU_SC_NO_ARGS = 2,
		MIRU_SC_NO_OUTPUT_FILE_FORMAT = 3,
		MIRU_SC_NO_SHADER_FILE = 4,
		MIRU_SC_CSO_ERROR = 5,
		MIRU_SC_SPV_ERROR = 6,
	};

	//Debugbreak and assert
	#ifdef _DEBUG
	#if defined(_MSC_VER)
	#define DEBUG_BREAK __debugbreak()
	#else
	#define DEBUG_BREAK raise(SIGTRAP)
	#endif
	#else
	#define DEBUG_BREAK
	#endif

	static bool output = true;

	//MIRU printf
	#if _DEBUG
	#define MIRU_SC_PRINTF(s, ...) if(output) {printf((s), __VA_ARGS__);}
	#else
	#define MIRU_SC_PRINTF(s, ...) if(output) {printf("");}
	#endif

	//Log error code
	#define MIRU_SHADER_COMPILER_RETURN(x, y) {if(x != miru::shader_compiler::ErrorCode::MIRU_SC_OK) { printf("MIRU_SHADER_COMPILER_ASSERT: %s(%d): %s", __FILE__, __LINE__, y); DEBUG_BREAK; } return static_cast<int>(x); } 
	#define MIRU_SHADER_COMPILER_ERROR_CODE(x, y) {if(x != miru::shader_compiler::ErrorCode::MIRU_SC_OK) { printf("MIRU_SHADER_COMPILER_WARN: %s(%d): %s", __FILE__, __LINE__, y); DEBUG_BREAK; } } 
}
}