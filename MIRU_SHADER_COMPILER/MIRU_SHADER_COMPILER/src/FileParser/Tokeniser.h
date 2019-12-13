#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace miru
{
namespace shader_compiler
{
	struct Token
	{
		enum class UsageBit
		{
			UNKNOWN,
			COMBINED,
			NAME,
			STRUCT,
			CLASS,
			VOID,
			BOOL,
			INT,
			UINT,
			FLOAT,
			DOUBLE,

			VEC2,
			VEC3,
			VEC4,
			DVEC2,
			DVEC3,
			DVEC4,
			BVEC2,
			BVEC3,
			BVEC4,
			IVEC2,
			IVEC3,
			IVEC4,
			UVEC2,
			UVEC3,
			UVEC4,

			MAT2X2,
			MAT2X3,
			MAT2X4,
			MAT3X2,
			MAT3X3,
			MAT3X4,
			MAT4X2,
			MAT4X3,
			MAT4X4,
			DMAT2X2,
			DMAT2X3,
			DMAT2X4,
			DMAT3X2,
			DMAT3X3,
			DMAT3X4,
			DMAT4X2,
			DMAT4X3,
			DMAT4X4,

			TEXTURE1D,
			TEXTURE2D,
			TEXTURE3D,
			TEXTURECUBE,
			TEXTURE1DARRAY,
			TEXTURE2DARRAY,
			TEXTURE2DMS,
			TEXTURE2DMSARRAY,
			TEXTURECUBEARRAY,

			SAMPLER,

			SAMPLER1D,
			SAMPLER2D,
			SAMPLER3D,
			SAMPLERCUBE,
			SAMPLER1DARRAY,
			SAMPLER2DARRAY,
			SAMPLER2DMS,
			SAMPLER2DMSARRAY,
			SAMPLERCUBEARRAY,

			IMAGE1D,
			IMAGE2D,
			IMAGE3D,
			IMAGECUBE,
			IMAGE1DARRAY,
			IMAGE2DARRAY,
			IMAGE2DMS,
			IMAGE2DMSARRAY,
			IMAGECUBEARRAY,

			HASH,
			BRACKETOPEN,
			BRACKETCLOSE,
			SCOPEOPEN,
			SCOPECLOSE,
			FULLSTOP,
			COMMA,
			SEMICOLON,
			COLON,
			SINGLEQUOTE,
			DOUBLEQUOTE,
			RETURN,
			PLUS,
			MINUS,
			MULTIPLE,
			DIVIDE,
			ASSIGN,
			ZERO,
			ONE,
			TWO,
			THREE,
			FOUR,
			FIVE,
			SIX,
			SEVEN,
			EIGHT,
			NINE,
		};

		UsageBit type;
		std::string value;
		std::vector<Token> tokens;

		Token()
			:type(UsageBit::UNKNOWN), value(""), tokens({}) {};
	};

	struct Line
	{
		uint32_t number;
		std::string value;
		std::vector<Token> tokens;
		Line()
			: number(0), value(""), tokens({}) {}
	};

	struct Type_GLSL_HLSL
	{
		Token::UsageBit type;
		std::string glsl;
		std::string hlsl;
	};

	const std::unordered_map<std::string, Type_GLSL_HLSL> typemap = {
	{"struct", {Token::UsageBit::STRUCT, "struct", "struct"}},
	{"class", {Token::UsageBit::CLASS, "class", "class"}},
	{"void", {Token::UsageBit::VOID, "void", "void"}},
	{"bool", {Token::UsageBit::BOOL, "bool", "bool"}},
	{"int", {Token::UsageBit::INT, "int", "int"}},
	{"uint", {Token::UsageBit::UINT, "uint", "uint"}},
	{"float", {Token::UsageBit::FLOAT, "float", "float"}},
	{"double", {Token::UsageBit::DOUBLE, "double", "double"}},

	{"vec2", {Token::UsageBit::VEC2, "vec2", "float2"}},
	{"vec3", {Token::UsageBit::VEC3, "vec3", "float3"}},
	{"vec4", {Token::UsageBit::VEC4, "vec4", "float4"}},
	{"dvec2", {Token::UsageBit::DVEC2, "dvec2", "double2"}},
	{"dvec3", {Token::UsageBit::DVEC3, "dvec3", "double3"}},
	{"dvec4", {Token::UsageBit::DVEC4, "dvec4", "double4"}},
	{"bvec2", {Token::UsageBit::BVEC2, "bvec2", "bool2"}},
	{"bvec3", {Token::UsageBit::BVEC3, "bvec3", "bool3"}},
	{"bvec4", {Token::UsageBit::BVEC4, "bvec4", "bool4"}},
	{"ivec2", {Token::UsageBit::IVEC2, "ivec2", "int2"}},
	{"ivec3", {Token::UsageBit::IVEC3, "ivec3", "int3"}},
	{"ivec4", {Token::UsageBit::IVEC4, "ivec4", "int4"}},
	{"uvec2", {Token::UsageBit::UVEC2, "uvec2", "uint2"}},
	{"uvec3", {Token::UsageBit::UVEC3, "uvec3", "uint3"}},
	{"uvec4", {Token::UsageBit::UVEC4, "uvec4", "uint4"}},

	{"mat2x2", {Token::UsageBit::MAT2X2, "mat2x2", "float2x2"}},
	{"mat2x3", {Token::UsageBit::MAT2X3, "mat2x3", "float2x3"}},
	{"mat2x4", {Token::UsageBit::MAT2X4, "mat2x4", "float2x4"}},
	{"mat3x2", {Token::UsageBit::MAT3X2, "mat3x2", "float3x2"}},
	{"mat3x3", {Token::UsageBit::MAT3X3, "mat3x3", "float3x3"}},
	{"mat3x4", {Token::UsageBit::MAT3X4, "mat3x4", "float3x4"}},
	{"mat4x2", {Token::UsageBit::MAT4X2, "mat4x2", "float4x2"}},
	{"mat4x3", {Token::UsageBit::MAT4X3, "mat4x3", "float4x3"}},
	{"mat4x4", {Token::UsageBit::MAT4X4, "mat4x4", "float4x4"}},
	{"mat2", {Token::UsageBit::MAT2X2, "mat2", "float2x2"}},
	{"mat3", {Token::UsageBit::MAT3X3, "mat3", "float3x3"}},
	{"mat4", {Token::UsageBit::MAT4X4, "mat4", "float4x4"}},
	{"dmat2x2", {Token::UsageBit::DMAT2X2, "dmat2x2", "double2x2"}},
	{"dmat2x3", {Token::UsageBit::DMAT2X3, "dmat2x3", "double2x3"}},
	{"dmat2x4", {Token::UsageBit::DMAT2X4, "dmat2x4", "double2x4"}},
	{"dmat3x2", {Token::UsageBit::DMAT3X2, "dmat3x2", "double3x2"}},
	{"dmat3x3", {Token::UsageBit::DMAT3X3, "dmat3x3", "double3x3"}},
	{"dmat3x4", {Token::UsageBit::DMAT3X4, "dmat3x4", "double3x4"}},
	{"dmat4x2", {Token::UsageBit::DMAT4X2, "dmat4x2", "double4x2"}},
	{"dmat4x3", {Token::UsageBit::DMAT4X3, "dmat4x3", "double4x3"}},
	{"dmat4x4", {Token::UsageBit::DMAT4X4, "dmat4x4", "double4x4"}},
	{"dmat2", {Token::UsageBit::DMAT2X2, "dmat2x2", "double2x2"}},
	{"dmat3", {Token::UsageBit::DMAT3X3, "dmat3x3", "double3x3"}},
	{"dmat4", {Token::UsageBit::DMAT4X4, "dmat4x4", "double4x4"}},

	{"texture1D", {Token::UsageBit::TEXTURE1D, "texture1D", "Texture1D"}},
	{"texture2D", {Token::UsageBit::TEXTURE2D, "texture2D", "Texture2D"}},
	{"texture3D", {Token::UsageBit::TEXTURE3D, "texture3D", "Texture3D"}},
	{"textureCube", {Token::UsageBit::TEXTURECUBE, "textureCube", "TextureCube"}},
	{"texture1DArray", {Token::UsageBit::TEXTURE1DARRAY, "texture1DArray", "Texture1DArray"}},
	{"texture2DArray", {Token::UsageBit::TEXTURE2DARRAY, "texture2DArray", "Texture2DArray"}},
	{"texture2DMS", {Token::UsageBit::TEXTURE2DMS, "texture2DMS", "Texture2DMS"}},
	{"texture2DMSArray", {Token::UsageBit::TEXTURE2DARRAY, "texture2DMSArray", "Texture2DMSArray"}},
	{"textureCubeArray", {Token::UsageBit::TEXTURECUBEARRAY, "textureCubeArray", "TextureCubeArray"}},

	{"sampler", {Token::UsageBit::SAMPLER , "sampler", "SamplerState"}},

	{"sampler1D", {Token::UsageBit::SAMPLER1D, "sampler1D", "sampler1D"}},
	{"sampler2D", {Token::UsageBit::SAMPLER2D, "sampler2D", "sampler2D"}},
	{"sampler3D", {Token::UsageBit::SAMPLER3D, "sampler3D", "sampler3D"}},
	{"samplerCube", {Token::UsageBit::SAMPLERCUBE, "samplerCube", "samplerCUBE"}},
	{"sampler1DArray", {Token::UsageBit::SAMPLER1DARRAY, "sampler1DArray", "sampler2D"}},
	{"sampler2DArray", {Token::UsageBit::SAMPLER2DARRAY, "sampler2DArray", "sampler3D"}},
	{"sampler2DMS", {Token::UsageBit::SAMPLER2DMS, "sampler2DMS", "sampler2D"}},
	{"sampler2DMSArray", {Token::UsageBit::SAMPLER2DARRAY, "sampler2DMSArray", "sampler3D"}},
	{"samplerCubeArray", {Token::UsageBit::SAMPLERCUBEARRAY, "samplerCubeArray", "sampler3D"}},

	{"image1D", {Token::UsageBit::IMAGE1D , "image1D", "RWTexture1D"}},
	{"image2D", {Token::UsageBit::IMAGE2D , "image2D", "RWTexture2D"}},
	{"image3D", {Token::UsageBit::IMAGE3D , "image3D", "RWTexture3D"}},
	{"imageCube", {Token::UsageBit::IMAGECUBE, "imageCube", ""}},
	{"image1DArray", {Token::UsageBit::IMAGE1DARRAY , "image1DArray", "RWTexture1DArray"}},
	{"image2DArray", {Token::UsageBit::IMAGE2DARRAY , "image2DArray", "RWTexture2DArray"}},
	{"image2DMS", {Token::UsageBit::IMAGE2DMS , "image2DMS", "RWTexture2D"}},
	{"image2DMSArray", {Token::UsageBit::IMAGE2DARRAY , "image2DMSArray", "RWTexture2DArray"}},
	{"imageCubeArray", {Token::UsageBit::IMAGECUBEARRAY, "imageCubeArray", ""}},

	{"#", {Token::UsageBit::HASH, "#", "#"}},
	{"(", {Token::UsageBit::BRACKETOPEN, "(", "("}},
	{")", {Token::UsageBit::BRACKETCLOSE, ")", ")"}},
	{"{", {Token::UsageBit::SCOPEOPEN, "{", "{"}},
	{"}", {Token::UsageBit::SCOPECLOSE, "}", "}"}},
	{".", {Token::UsageBit::FULLSTOP, ".", "."}},
	{",", {Token::UsageBit::COMMA, ",", ","}},
	{";", {Token::UsageBit::SEMICOLON, ";", ";"}},
	{":", {Token::UsageBit::COLON, ":", ":"}},
	{"'", {Token::UsageBit::SINGLEQUOTE, "'", "'"}},
	{"\"", {Token::UsageBit::DOUBLEQUOTE, "\"", "\""}},
	{"return", {Token::UsageBit::RETURN, "return", "return"}},
	{"+", {Token::UsageBit::PLUS, "+", "+"}},
	{"-", {Token::UsageBit::MINUS, "-", "-"}},
	{"*", {Token::UsageBit::MULTIPLE, "*", "*"}},
	{"/", {Token::UsageBit::DIVIDE, "/", "/"}},
	{"=", {Token::UsageBit::ASSIGN, "=", "="}},
	{"0", {Token::UsageBit::ZERO, "0", "0"}},
	{"1", {Token::UsageBit::ONE, "1", "1"}},
	{"2", {Token::UsageBit::TWO, "2", "2"}},
	{"3", {Token::UsageBit::THREE, "3", "3"}},
	{"4", {Token::UsageBit::FOUR, "4", "4"}},
	{"5", {Token::UsageBit::FIVE, "5", "5"}},
	{"6", {Token::UsageBit::SIX, "6", "6"}},
	{"7", {Token::UsageBit::SEVEN, "7", "7"}},
	{"8", {Token::UsageBit::EIGHT, "8", "8"}},
	{"9", {Token::UsageBit::NINE, "9", "9"}},
	};

	std::vector<Line> TokeniseCode(const std::string& src);

	bool CheckScopes(const std::vector<Line>& lines);
}
}