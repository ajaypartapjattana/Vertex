#pragma once

#include "XMLReadTypes/XMLTypes.h"

#include <string>

class XMLConfigLoader {
public:
	std::string readFile(const std::string& path);
	XMLNode Parse(const std::string& text);

	const XMLAttribute* FindAttr(const XMLNode& node, const std::string& name);
	const XMLNode* FindChild(const XMLNode& node, const std::string& name);

private:
	char Peek() const { return m_text[m_pos]; }
	char Get() { return m_text[m_pos++]; }

	void SkipWhitespace();

	XMLNode ParseNode();

	std::string ParseIdentifier();
	std::vector<XMLAttribute> ParseAttributes();
	std::string ParseQuotedString();

	bool StartsWith(const std::string& s) const;

private:
	std::string m_text;
	size_t m_pos = 0;
};