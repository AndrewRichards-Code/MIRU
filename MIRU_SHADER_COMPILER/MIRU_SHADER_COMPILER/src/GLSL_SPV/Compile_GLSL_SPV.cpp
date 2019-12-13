#include "Compile_GLSL_SPV.h"

using namespace miru;
using namespace shader_compiler;



const std::string miru::shader_compiler::GenerateGLSLShaderInputOutput(const ShaderVariable& sv, bool in, bool interface_in_out)
{
	std::string layout = "layout(";
	std::string location = "location = " + std::to_string(sv.location) + ") ";
	std::string in_out = in ? "in " : "out ";
	std::string interface = interface_in_out ? "v_" : "";

	std::string result = layout + location + in_out + sv.type + " " + interface + sv.name + ";\n";
	return result;
}

const std::string miru::shader_compiler::GenerateGLSLShaderResource(const ShaderVariable& sv)
{
	std::string layout = "layout(";
	std::string layout_std = sv.usage == "uniform" ? "std140, " : sv.usage == "buffer" ? "std430, " : "";
	std::string binding = "binding = " + std::to_string(sv.location) + ") ";

	std::string result = layout + layout_std + binding + sv.usage + " " + sv.name + "\n";

	if (!sv.nestedSV.empty())
	{
		result += "{\n";
		for (const auto& _sv : sv.nestedSV)
			result += "\t" + _sv.type + " u_" + _sv.name + ";\n";

		result += "};\n";
	}
	return result;
}

const std::string miru::shader_compiler::GenerateGLSLFunctions(const ParsedShader & parsedShader, ErrorCode error)
{
	std::stringstream ss(parsedShader.function_block);
	std::string line, result;
	while (std::getline(ss, line, '\n'))
	{
		if (line == "")
			continue;

		size_t vertex_input_pos = 0;
		size_t fragment_output_pos = 0;
		size_t interface_in_pos = 0;
		size_t interface_out_pos = 0;
		size_t shader_resources_pos = 0;

		while (
			vertex_input_pos != std::string::npos ||
			fragment_output_pos != std::string::npos ||
			interface_in_pos != std::string::npos ||
			interface_out_pos != std::string::npos ||
			shader_resources_pos != std::string::npos
			)
		{
			vertex_input_pos = line.find("vertex_input");
			fragment_output_pos = line.find("fragment_output");
			interface_in_pos = line.find("interface_in");
			interface_out_pos = line.find("interface_out");
			shader_resources_pos = line.find("shader_resources");

			if (vertex_input_pos != std::string::npos)
			{
				line.replace(vertex_input_pos, line.rfind('.', line.find_first_of(' ', vertex_input_pos)) + 1 - vertex_input_pos, ""); continue;
			}
			if (fragment_output_pos != std::string::npos)
			{
				line.replace(fragment_output_pos, line.rfind('.', line.find_first_of(' ', fragment_output_pos)) + 1 - fragment_output_pos, ""); continue;
			}
			if (interface_in_pos != std::string::npos)
			{
				line.replace(interface_in_pos, line.rfind('.', line.find_first_of(' ', interface_in_pos)) + 1 - interface_in_pos, "v_"); continue;
			}
			if (interface_out_pos != std::string::npos)
			{
				line.replace(interface_out_pos, line.rfind('.', line.find_first_of(' ', interface_out_pos)) + 1 - interface_out_pos, "v_"); continue;
			}
			if (shader_resources_pos != std::string::npos)
			{
				line.replace(shader_resources_pos, line.rfind('.', line.find_first_of(' ', shader_resources_pos)) + 1 - shader_resources_pos, "u_"); continue;
			}

		}
		result += (line + "\n");
	}

	//Special Keyword replaces
	std::string v_pos_str;
	for (auto& sv_io : parsedShader.interface_out)
	{
		if (sv_io.usage == "POSITION")
		{
			v_pos_str = "v_" + sv_io.name;
			break;
		}
	}

	if (!v_pos_str.empty())
	{
		size_t v_pos = result.find(v_pos_str);
		bool IsAssigned = false;
		if (v_pos != std::string::npos)
		{
			for (size_t i = v_pos + v_pos_str.size(); i < v_pos + v_pos_str.size() + 2; i++)
			{
				if (result[i] == '=')
				{
					IsAssigned = true; 
					break;
				}
			}
		}
		if (IsAssigned)
			result.replace(v_pos, v_pos_str.size(), "gl_Position");
		else
		{
			error = ErrorCode::MIRU_SC_GLSL_ERROR;
			MIRU_SHADER_COMPILER_ERROR_CODE(error, "The shader variable from #interface_out with usage \"POSITION\" was not found, or was not initially assigned a value.");
		}
	}

	return result;
}

ErrorCode miru::shader_compiler::Compile_GLSL_SPV(const std::string& name, const std::string& source, const std::string& outputDir, const std::string& intDir, bool buildBinary, bool saveSrc)
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
		std::string glsl_src;
		glsl_src += "#version 450 core\n";
		glsl_src += "\n//vertex_input\n";
		for (const auto& vi : ps.vertex_input)
			glsl_src += GenerateGLSLShaderInputOutput(vi, true, false);

		glsl_src += "\n//fragment_output\n";
		for (const auto& fo : ps.fragment_output)
			glsl_src += GenerateGLSLShaderInputOutput(fo, false, false);

		glsl_src += "\n//interface_in\n";
		for (const auto& ii : ps.interface_in)
			glsl_src += GenerateGLSLShaderInputOutput(ii, true, true);

		glsl_src += "\n//interface_out\n";
		for (const auto& io : ps.interface_out)
			glsl_src += GenerateGLSLShaderInputOutput(io, false, true);

		glsl_src += "\n//shader_resources\n";
		for (const auto& sr : ps.shader_resources)
			glsl_src += GenerateGLSLShaderResource(sr);

		glsl_src += "\n//functions\n";
		ErrorCode error = ErrorCode::MIRU_SC_OK;
		glsl_src += GenerateGLSLFunctions(ps, error);
		if(error != ErrorCode::MIRU_SC_OK)
			MIRU_SHADER_COMPILER_RETURN_ERROR_CODE(error, "Error in generating GLSL functions for MIRU_SHADER_COMPILER.");

		//Build Intermediate file
		std::ofstream file;
		file.open((intDir + name + ps.shader_ext).c_str(), std::ios::out | std::ios::beg);
		if (file.is_open())
		{
			file << glsl_src;
			file.close();
		}
		else
			MIRU_SHADER_COMPILER_RETURN_ERROR_CODE(ErrorCode::MIRU_SC_UNABLE_TO_SAVE_INT, "Unable to save intermediate file for MIRU_SHADER_COMPILER.");

		if (buildBinary)
		{
			std::string ammendedOutDir = outputDir;
			if (size_t pos = ammendedOutDir.find('/'))
				ammendedOutDir.erase(pos, 1);

			BuildSPV((intDir + name + ps.shader_ext).c_str(), outputDir);
		}

		if (!saveSrc)
		{
			if(std::remove((intDir + name + ps.shader_ext).c_str()) != 0)
				MIRU_SHADER_COMPILER_RETURN_ERROR_CODE(ErrorCode::MIRU_SC_UNABLE_TO_DELETE_INT, "Unable to delete intermediate file for MIRU_SHADER_COMPILER.");
		}
	}
	
	return ErrorCode::MIRU_SC_OK;
}
