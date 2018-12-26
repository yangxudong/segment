#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "ws_wrapper.h"
using namespace ws;
using namespace std;

void SplitString(const std::string& s, std::vector<std::string>& v, const char* c)
{
  v.clear();
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while(std::string::npos != pos2)
  {
    v.push_back(s.substr(pos1, pos2-pos1));
 
    pos1 = pos2 + strlen(c);
    pos2 = s.find(c, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}

int main(int argc, char *argv[])
{
    char* conf = NULL;
    char* ws = NULL;
    char* inputFile = NULL;
    char* tokenType = NULL;
    char* stopWordFile = NULL;
    char* splitSeperator = NULL;
    size_t field = 0;
    int outputPosTag = 0;
    int errflag = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-conf") == 0)
        {
            i++;
            if(i < argc){ conf = argv[i]; } else { errflag = 1; break; }
        }
        else  if (strcmp(argv[i], "-ws") == 0)
        {
            i++ ;
            if(i < argc){ ws = argv[i]; } else { errflag = 1; break; }
        }
        else  if (strcmp(argv[i], "-input") == 0)
        {
            i++ ;
            if(i < argc){ inputFile = argv[i]; } else { errflag = 1; break; }
        }
        else  if (strcmp(argv[i], "-sep") == 0)
        {
            i++ ;
            if(i < argc){ splitSeperator = argv[i]; } else { errflag = 1; break; }
        }
        else  if (strcmp(argv[i], "-field") == 0)
        {
            i++ ;
            if(i < argc){ field = atoi(argv[i]); } else { errflag = 1; break; }
        }
        else  if (strcmp(argv[i], "-postag") == 0)
        {
            i++ ;
            if(i < argc){ outputPosTag = atoi(argv[i]); } else { errflag = 1; break; }
        }
        else  if (strcmp(argv[i], "-stopword") == 0)
        {
            i++ ;
            if(i < argc){ stopWordFile = argv[i]; } else { errflag = 1; break; }
        }
        else  if (strcmp(argv[i], "-tokentype") == 0)
        {
            i++ ;
            if(i < argc){ tokenType = argv[i]; } else { errflag = 1; break; }
        }
        else
        {
            cerr << argv[i] << endl;
            errflag = 1;
        }
    }

    if(!conf || !ws || errflag)
    {
        cerr << "Usage: " << argv[0] << " [options]" << endl;
        cerr << "Options:" << endl;
        cerr << "  configuration path:" << endl;
        cerr << "     -conf AliTokenizer_example.conf" << endl;
        cerr << "  tokenizerId: B2B_CHN; TAOBAO_CHN; O2O_CHN; INTERNET_CHN; DAOGOU_CHN; INTERNET_JPN; INTERNET_ENG" << endl;
        cerr << "     -ws tokenizerId" << endl;
        cerr << "  input file:" << endl;
        cerr << "     -input input.txt" << endl;
        cerr << "  segTokenType: 0 for compound-semantic token; 1 for minimum-semantic token; 2 for retrieval token [default]; 3 for all tokens" << endl;
        cerr << "     -tokentype 1" << endl;
        cerr << "Example:" << endl;
        cerr << "     " << argv[0] << " -conf AliTokenizer_example.conf -ws INTERNET_CHN" << endl;
        exit(1);
    }

    set<string> stopWords;
    stopWords.insert(" ");
    //stopWords.insert("、");
    //stopWords.insert("丶");

    if (NULL != stopWordFile)
    {
        ifstream stopfile(stopWordFile);
        if (!stopfile) {
              cerr << "load input file " << inputFile << " failed." << endl;
        }
        else {
            string word;
            while (!stopfile.eof()) {
                if (!getline(stopfile, word))
                    continue;
                stopWords.insert(word);
            }
        }
    }

    WSWrapper segment;
    if (!segment.init(conf, ws))
    {
        cerr << "init failed." << endl;
        return -2;
    }
    int level = 2;
    if (NULL != tokenType)
       level = atoi(tokenType);

    ifstream infile(inputFile);
    if (!infile)
    {
      cerr << "load input file " << inputFile << " failed." << endl;
      return -1;
    }

    bool split = splitSeperator != NULL;
    std::vector<std::string> fields;
    std::vector<std::string> vecTerms;
    std::vector<unsigned> vecTagIds;
    std::vector<std::string> vecTags;
    string line;
    while (!infile.eof())
    {
        if (!getline(infile, line))
        {
            cerr << "end of file." << endl;
            continue;
        }
        vecTags.clear();
        vecTerms.clear();
        vecTagIds.clear();
        if (split) {
          SplitString(line, fields, splitSeperator);
          segment.tokenize_with_pos_tag(fields[field], level, vecTerms, vecTagIds, vecTags);
        }
        else {
          //segment.tokenize_with_pos_tag(fields[field], level, vecTerms, vecTagIds, vecTags);
          segment.tokenize(line, vecTerms, level);
        }
        if (split && field > 0) {
          size_t j = 0;
          while (j < field)
            cout << fields[j++] << splitSeperator;
        }
        for (size_t i = 0; i < vecTerms.size(); ++i)
        {
            if (stopWords.find(vecTerms[i]) != stopWords.end())
               continue;
            if (i > 0)
               cout << " ";
            if (vecTerms[i] == "、")
               cout << ";";
            else
               cout << vecTerms[i];
            if (outputPosTag)
              cout << "|" << vecTags[i];
        }
        if (split && field + 1 < fields.size()) {
          size_t j = field + 1;
          while (j < fields.size()) 
            cout << splitSeperator << fields[j++];
        }
        cout << endl;
    }
    infile.close();
    return 0;
}
