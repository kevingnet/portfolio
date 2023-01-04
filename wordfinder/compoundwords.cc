/////////////////////////////////////////////////////////////////////////////////
//
// Author: KevinG
// classes: CompoundWordCounter
// main()
/*
*/
//////////////////////////////////////////////////////////////////////

#include "compoundwords.h"
#include <pthread.h>
#include <omp.h>

//--------------------------------------------------------------------
//constructor, preallocates memory for container
CompoundWordCounter::CompoundWordCounter() 
	: m_Min(MIN_START)
{ 
}

//--------------------------------------------------------------------
// output result up to max words, default all (0)
void CompoundWordCounter::Print()
{
    const char* FoundWordsFile = "words_found.txt";
    std::ofstream logFile(FoundWordsFile);
	for (itsvec it = m_Compounds.begin(); it != m_Compounds.end(); ++it)
		logFile << *it << "\n";
}

//--------------------------------------------------------------------
//add a word from the file to container, uses minimal optimization by eliminating branching
void CompoundWordCounter::AddWord(const string& s)
{
	if (!s.size())
		return;
        //m_Min = MINI(m_Min, s.size()); //save a bit of time, instead of...
	if (s.size() < m_Min )
		m_Min = s.size();
	m_Words.insert(s);
}

//--------------------------------------------------------------------
//main operation, adds matching words to container
void CompoundWordCounter::CalcCount()
{
      for (itsset its=m_Words.begin(); its!=m_Words.end(); ++its)
      {
	      if (IsCompound(*its))
		      m_Compounds.push_back(*its);
      }
      compare c;
      sort(m_Compounds.begin(), m_Compounds.end(), c);
}

//--------------------------------------------------------------------
bool CompoundWordCounter::IsCompound(const string w) const
{
	string l;
	l.reserve(w.size() - m_Min);
	l = w.substr(0, m_Min);
	string r = w.substr(m_Min, w.size() - m_Min);
	while (r.size() >= m_Min)
	{
	  if (m_Words.find(l) != m_Words.end())
	  {
	      if (m_Words.find(r) != m_Words.end())
	      {
		return true;
	      }
	      else
	      {
		if (IsCompound(r))
		  return true;
	      }
	  }
	  l.insert(l.end(), r[0]);
	  r.erase(r.begin());
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// main()
//////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	//ifstream ifwords(argv[1]);
	//ifstream ifwords("/home/kg/w");
	//if (!ifwords.is_open())
	//	return 1;
	//time operations
	//const clock_t begin_time = clock();

	//declare counter class
	CompoundWordCounter cwc;
	//string line;
	//line.reserve(DEFAULT_LINE_SIZE);
	//read words from file and add to counter class
	
//	while (getline(ifwords, line))
//		cwc.AddWord(line);
//	ifwords.close();

	unsigned int numWords = 0;
	std::fstream wordStream(argv[1]);
	std::istream_iterator<std::string> itr_word(wordStream), itr_word_end;
	for( ;itr_word != itr_word_end; ++itr_word, ++numWords ) {
	  cwc.AddWord(*itr_word);
	}
	
	//calculate results and output stats
	cwc.CalcCount();
	//cwc.CalcCount(DONOT_OVERLAP_WORDS);
	//cout << "Time " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << endl;
	cout << "Words " << cwc.WordCount() << " Compounds: " << cwc.CompoundCount() << endl;

	cwc.Print();

	return 0;
}
//--------------------------------------------------------------------