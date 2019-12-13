#include "ErrorCodes.h"
#include "MSCDocumentation.h"
#include "GLSL_SPV/Compile_GLSL_SPV.h"
#include "HLSL_CSO/Compile_HLSL_CSO.h"
#include "FileParser/Tokeniser.h"
#include "FileParser/ShaderResource.h"

using namespace miru;
using namespace shader_compiler;

int main(int argc, const char** argv)
{
	ErrorCode error = ErrorCode::MIRU_SC_OK;

	//Null arguments
	if (!argc)
	{
		error = ErrorCode::MIRU_SC_NO_ARGS;
		MIRU_SHADER_COMPILER_RETURN(error, "No arguements passed to MIRU_SHADER_COMPILER.");
	}

	//Help documentation
	for (int i = 0; i < argc; i++)
	{
		if (!_stricmp(argv[i], "-h") || !_stricmp(argv[i], "-help"))
		{
			std::cout << help_doucumentation << std::endl;
			break;
		}
	}

	//Parse other command line arguements
	bool hlsl = false;
	bool glsl = false;
	bool hlsl_bin = true;
	bool glsl_bin = true;
	bool hlsl_src = true;
	bool glsl_src = true;
	bool vi = false;
	bool sr = false;
	bool fo = false;
	for(int i = 0; i < argc; i++)
	{
		if (!_stricmp(argv[i], "-hlsl"))
		{
			hlsl = true;
			continue;
		}
		if (!_stricmp(argv[i], "-glsl"))
		{
			glsl = true;
			continue;
		}
		if (!_stricmp(argv[i], "-hlsl_no_bin"))
		{
			hlsl_bin = false;
			continue;
		}
		if (!_stricmp(argv[i], "-glsl_no_bin"))
		{
			glsl_bin = false;
			continue;
		}
		if (!_stricmp(argv[i], "-hlsl_no_src"))
		{
			hlsl_src = false;
			continue;
		}
		if (!_stricmp(argv[i], "-glsl_no_src"))
		{
			glsl_src = false;
			continue;
		}
		if (!_stricmp(argv[i], "-vi"))
		{
			vi = true;
			continue;
		}
		if (!_stricmp(argv[i], "-sr"))
		{
			sr = true;
			continue;
		}
		if (!_stricmp(argv[i], "-fo") || !_stricmp(argv[i], "-po"))
		{
			fo = true;
			continue;
		}
	}
	if (!hlsl && !glsl)
	{
		error = ErrorCode::MIRU_SC_NO_OUTPUT_FILE_FORMAT;
		MIRU_SHADER_COMPILER_RETURN(error, "No output file formats passed to MIRU_SHADER_COMPILER.");
	}

	//Get Filepath and Directories
	std::string name, filepath, outputDir, intDir;
	const size_t tagSize = std::string("-X:").size();
	for (int i = 0; i < argc; i++)
	{
		std::string tempFilepath = argv[i];
		if (tempFilepath.find(".msl") != std::string::npos && (tempFilepath.find("-f:") != std::string::npos || tempFilepath.find("-F:") != std::string::npos))
		{
			tempFilepath.erase(0, tagSize);
			filepath = tempFilepath;

			size_t start = filepath.find_last_of('/') + 1;
			size_t end = filepath.find_first_of('.');
			name = filepath.substr(start, end - start);
		}
		if (tempFilepath.find("-o:") != std::string::npos || tempFilepath.find("-O:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			outputDir = tempFilepath;
		}
		if (tempFilepath.find("-i:") != std::string::npos || tempFilepath.find("-I:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			intDir = tempFilepath;
		}
	}

	//Get msf source
	std::string  msf_src_str;
	std::ifstream file(filepath, std::ios::ate | std::ios::in);
	if(!file.is_open())
	{
		error = ErrorCode::MIRU_SC_NO_SHADER_FILE;
		MIRU_SHADER_COMPILER_RETURN(error, "No shader has been passed to MIRU_SHADER_COMPILER.");
	}
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char>msf_src(fileSize);
	file.seekg(0);
	file.read(msf_src.data(), fileSize);
	file.close();

	msf_src_str = msf_src.data(); //Convert to std::string and add '\0'
	msf_src_str += '\0';

	auto result = TokeniseCode(msf_src_str);
	bool scopes = CheckScopes(result);

	auto pssrs = GetPerStageShaderResource(result);

	//Compile and build binary
	if (hlsl)
	{
		error = Compile_HLSL_CSO(name, msf_src_str, outputDir, intDir, glsl_bin, glsl_src);
	}
	else if (glsl)
	{
		error = Compile_GLSL_SPV(name, msf_src_str, outputDir, intDir, glsl_bin, glsl_src);
	}
	else
	{
		error = ErrorCode::MIRU_SC_ERROR;
		MIRU_SHADER_COMPILER_RETURN(error, "Unknown error occured in MIRU_SHADER_COMPILER.");
	}
	MIRU_SHADER_COMPILER_RETURN(error, "Success! MIRU_SHADER_COMPILER.");
}