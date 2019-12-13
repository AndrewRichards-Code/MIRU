#pragma once
#include "../ErrorCodes.h"
#include "../FileParser/HelperFunctions.h"
#include "BuildSPV.h"

namespace miru
{
namespace shader_compiler
{
	const std::string GenerateGLSLShaderInputOutput(const ShaderVariable& sv, bool in, bool interface_in_out);
	const std::string GenerateGLSLShaderResource(const ShaderVariable& sv);
	const std::string GenerateGLSLFunctions(const ParsedShader & parsedShader, ErrorCode error = ErrorCode::MIRU_SC_OK);
	ErrorCode Compile_GLSL_SPV(const std::string& name, const std::string& source, const std::string& outputDir, const std::string& intDir, bool buildBinary, bool saveSrc);
}
}