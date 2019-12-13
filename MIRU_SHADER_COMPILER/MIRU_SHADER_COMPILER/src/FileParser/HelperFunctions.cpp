#include "HelperFunctions.h"

using namespace miru;
using namespace shader_compiler;

const std::string miru::shader_compiler::GetPreProcessorString(const std::string& _src, const std::string& _search_term)
{
	size_t _search_term_start_pos = _src.find(_search_term);
	size_t _val_start_pos = _src.find_first_not_of(' ', _search_term_start_pos + _search_term.size());
	size_t _val_end_pos = _src.find_first_of('\n', _val_start_pos);
	const std::string _val_str = _src.substr(_val_start_pos, _val_end_pos - _val_start_pos);

	return _val_str;
}

template<class _Ty>
const _Ty miru::shader_compiler::GetPreProcessorValue(const std::string& _src, const std::string& _search_term)
{
	size_t _search_term_start_pos = _src.find(_search_term);
	size_t _val_start_pos = _src.find_first_not_of(' ', _search_term_start_pos + _search_term.size());
	size_t _val_end_pos = _src.find_first_of('\n', _val_start_pos);
	const std::string _val_str = _src.substr(_val_start_pos, _val_end_pos - _val_start_pos);

	return (_Ty)(stoi(_val_str));
}

int16_t miru::shader_compiler::HasScope(const std::string& _line)
{
	if (_line.find('{') != std::string::npos)
		return 1;
	if (_line.find('}') != std::string::npos)
		return -1;

	return 0;
}

const std::string miru::shader_compiler::GetShaderExtension(const std::string& shader_type)
{
	if (!_stricmp(shader_type.c_str(), "VERTEX")) return ".vert";
	if (!_stricmp(shader_type.c_str(), "TESSELLATION_CONTROL")) return ".tesc";
	if (!_stricmp(shader_type.c_str(), "TESSELLATION_EVALUATION")) return ".tese";
	if (!_stricmp(shader_type.c_str(), "GEOMETRY")) return ".geom";
	if (!_stricmp(shader_type.c_str(), "FRAGMENT")) return ".frag";
	if (!_stricmp(shader_type.c_str(), "COMPUTE")) return ".comp";
	if (!_stricmp(shader_type.c_str(), "HULL")) return ".tesc";
	if (!_stricmp(shader_type.c_str(), "DOMAIN")) return ".tese";
	if (!_stricmp(shader_type.c_str(), "PIXEL")) return ".frag";
	else
		return "";
}

size_t miru::shader_compiler::FindEndOfShader(const std::string& _src, size_t _offset)
{
	std::string _search_term = "#shader_end";
	size_t _search_term_start_pos = _src.find(_search_term, _offset);
	size_t _search_term_end_pos = _src.find_first_of('\n', _search_term_start_pos);

	return _search_term_end_pos;
}

size_t miru::shader_compiler::FindEndOfFile(const std::string& _src, size_t _offset)
{
	std::string _search_term = "#END_OF_FILE";
	size_t _search_term_start_pos = _src.find(_search_term, _offset);

	return _search_term_start_pos;
}

const std::vector<ShaderVariable> miru::shader_compiler::ParseScopes(const std::string& _src, const std::string& _search_term)
{
	size_t _search_term_start_pos = _src.find(_search_term);
	if (_search_term_start_pos == std::string::npos)
		return std::vector<ShaderVariable>();
	
	size_t _val_start_pos = _src.find_first_not_of(' ', _search_term_start_pos + _search_term.size() + 1);
	size_t _val_end_pos = _src.length();
	const std::string _val_str = _src.substr(_val_start_pos, _val_end_pos - _val_start_pos);

	uint16_t scopeNumber = 0;

	std::string line;
	std::stringstream ss(_val_str);
	std::vector<ShaderVariable> svs;

	while (std::getline(ss, line, '\n'))
	{
		if (line == "")
			continue;
		
		scopeNumber += HasScope(line);
		ShaderVariable sv(line, scopeNumber);

		if (!svs.empty())
		{
			if (svs.back().scopeNumber < scopeNumber && sv.valid)
			{
				svs.back().nestedSV.push_back(sv);
				continue;
			}
		}

		if (sv.valid)
			svs.push_back(sv);
		
		if (scopeNumber == 0)
			break;
	}

	return svs;
}

const std::string miru::shader_compiler::GetFunctions(const std::string& _src)
{
	std::string _search_term = "#functions";
	size_t _search_term_start_pos = _src.find(_search_term);
	size_t _val_start_pos = _src.find_first_not_of(' ', _search_term_start_pos + _search_term.size() + 1);
	size_t _val_end_pos = _src.length();
	const std::string _val_str = _src.substr(_val_start_pos, _val_end_pos - _val_start_pos);

	uint16_t scopeNumber = 0;

	std::string line, functionsBlock;
	std::stringstream ss(_val_str);

	while (std::getline(ss, line, '\n'))
	{
		if (line == "")
			continue;
		
		scopeNumber += HasScope(line);
		functionsBlock += (line + "\n");

		if (line.find(GetPreProcessorString(_src, "#entry_point")) != std::string::npos)
			continue;

		if (scopeNumber == 0)
			break;
	}

	return functionsBlock;
}

const ParsedShader miru::shader_compiler::ParseShader(const std::string& _src)
{
	ParsedShader ps;

	ps.shader_type = GetPreProcessorString(_src, "#shader_type");
	ps.shader_ext = GetShaderExtension(ps.shader_type);
	ps.entry_point = GetPreProcessorString(_src, "#entry_point");
	ps.includes = GetPreProcessorString(_src, "#includes");

	if (ps.shader_type == "VERTEX")
		ps.vertex_input = ParseScopes(_src, "#vertex_input");
	if (ps.shader_type == "FRAGMENT")
		ps.fragment_output = ParseScopes(_src, "#fragment_output");
	
	ps.shader_resources = ParseScopes(_src, "#shader_resources");
	ps.interface_in = ParseScopes(_src, "#interface_in");
	ps.interface_out = ParseScopes(_src, "#interface_out");

	ps.function_block = GetFunctions(_src);

	return ps;
}
