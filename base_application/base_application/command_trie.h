#ifndef COMMANDTRIE_H
#define COMMANDTRIE_H

namespace emunisce {

class CommandTrie {
public:
	CommandTrie(CommandTrie* parent = nullptr);
	~CommandTrie();

	void Add(const char* command, unsigned int position = 0);
	const char* GetValue();

	bool IsLeaf();

	unsigned int NumLeaves();
	CommandTrie* GetLeaf(unsigned int index);

	CommandTrie* GetNode(const char* prefix, unsigned int position = 0);

private:
	void RegisterLeaf(CommandTrie* leaf);

	class CommandTrie_Private* m_private;
};

}  // namespace emunisce

#endif