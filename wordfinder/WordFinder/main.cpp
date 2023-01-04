/** 
* @file main.cpp
* @brief 
* @author Manojkumar M Waghmare
* @version 1.0
* @date 2011-11-04
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <map>
#include <set>
#include <algorithm>
#include <list>
#include <stack>

#include <Trie.h>

struct LookAheadBuffer
{
    public:

	LookAheadBuffer(ITrie* trie = NULL, int i = 0) : savedTrie_(trie), savedI_(i)
	{}

	void get(ITrie*& trie, int& i) const
	{
	    trie = savedTrie_;
	    i = savedI_;
	}

	ITrie*	savedTrie_;
	int	savedI_;
};

typedef std::stack<LookAheadBuffer> LookAheadContextStack;


/** 
* @brief 
* 
* @param nTrie [in/out] - Trie to be used for look ahead in the given string str. 
*			    Upon return this parameter holds Trie for first character after found word.
* @param str [in] - String to be looked ahead for words.
* @param from [in/out] - location within string param str from which the look ahead is to be started.
* @param to [in] - look ahead until 'to' location withing string 'str'
* @param context [out] - if a word is found during look ahead then this context is updated with information
*			    just before look ahead started.
* 
* @return true if a word is found during look ahead & out params are updated accordingly.
*	    false if word is not found during look ahead.
*/
bool lookAhead(ITrie*& nTrie, const std::string& str, 
		int& from, const int to, LookAheadContextStack& context)
{
    int i = from;
    ITrie* nTrieNext = nTrie->Next(str[i++]);
    ITrie* peekAhead = nTrieNext;
    while( peekAhead && !peekAhead->IsEmpty() && i < to) {
	peekAhead = peekAhead->Next(str[i++]); 
    }

    if( NULL == peekAhead ) {
	return false;
    }
    else if(peekAhead->IsEmpty()) {
	context.push(LookAheadBuffer(nTrie, from));
	from = i;
	nTrie = peekAhead;
	return true;
    }
    return false;
}

int main(int argc, char* argv[])
{

    const char* ProblemWordsFile = (argc == 2) ? argv[1] : "problemwords.txt"; 
    std::fstream wordStream(ProblemWordsFile);
    std::istream_iterator<std::string> itr_word(wordStream), itr_word_end;

    ITrie* trie[26] = { 0 };  //26 different tries for 26 alphabetes
    for(int i = 0; i < 26; ++i) {
	trie[i] = new Trie();
    }

    typedef std::list<std::string> StringList;
    typedef std::map<unsigned int, StringList>  LengthToStringsMap;
    typedef std::set<int> SetOfLengths;

    LengthToStringsMap lengthStringsMap;    //map for grouping strings based on their length.
    SetOfLengths lengthSet;		    //set of lengths of all strings in the file

    unsigned int numWords = 0;
    std::cout << "Loading strings from file " << ProblemWordsFile << " .." << std::endl;
    for( ;itr_word != itr_word_end; ++itr_word, ++numWords ) {
	const unsigned index =  tolower(itr_word->at(0)) - 'a' ; //get the trie index for the string
	trie[index] = trie[index]->Add(*itr_word);  //add the string to the trie

	const size_t wordLength = itr_word->length();
	lengthStringsMap[wordLength].push_back(*itr_word);	//put the string in its length group
	lengthSet.insert(wordLength);				//put the length of the string in the set
    }
    std::cout << "Strings Loaded." << std::endl;

    const char* FoundWordsFile = "words_found.txt";
    std::ofstream logFile(FoundWordsFile);
    int numStringsFound = 0;

    //Iterate over set of lengths beginning with longest length.
    for(SetOfLengths::reverse_iterator itr = lengthSet.rbegin(); itr!=lengthSet.rend(); itr++) {
	const size_t stringLength = *itr;
	const StringList& sl = lengthStringsMap[stringLength];			

	for(StringList::const_iterator sitr = sl.begin(); sitr != sl.end(); ++sitr) {
	    ITrie* nTrie = trie[tolower((*sitr)[0]) - 'a']; //get the correct trie for the string 
	    int i = 0; 
	    bool wordFound = false , isLookedAhead = false, fromContext = false;
	    LookAheadContextStack lookAheadContext;

	    while(i <= stringLength) { //iterate over string

		while(nTrie && !nTrie->IsEmpty() && i < stringLength) { //iterate until a word is found
		    nTrie = nTrie->Next((*sitr)[i++]);
		}

		if( stringLength == i || NULL == nTrie) {
		    if( wordFound && nTrie && nTrie->IsEmpty() ) //if iteration ends with finding word then break.
			break;

		    if(isLookedAhead) {  //reached end because of lookahead then backtrack to previous context & continue.
			lookAheadContext.top().get(nTrie, i); //get the last lookahead context from top
			lookAheadContext.pop();		//pop off the last context
			fromContext = true;	
			if(lookAheadContext.empty()) { //all lookahead contexts have been searched now.
			    isLookedAhead = false; 
			}
			continue;
		    }
		    break;
		}

		//look ahead only if we are not backtracking i.e. fromContext is false
		if( !fromContext && lookAhead(nTrie, *sitr, i, stringLength, lookAheadContext) )  { 
		    isLookedAhead = true;
		    continue;
		}
		fromContext = false;	//enable look aheads.

		nTrie = trie[tolower((*sitr)[i]) - 'a'];	//switch the trie to next word's first character.
		wordFound = true;							//a word has been found 
	    }

	    //Desired string is found only if iteration is complete with wordFound set and Trie is Empty.
	    if(wordFound && i == stringLength && nTrie && nTrie->IsEmpty()) {
		++numStringsFound; 
		if( 1 == numStringsFound ) {
		    std::cout << "Longest String: " << *sitr << std::endl;
		    std::cout << "Finding other strings .." ;
		}
		logFile << *sitr << "\n";
	    }
	}
	if( numStringsFound ) 
	    std::cout << " .";
    }

    for(int i = 0; i < 26; ++i) {  //cleanup tries
	delete trie[i];
	trie[i] = NULL;
    }

    logFile.close();
    std::cout << "\nNumber of Words composed of other words in the list: " << numStringsFound << "\n"
	<< "Total Number of Words Read: " << numWords  << "\n"
	<< "Words found are saved in file: " << FoundWordsFile << std::endl;

    return 0;
}

