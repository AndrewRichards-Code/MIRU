#include "Tokeniser.h"

using namespace miru;
using namespace shader_compiler;

std::vector<Line> miru::shader_compiler::TokeniseCode(const std::string& src)
{
	std::vector<Line> lines;
	std::stringstream ss(src);
	std::string line_str;
	uint32_t lineNumber = 1;
	while (std::getline(ss, line_str, '\n'))
	{
		while (line_str.find('\t') != std::string::npos)
			line_str.erase(line_str.find('\t'), 1);

		Line line;
		line.value = line_str;
		line.number = (lineNumber++);

		std::stringstream ss_line(line_str);
		std::string token_str;
		while (std::getline(ss_line, token_str, ' '))
		{
			Token token;
			token.value = token_str;

			auto it = typemap.find(token_str);
			if (it != typemap.end())
				token.type = it->second.type;
			else
				token.type = Token::UsageBit::COMBINED;

			if (token.type == Token::UsageBit::COMBINED)
			{
				std::string temp, tempCharStr;
				for (auto it = token.value.begin(); it != token.value.end(); it++)
				{
					tempCharStr = *it;
					auto it1 = typemap.find(tempCharStr);
					if (it1 != typemap.end())
					{
						Token temp_token1, temp_token2;

						if (!temp.empty())
						{
							temp_token1.type = Token::UsageBit::NAME;
							temp_token1.value = temp;
							token.tokens.push_back(temp_token1);
							temp.clear();
						}

						temp_token2.type = it1->second.type;
						temp_token2.value = tempCharStr;
						token.tokens.push_back(temp_token2);
						tempCharStr.clear();
					}
					temp += tempCharStr;
					auto it2 = typemap.find(temp); //For token.value.size() w/ lenght > 1.
					if (it2 != typemap.end() && it != token.value.end() - 1 && !temp.empty())
					{
						if (*(it + 1) != ' ')
							continue;
						Token temp_token;
						temp_token.type = it2->second.type;
						temp_token.value = temp;
						token.tokens.push_back(temp_token);
						temp.clear();
					}
					if (it == token.value.end() - 1 && !temp.empty())
					{
						Token temp_token;
						temp_token.type = Token::UsageBit::NAME;
						temp_token.value = temp;
						token.tokens.push_back(temp_token);
						temp.clear();
					}
				}
				if (token.tokens.empty())
					token.type = Token::UsageBit::NAME;
			}
			line.tokens.push_back(token);
		}
		lines.push_back(line);
	}
	return lines;
}

bool miru::shader_compiler::CheckScopes(const std::vector<Line>& lines)
{
	int32_t scopeNumber = 0;
	for (auto& line : lines)
	{
		for (auto& token : line.tokens)
		{
			if (token.type == Token::UsageBit::SCOPEOPEN)
				scopeNumber++;
			else if (token.type == Token::UsageBit::SCOPECLOSE)
				scopeNumber--;
			else if (token.type == Token::UsageBit::COMBINED)
			{
				for (auto& subToken : token.tokens)
				{
					if (subToken.type == Token::UsageBit::SCOPEOPEN)
						scopeNumber++;
					else if (subToken.type == Token::UsageBit::SCOPECLOSE)
						scopeNumber--;
					else
						continue;
				}
			}
			else
				continue;
		}
	}
	if (scopeNumber)
		return false;
	else
		return true;
}
