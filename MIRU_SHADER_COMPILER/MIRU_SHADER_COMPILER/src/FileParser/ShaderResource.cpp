#include "ShaderResource.h"

using namespace miru;
using namespace shader_compiler;

std::vector<PerStageShaderResource> miru::shader_compiler::GetPerStageShaderResource(const std::vector<Line>& lines)
{
	std::vector<PerStageShaderResource> result;
	PerStageShaderResource pssr = {};

	int32_t scopeNumber = 0;
	bool shader_type = false;
	bool vi = false, fo = false, ii = false, io = false, sr = false;
	bool struct_class = false;

	for (auto& line : lines)
	{
		for (auto& token : line.tokens)
		{
			if (token.type == Token::UsageBit::COMBINED && token.tokens.size() == 2 && token.tokens[0].type == Token::UsageBit::HASH
				&& token.tokens[1].value == "shader_type")
			{
				shader_type = true;
				continue;
			}
			else if (shader_type)
			{
				pssr.shader_type = token.value; 
				shader_type = false;
				continue;
			}
			else if (token.type == Token::UsageBit::COMBINED && token.tokens.size() == 2 && token.tokens[0].type == Token::UsageBit::HASH)
			{
				//assert(scopeNumber, "Previous scope not correctly closed.");
				vi = token.tokens[1].value == "vertex_input";
				fo = token.tokens[1].value == "fragment_output";
				ii = token.tokens[1].value == "interface_in";
				io = token.tokens[1].value == "interface_out";
				sr = token.tokens[1].value == "shader_resources";

				if (token.tokens[1].value == "shader_end")
				{
					result.push_back(pssr);

					pssr.shader_type.clear();
					for (auto& temp : pssr.vi)
						temp.subresources.clear();
					for (auto& temp : pssr.fo)
						temp.subresources.clear();
					for (auto& temp : pssr.ii)
						temp.subresources.clear();
					for (auto& temp : pssr.io)
						temp.subresources.clear();
					for (auto& temp : pssr.sr)
						temp.subresources.clear();
					pssr.vi.clear();
					pssr.fo.clear();
					pssr.ii.clear();
					pssr.io.clear();
					pssr.sr.clear();
				}
			}
			else if ((vi || fo || ii || io || sr) && token.type == Token::UsageBit::SCOPEOPEN)
				scopeNumber++;
			else if ((vi || fo || ii || io || sr) && token.type == Token::UsageBit::SCOPECLOSE)
				scopeNumber--;
			else if ((vi || fo || ii || io || sr) && token.type == Token::UsageBit::COMBINED && token.tokens[0].type == Token::UsageBit::SCOPECLOSE)
			{
				scopeNumber--;
				if (struct_class)
					struct_class = false;
			}
			else if ((vi || fo || ii || io || sr) && scopeNumber > 0 && line.tokens.size() == 5)
			{
				//assert((token.tokens[0].type < Token::UsageBit::STRUCT || token.tokens[0].type > Token::UsageBit::IMAGECUBEARRAY), "Invalid shader resource type");
				if (vi)
					pssr.vi.push_back({ line.tokens[0].type, line.tokens[1].value, static_cast<uint32_t>(atoi(line.tokens[3].value.c_str())), line.tokens[4].value, {} });
				else if (fo)
					pssr.fo.push_back({ line.tokens[0].type, line.tokens[1].value, static_cast<uint32_t>(atoi(line.tokens[3].value.c_str())), line.tokens[4].value, {} });
				else if (ii)
					pssr.ii.push_back({ line.tokens[0].type, line.tokens[1].value, static_cast<uint32_t>(atoi(line.tokens[3].value.c_str())), line.tokens[4].value, {} });
				else if (io)
					pssr.io.push_back({ line.tokens[0].type, line.tokens[1].value, static_cast<uint32_t>(atoi(line.tokens[3].value.c_str())), line.tokens[4].value, {} });
				else if (sr)
					pssr.sr.push_back({ line.tokens[0].type, line.tokens[1].value, static_cast<uint32_t>(atoi(line.tokens[3].value.c_str())), line.tokens[4].value, {} });

				if (line.tokens[0].type == Token::UsageBit::STRUCT || line.tokens[0].type == Token::UsageBit::CLASS)
					struct_class = true;
				else
					struct_class = false;
				
				break;
			}
			else if ((vi || fo || ii || io || sr) && struct_class && line.tokens.size() == 2)
			{
				//assert((token.tokens[0].type < Token::UsageBit::STRUCT || token.tokens[0].type > Token::UsageBit::IMAGECUBEARRAY), "Invalid shader resource type");
				if (vi)
					pssr.vi.back().subresources.push_back({ line.tokens[0].type, line.tokens[1].value });
				else if (fo)
					pssr.fo.back().subresources.push_back({ line.tokens[0].type, line.tokens[1].value });
				else if (ii)
					pssr.ii.back().subresources.push_back({ line.tokens[0].type, line.tokens[1].value });
				else if (io)
					pssr.io.back().subresources.push_back({ line.tokens[0].type, line.tokens[1].value });
				else if (sr)
					pssr.sr.back().subresources.push_back({ line.tokens[0].type, line.tokens[1].value });
				
				break;
			}
			else
				continue;
		}
	}
	return result;
}
