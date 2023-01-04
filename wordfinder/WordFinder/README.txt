Approach
=========
I am making use of Trie data structure to search for the words.
More details about Trie can be found at -
http://en.wikipedia.org/wiki/Trie


Build Instructions
==================

To build the program just fire make command as -

# make

After you run make, you should see an executable 'wordfinder' in the current folder.


If for some reason make does not work for you then run the following command to build the executable.

# g++ -I. Trie.cpp TrieChild.cpp TrieChildren.cpp main.cpp -o wordfinder


Run Instructions
================

The executable 'wordfinder' takes an OPTIONAL parameter which is the file from which the words are to be read.
If you do NOT pass file parameter then the program tries to read the words from file "problemwords.txt" present
in the current working directory. 

Just run the executable as -

#./wordfinder

OR

#./wordfinder <name of the file that contains the word list>

Sample Runs
============
Following is a the output of the sample run of the program 'wordfinder' -

#./wordfinder
Loading strings from file problemwords.txt ..
Strings Loaded.
Longest String: ethylenediaminetetraacetates
Finding other strings .. . . . . . . . . . . . . . . . . . . . . . . . . . .
Number of Words composed of other words in the list: 97107
Total Number of Words Read: 173528
Words found are saved in file: words_found.txt

#./wordfinder test.txt 
Loading strings from file test.txt ..
Strings Loaded.
Longest String: bbbbbbbbbbbb
Finding other strings .. . . . . .
Number of Words composed of other words in the list: 3
Total Number of Words Read: 11
Words found are saved in file: words_found.txt
