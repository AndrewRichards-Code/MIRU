#include "Compile_HLSL_CSO.h"

using namespace miru;
using namespace shader_compiler;

const std::string miru::shader_compiler::GenerateHLSLShaderInputOutput(const std::vector<ShaderVariable>& svs, bool in, bool interface_in_out)
{
	if (svs.empty())
		return "";
	
	std::string structName = interface_in_out ? (in ? "INTERFACE_IN" : "INTERFACE_OUT") : (in ? "VERTEX_INPUT" : "FRAGMENT_OUT");
	std::string header = "struct " + structName + "\n{\n";
	std::string footer = "};\n";
	std::string body;

	for (auto& sv : svs)
	//TODO Change to HLSL type
		body += "\t" + sv.type + " " + sv.name + " : " + (sv.usage == "POSITION" ? "SV_": "") + sv.usage + ";\n";

	return header + body + footer;
}

const std::string miru::shader_compiler::GenerateHLSLShaderResource(const std::vector<ShaderVariable>& svs)
{
	if (svs.empty())
		return "";

	std::string result;
	for (auto& sv : svs)
	{
		std::string structName;
		for (auto it = sv.name.begin(); it != sv.name.end(); it++)
			structName += std::toupper(*it);

		std::string header = "struct " + structName + "\n{\n";
		std::string footer = "};\n";
		std::string body;

		body += header;
		for (auto& _sv : sv.nestedSV)
		//TODO Change to HLSL type
			body += "\t" + _sv.type + " " + _sv.name + ";\n";

		body += footer;

		if (sv.usage == "uniform")
		{
			body += "ConstantBuffer<" + structName + "> " + sv.name + " : register(b" + std::to_string(sv.location) + ");\n\n";
		}
		result += body;
	}
	return result;
}

const std::string miru::shader_compiler::GenerateHLSLFunctions(const ParsedShader& parsedShader, ErrorCode error)
{
	std::stringstream ss(parsedShader.function_block);
	std::string line, result;
	while (std::getline(ss, line, '\n'))
	{
		if (line == "")
			continue;

		size_t shader_resources_pos = 0;

		while (shader_resources_pos != std::string::npos)
		{
			shader_resources_pos = line.find("shader_resources");

			if (shader_resources_pos != std::string::npos)
			{
				line.replace(shader_resources_pos, line.find_first_of(".", shader_resources_pos) + 1 - shader_resources_pos, ""); continue;
			}

		}
		result += (line + "\n");
	}

	//Special Keyword replace
	bool vert = parsedShader.shader_type == "VERTEX";
	bool frag = parsedShader.shader_type == "FRAGEMENT" || parsedShader.shader_type == "PIXEL";
	size_t entryPointPos = result.find(parsedShader.entry_point + "(");
	if (entryPointPos != std::string::npos)
	{
		size_t voidPos = result.rfind("void", entryPointPos);
		result.erase(voidPos, result.find_first_of('\n', voidPos));
		std::string entryPointDeclaration = (frag ? "FRAGMENT_OUTPUT" : "INTERFACE_OUT") + std::string(" ") + parsedShader.entry_point + " (" + (vert ? "VERTEX_INPUT vertex_input" : "INTERFACE_IN interface_in") + ")";
		result.insert(voidPos, entryPointDeclaration);
	}
	else
	{
		error = ErrorCode::MIRU_SC_GLSL_ERROR;
		MIRU_SHADER_COMPILER_ERROR_CODE(error, "Entry Point Declaration is invalid.");
	}

	size_t openEntryPointScopePos = result.find_first_of('{');
	size_t closeEntryPointScopePos = result.find_first_of('}');
	if (openEntryPointScopePos != std::string::npos && closeEntryPointScopePos != std::string::npos)
	{
		result.insert(closeEntryPointScopePos, "\n\t" + std::string(frag ? "return fragment_output;" : "return  interface_out;") + "\n");
		result.insert(openEntryPointScopePos + 1, "\n\t" + std::string( frag ? "FRAGMENT_OUTPUT fragment_output;" : "INTERFACE_OUT interface_out;"));
	}
	else
	{
		error = ErrorCode::MIRU_SC_GLSL_ERROR;
		MIRU_SHADER_COMPILER_ERROR_CODE(error, "No scoper found for the Entry Point.");
	}

	return result;
}

ErrorCode miru::shader_compiler::Compile_HLSL_CSO(const std::string& name, const std::string& source, const std::string& outputDir, const std::string& intDir, bool buildBinary, bool saveSrc)
{
	if (source.find("#MIRU_SHADER_LANGUAGE") == std::string::npos)
		return ErrorCode::MIRU_SC_INVALID_SHADER_FILE;

	size_t eof = FindEndOfFile(source);
	size_t eos = 0;
	size_t prev_eos = 0;

	while (eof > (prev_eos + 1))
	{
		size_t eos = FindEndOfShader(source, prev_eos);
		const std::string& shader_source = source.substr(prev_eos, eos - prev_eos);
		prev_eos = eos;

		ParsedShader ps = ParseShader(shader_source);

		//Construct GLSL
		std::string hlsl_src;
		hlsl_src += "//shader model 6.0\n";
		hlsl_src += "\n//vertex_input\n";
		hlsl_src += GenerateHLSLShaderInputOutput(ps.vertex_input, true, false);

		hlsl_src += "\n//fragment_output\n";
		hlsl_src += GenerateHLSLShaderInputOutput(ps.fragment_output, false, false);

		hlsl_src += "\n//interface_in\n";
		hlsl_src += GenerateHLSLShaderInputOutput(ps.interface_in, true, true);

		hlsl_src += "\n//interface_out\n";
		hlsl_src += GenerateHLSLShaderInputOutput(ps.interface_out, false, true);

		hlsl_src += "\n//shader_resources\n";
		hlsl_src += GenerateHLSLShaderResource(ps.shader_resources);


		hlsl_src += "\n//functions\n";
		ErrorCode error = ErrorCode::MIRU_SC_OK;
		hlsl_src += GenerateHLSLFunctions(ps, error);
		if (error != ErrorCode::MIRU_SC_OK)
			MIRU_SHADER_COMPILER_RETURN_ERROR_CODE(error, "Error in generating GLSL functions for MIRU_SHADER_COMPILER.");

		std::cout << hlsl_src << std::endl;
		//Build Intermediate file
		std::ofstream file;
		file.open((intDir + name + ps.shader_ext + ".hlsl").c_str(), std::ios::out | std::ios::beg);
		if (file.is_open())
		{
			file << hlsl_src;
			file.close();
		}
		else
			MIRU_SHADER_COMPILER_RETURN_ERROR_CODE(ErrorCode::MIRU_SC_UNABLE_TO_SAVE_INT, "Unable to save intermediate file for MIRU_SHADER_COMPILER.");

		if (buildBinary)
		{
			std::string ammendedOutDir = outputDir;
			if (size_t pos = ammendedOutDir.find('/'))
				ammendedOutDir.erase(pos, 1);

			BuildCSO((intDir + name + ps.shader_ext).c_str(), outputDir, ps.entry_point);
		}

		if (!saveSrc)
		{
			if (std::remove((intDir + name + ps.shader_ext).c_str()) != 0)
				MIRU_SHADER_COMPILER_RETURN_ERROR_CODE(ErrorCode::MIRU_SC_UNABLE_TO_DELETE_INT, "Unable to delete intermediate file for MIRU_SHADER_COMPILER.");
		}
	}
	return ErrorCode::MIRU_SC_OK;
}
