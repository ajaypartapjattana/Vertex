#include "XMLConfigLoader.h"

#include <fstream>

std::string XMLConfigLoader::readFile(const std::string& path) 
{
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("failed to load config file!");

	std::string content, line;
	while (std::getline(file, line)) {
		content += line;
		content += '\n';
	}
	return content;
}

XMLNode XMLConfigLoader::Parse(const std::string& text) 
{
	m_text = text;
	m_pos = 0;
	SkipWhitespace();
	return ParseNode();
}

const XMLAttribute* XMLConfigLoader::FindAttr(
	const XMLNode&		node, 
	const std::string&	name) 
{
	for (auto& a : node.attributes)
		if (a.name == name)
			return &a;
	return nullptr;
}

const XMLNode* XMLConfigLoader::FindChild(const XMLNode& node, const std::string& name)
{
	for (auto& c : node.children) {
		if (c.name == name)
			return &c;
		return nullptr;
	}
}

void XMLConfigLoader::SkipWhitespace() 
{
	while (m_pos < m_text.size() && isspace(Peek()))
		m_pos++;
}

XMLNode XMLConfigLoader::ParseNode() 
{
	if (Get() != '<')
		throw std::runtime_error("Expected '<'");

	XMLNode node;
	node.name = ParseIdentifier();
	node.attributes = ParseAttributes();

	if (StartsWith("/>")) {
		m_pos += 2;
		return node;
	}

	if (Get() != '>')
		throw std::runtime_error("Expected '>'");

	while (true) {
		SkipWhitespace();

		if (StartsWith("</")) {
			m_pos += 2;
			std::string endName = ParseIdentifier();
			if (endName != node.name)
				throw std::runtime_error("Mismatched closing tag");

			SkipWhitespace();
			Get();
			break;
		}

		if (Peek() == '<')
			node.children.push_back(ParseNode());
		else
			Get();
	}

	return node;
}

std::string XMLConfigLoader::ParseIdentifier() 
{
	SkipWhitespace();
	std::string id;

	while (m_pos < m_text.size()) {
		char c = Peek();
		if (isalnum(c) || c == '_' || c == '-')
			id += Get();
		else
			break;
	}

	return id;
}

std::vector<XMLAttribute> XMLConfigLoader::ParseAttributes() 
{
	std::vector<XMLAttribute> attrs;

	while (true) {
		SkipWhitespace();

		if (Peek() == '/' || Peek() == '>')
			break;

		XMLAttribute attr;
		attr.name = ParseIdentifier();

		SkipWhitespace();
		if (Get() != '=')
			throw std::runtime_error("Expected '='");

		attr.value = ParseQuotedString();
		attrs.push_back(attr);
	}

	return attrs;
}

std::string XMLConfigLoader::ParseQuotedString() 
{
	SkipWhitespace();
	if (Get() != '"')
		throw std::runtime_error("Expected '\"'");

	std::string value;
	while (Peek() != '"')
		value += Get();

	Get();
	return value;
}

bool XMLConfigLoader::StartsWith(const std::string& s) const 
{
	return m_text.compare(m_pos, s.size(), s) == 0;
}