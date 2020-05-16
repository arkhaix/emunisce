#include "CommandTrie.h"
using namespace Emunisce;

#include <cstring> //strlen
#include <string>
#include <vector>


namespace Emunisce
{

	class CommandTrie_Private
	{
	public:

		CommandTrie_Private()
		{
		}

		CommandTrie* owner;

		CommandTrie* parent;

		std::string value;
		CommandTrie* children[26]; //a-z

		std::vector<CommandTrie*> leaves;
	};

} // namespace Emunisce

CommandTrie::CommandTrie(CommandTrie* parent)
{
	m_private = new CommandTrie_Private();
	m_private->owner = this;
	m_private->parent = parent;

	m_private->value.clear();

	for (auto& child : m_private->children)
	{
		child = nullptr;
	}
}

CommandTrie::~CommandTrie()
{
	for (auto& child : m_private->children)
	{
		if (child != nullptr) {
			delete child;
		}
	}

	delete m_private;
}


void CommandTrie::Add(const char* command, unsigned int position)
{
	if (command == nullptr || strlen(command) == 0) {
		return;
	}

	//Leaf
	if (position >= strlen(command) && m_private->parent != nullptr)
	{
		m_private->value = command;
		RegisterLeaf(this);
	}

	//Not leaf
	else
	{
		// Must be a-z
		if (command[position] < 'a' || command[position] > 'z') {
			return;
		}

		unsigned int index = (unsigned int)(command[position] - 'a');
		if (m_private->children[index] == nullptr) {
			m_private->children[index] = new CommandTrie(this);
		}

		m_private->children[index]->Add(command, position + 1);
	}
}

const char* CommandTrie::GetValue()
{
	return m_private->value.c_str();
}

bool CommandTrie::IsLeaf()
{
	return !m_private->value.empty();
}


unsigned int CommandTrie::NumLeaves()
{
	return (unsigned int)m_private->leaves.size();
}

CommandTrie* CommandTrie::GetLeaf(unsigned int index)
{
	if (index >= m_private->leaves.size()) {
		return nullptr;
	}

	return m_private->leaves[index];
}

CommandTrie* CommandTrie::GetNode(const char* prefix, unsigned int position)
{
	if (prefix == nullptr || strlen(prefix) == 0) {
		return this;
	}

	if (position >= strlen(prefix) && m_private->parent != nullptr) {
		return this;
	}

	unsigned int index = (unsigned int)(prefix[position] - 'a');
	if (m_private->children[index] == nullptr) {
		return nullptr;
	}

	return m_private->children[index]->GetNode(prefix, position + 1);
}

void CommandTrie::RegisterLeaf(CommandTrie* leaf)
{
	if (leaf == nullptr) {
		return;
	}

	m_private->leaves.push_back(leaf);
	if (m_private->parent != nullptr)
	{
		m_private->parent->RegisterLeaf(leaf);
	}
}
