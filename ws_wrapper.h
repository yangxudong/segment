#ifndef __WS_WRAP_H_
#define __WS_WRAP_H_
#include <vector>
#include <set>
#include <string>
#include <ali_tokenizer.h>
#include <pthread.h>
#include <cstdio>

namespace ws
{
class WSWrapper 
{
public:
  WSWrapper();
  ~WSWrapper();
  bool init(const char *wsDictPath, const char *chn = "O2O_CHN"); 
  int tokenize(std::string &, int grain, std::vector<std::string>& vecTerms,
    std::vector<std::vector<unsigned> >& vecTagIds, std::vector<std::vector<std::string> >& vecTags);
  /**
  * 获取嵌套的结果
  */
  int tokenize(const std::string &keyword, std::vector<std::pair<std::string, std::vector<std::string> > > &vecWord,
    std::vector<std::set<std::string> >& vecTags);
  int tokenize(const std::string &keyword, std::vector<std::pair<std::string, std::vector<std::string> > > &vecWord,
    std::vector<std::set<std::string> >& vecTags, std::vector<std::vector<unsigned> >& vecTagIds);
  int tokenize(const std::string &keyword, std::vector<std::string> &vecWord, int level = 2);
  int tokenize_with_pos_tag(std::string &strLineBuf, int grain, std::vector<std::string>& vecTerms,
    std::vector<unsigned>& vecTagIds, std::vector<std::string>& vecTags);
private:
  bool getalltag(SegResult* pSegResult, SegToken* pToken, std::set<std::string>& tags, std::vector<unsigned> &vecTagId);

private:
  bool m_bInited;
  ws::AliTokenizerFactory *m_pFactory;
  ws::AliTokenizer *m_pTokenizer;
  std::set<int16_t> _productTags;
  std::set<int16_t> _poiTags;
  std::set<int16_t> _addressTags;
  std::set<int16_t> _importantTags;
  pthread_key_t _key;
};

}
#endif
