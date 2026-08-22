#pragma once

#include <string>
#include <vector>

struct ML_Attribute {
	std::string name;
	std::string value;
};

struct ML_Node {
	std::string name;
	std::vector<ML_Attribute> attributes;
	std::vector<ML_Node> children;
};

class XMLConfigLoader {
public:
	XMLConfigLoader(const std::string& path);
	
	ML_Node root;

private:
	std::string readFile(const std::string& path);
	void parse();

	char Peek() const { return m_text[m_pos]; }
	char Get() { return m_text[m_pos++]; }

	void SkipWhitespace();

	ML_Node ParseNode();

	std::string ParseIdentifier();
	std::vector<ML_Attribute> ParseAttributes();
	std::string ParseQuotedString();

	bool StartsWith(const std::string& s) const;

private:
	std::string m_text;
	size_t m_pos = 0;
};