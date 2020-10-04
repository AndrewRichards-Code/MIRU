#pragma once

namespace miru
{
namespace shader_compiler
{
	//Return values from the main function
	enum class ErrorCode : int
	{
		MIRU_SHADER_COMPILER_OK,
		MIRU_SHADER_COMPILER_ERROR,
		MIRU_SHADER_COMPILER_NO_ARGS,
		MIRU_SHADER_COMPILER_NO_OUTPUT_FILE_FORMAT,
		MIRU_SHADER_COMPILER_NO_SHADER_FILE,
		MIRU_SHADER_COMPILER_NO_SHADER_ENTRY_POINT,
		MIRU_SHADER_COMPILER_NO_SHADER_STAGE,
		MIRU_SHADER_COMPILER_CSO_ERROR,
		MIRU_SHADER_COMPILER_SPV_ERROR
	};

	inline std::string ErrorCodeStr(ErrorCode code)
	{
		switch (code)
		{
		default:
		case ErrorCode::MIRU_SHADER_COMPILER_OK:
			return "MIRU_SHADER_COMPILER_OK";
		case ErrorCode::MIRU_SHADER_COMPILER_ERROR:
			return "MIRU_SHADER_COMPILER_ERROR";
		case ErrorCode::MIRU_SHADER_COMPILER_NO_ARGS:
			return "MIRU_SHADER_COMPILER_NO_ARGS";
		case ErrorCode::MIRU_SHADER_COMPILER_NO_OUTPUT_FILE_FORMAT:
			return "MIRU_SHADER_COMPILER_NO_OUTPUT_FILE_FORMAT";
		case ErrorCode::MIRU_SHADER_COMPILER_NO_SHADER_FILE:
			return "MIRU_SHADER_COMPILER_NO_SHADER_FILE";
		case ErrorCode::MIRU_SHADER_COMPILER_NO_SHADER_ENTRY_POINT:
			return "MIRU_SHADER_COMPILER_NO_SHADER_ENTRY_POINT";
		case ErrorCode::MIRU_SHADER_COMPILER_NO_SHADER_STAGE:
			return "MIRU_SHADER_COMPILER_NO_SHADER_STAGE";
		case ErrorCode::MIRU_SHADER_COMPILER_CSO_ERROR:
			return "MIRU_SHADER_COMPILER_CSO_ERROR";
		case ErrorCode::MIRU_SHADER_COMPILER_SPV_ERROR:
			return "MIRU_SHADER_COMPILER_SPV_ERROR";
		}
	}

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
	#define MIRU_SHADER_COMPILER_PRINTF(s, ...) if(output) {printf((s), __VA_ARGS__);}

	//Log error code
	#define MIRU_SHADER_COMPILER_RETURN(x, y) {if(x != miru::shader_compiler::ErrorCode::MIRU_SHADER_COMPILER_OK) { printf("MIRU_SHADER_COMPILER_ASSERT: %s(%d): [%s] %s\n", __FILE__, __LINE__, ErrorCodeStr(x).c_str(), y); DEBUG_BREAK; } return static_cast<int>(x); } 
	#define MIRU_SHADER_COMPILER_ERROR_CODE(x, y) {if(x != miru::shader_compiler::ErrorCode::MIRU_SHADER_COMPILER_OK) { printf("MIRU_SHADER_COMPILER_ASSERT: %s(%d): [%s] %s\n", __FILE__, __LINE__, ErrorCodeStr(x).c_str(), y); DEBUG_BREAK; } } 
}
}