/** 
* @file Trie.cpp
* @brief Implementation of Class Trie.
* @author Manojkumar M Waghmare
* @version 1.0
* @date 2011-11-04
*/
#include<Trie.h>
#include<TrieChild.h>

ITrie* Trie::Add(const std::string& word)
{
    return ( word == "" ) ? (ITrie*) new Trie(true) : (ITrie*) new TrieChild(word, isEmpty_);
}

ITrie* Trie::Next(const char ch)
{
    return NULL;
}

bool Trie::IsEmpty() const
{
    return isEmpty_;
}


