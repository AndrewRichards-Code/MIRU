#pragma once

#include <cstdint>
#include "ARC/src/Log.h"

namespace miru
{
namespace shader_compiler
{
	//Return values from the main function
	enum class ErrorCode : uint32_t
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

	static std::string ErrorCodeToString(int64_t code)
	{
		switch ((ErrorCode)code)
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

	static bool output = true;
}
}

//MIRU printf
#define MIRU_SHADER_COMPILER_PRINTF(fmt, ...) if(output) { ARC_PRINTF(fmt, __VA_ARGS__); }

//Log error code
inline arc::Log MiruShaderCompilerLog("MIRU_SHADER_COMPILER");
#ifdef ARC_LOG_INSTANCE
#undef ARC_LOG_INSTANCE
#define ARC_LOG_INSTANCE MiruShaderCompilerLog
#endif
#define MIRU_SHDAER_COMPILER_SET_ERROR_CODE_TO_STRING_FUNCTION MiruShaderCompilerLog.SetErrorCodeToStringFunction(ErrorCodeToString)

#define MIRU_SHADER_COMPILER_RETURN(x, y) if((x) != miru::shader_compiler::ErrorCode::MIRU_SHADER_COMPILER_OK) { ARC_FATAL(static_cast<int64_t>(x), "%s", y); ARC_ASSERT(false); } return static_cast<int>(x)
#define MIRU_SHADER_COMPILER_ERROR_CODE(x, y) if((x) != miru::shader_compiler::ErrorCode::MIRU_SHADER_COMPILER_OK) { ARC_FATAL(static_cast<int64_t>(x), "%s", y); ARC_ASSERT(false); }