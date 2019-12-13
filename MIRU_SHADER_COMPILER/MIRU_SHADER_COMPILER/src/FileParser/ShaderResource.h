#pragma once
#include "assert.h"
#include "Tokeniser.h"

namespace miru
{
namespace shader_compiler
{
	struct ShaderSubresource
	{
		Token::UsageBit type;
		std::string value;
	};
	struct ShaderResource
	{
		Token::UsageBit type;
		std::string value;
		uint32_t location;
		std::string semantic;
		std::vector<ShaderSubresource> subresources; //Only used for struct/class type.
	};

	struct PerStageShaderResource
	{
		std::string shader_type;
		std::vector<ShaderResource> vi;
		std::vector<ShaderResource> fo;
		std::vector<ShaderResource> io;
		std::vector<ShaderResource> ii;
		std::vector<ShaderResource> sr;
	};

	std::vector<PerStageShaderResource> GetPerStageShaderResource(const std::vector<Line>& lines);
}
}