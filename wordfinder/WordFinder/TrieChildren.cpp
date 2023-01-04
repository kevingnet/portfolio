/** 
* @file TrieChildren.cpp
* @brief Implementation of TrieChildren class.
* @author Manojkumar M Waghmare
* @version 1.0
* @date 2011-11-04
*/
#include<TrieChildren.h>
#include<Trie.h>
#include<ctype.h>
#include<string.h>

TrieChildren::TrieChildren(const char label, ITrie* child, 
	const std::string& word, const bool storeEmpty) : isEmpty_(storeEmpty)
{
    memset(children_, 0, sizeof(children_));
    children_[tolower(label) - 'a'] = child;

    Trie t;
    children_[tolower(word[0]) - 'a'] = t.Add(word.substr(1));
}

ITrie* TrieChildren::Add(const std::string& word)
{
    if( "" == word ) {
	isEmpty_ = true;
	return this;
    }

    const unsigned childIndex = tolower(word[0]) - 'a';
    if( children_[childIndex] ) {
	children_[childIndex] = children_[childIndex]->Add(word.substr(1));
    }
    else {
	Trie t;
	children_[childIndex] = t.Add(word.substr(1));
    }

    return this;
}

ITrie* TrieChildren::Next(const char ch)
{
    char lowerCh = tolower(ch);
    return ( lowerCh >= 'a' && lowerCh <= 'z' ) ? children_[lowerCh - 'a'] : NULL;
}

bool TrieChildren::IsEmpty() const
{
    return isEmpty_;
}

TrieChildren::~TrieChildren()
{
    for(int i = 0; i < NUM_CHARS; ++i) {
	if(children_[i]) {
	    delete children_[i];
	    children_[i] = NULL;
	}
    }
}
