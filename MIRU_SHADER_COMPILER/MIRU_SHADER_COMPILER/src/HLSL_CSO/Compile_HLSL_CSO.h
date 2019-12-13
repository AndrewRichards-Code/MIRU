#pragma once
#include "../ErrorCodes.h"
#include "../FileParser/HelperFunctions.h"
#include "BuildCSO.h"

namespace miru
{
namespace shader_compiler
{
	const std::string GenerateHLSLShaderInputOutput(const std::vector<ShaderVariable>& svs, bool in, bool interface_in_out);
	const std::string GenerateHLSLShaderResource(const std::vector<ShaderVariable>& svs);
	const std::string GenerateHLSLFunctions(const ParsedShader& parsedShader, ErrorCode error = ErrorCode::MIRU_SC_OK);
	ErrorCode Compile_HLSL_CSO(const std::string& name, const std::string& source, const std::string& outputDir, const std::string& intDir, bool buildBinary, bool saveSrc);
}
}