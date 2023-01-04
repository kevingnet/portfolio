/** 
* @file TrieChild.cpp
* @brief Implementation of class TrieChild.
* @author Manojkumar M Waghmare
* @version 1.0
* @date 2011-11-04
*/
#include<TrieChild.h>
#include<Trie.h>
#include<TrieChildren.h>
#include<ctype.h>

TrieChild::TrieChild(const std::string& word, 
	const bool storeEmpty) : isEmpty_(storeEmpty), label_(tolower(word[0])) 
{
    Trie t;
    child_ = t.Add(word.substr(1));
}

ITrie* TrieChild::Add(const std::string& word)
{
    if( word == "" ) {
	isEmpty_ = true;
	return this;
    }

    if( label_ == tolower(word[0]) ) {
	child_ = child_->Add(word.substr(1));
	return this;
    }

    return new TrieChildren(label_, child_, word, isEmpty_);
}

ITrie* TrieChild::Next(const char ch)
{
    return (tolower(ch) ==  label_) ? child_ : NULL;
}

bool TrieChild::IsEmpty() const
{
    return isEmpty_;
}

TrieChild::~TrieChild()
{
    if( child_ ) {
	delete child_;
	child_ = NULL;
    }
}
