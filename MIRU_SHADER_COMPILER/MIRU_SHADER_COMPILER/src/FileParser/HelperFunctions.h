#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>

namespace miru
{
namespace shader_compiler
{
	const std::string GetPreProcessorString(const std::string& _src, const std::string& _search_term);
	
	template<class _Ty>
	const _Ty GetPreProcessorValue(const std::string& _src, const std::string& _search_term);

	//0 == No Scope, 1 == '{' and -1 == '}' 
	int16_t HasScope(const std::string& _line);
	
	const std::string GetShaderExtension(const std::string& shader_type);
	
	size_t FindEndOfShader(const std::string& _src, size_t _offset = 0);
	
	size_t FindEndOfFile(const std::string& _src, size_t _offset = 0);
	
	const std::string GetFunctions(const std::string& _src);

	struct ShaderVariable
	{
	public:
		std::string type;
		std::string name;
		size_t location = 0;
		std::string usage;
		std::vector<ShaderVariable> nestedSV;
		uint16_t scopeNumber = 0;
		bool valid = false;
		bool hasTrailingParameters = false;

	private:
		inline void RemoveCharAtEnd(char _char, std::string& _line)
		{
			size_t pos = _line.find(_char);
			if (pos != std::string::npos)
				_line.erase(pos, 1);
		}

		inline bool CheckForListInVariable(std::string& _line)
		{
			if (_line.find(',') != std::string::npos)
			{
				return true;
			}
			else
				return false;
		}

		inline bool CheckForEndOfVariable(std::string& _line)
		{
			if (_line.find(';') != std::string::npos)
			{
				return true;
			}
			else
				return false;
		}

	public:
		ShaderVariable(const std::string& line, uint16_t _scopeNumber)
		{
			if (HasScope(line) != 0)
			{
				//TODO: Solve logic!
				/*bool nlc = line.find_first_not_of('\n') != std::string::npos;
				bool tc = line.find_first_not_of('\t') != std::string::npos;
				bool obc = line.find_first_not_of('{') != std::string::npos;
				bool ccc = line.find_first_not_of('}') != std::string::npos;
				bool cccc = line.find_first_not_of("};") != std::string::npos;
				if((nlc && tc) && (obc && ccc && cccc))*/
				return;
			}
			scopeNumber = _scopeNumber;

			std::string temp;
			std::stringstream ss(line);

			ss >> type;
			RemoveCharAtEnd(';', type);
			if (CheckForEndOfVariable(temp))
				return;

			ss >> name;
			valid = true;
			RemoveCharAtEnd(';', name);
			if (CheckForEndOfVariable(name))
				return;

			ss >> temp;
			hasTrailingParameters = temp == ":";
			if (!hasTrailingParameters)
				return;

			ss >> temp;
			RemoveCharAtEnd(',', temp);
			CheckForListInVariable(temp);
			location = stoi(temp);

			ss >> usage;
			RemoveCharAtEnd(';', usage);
			if (CheckForEndOfVariable(usage))
				return;
		}
	};
	const std::vector<ShaderVariable> ParseScopes(const std::string& _src, const std::string& _search_term);
	
	struct ParsedShader
	{
		std::string shader_type;
		std::string shader_ext;
		std::string entry_point;
		std::string includes;

		std::vector<ShaderVariable> vertex_input;
		std::vector<ShaderVariable> fragment_output;
		std::vector<ShaderVariable> interface_in;
		std::vector<ShaderVariable> interface_out;
		std::vector<ShaderVariable> shader_resources;

		std::string function_block;
	};
	const ParsedShader ParseShader(const std::string& _src);
}
}