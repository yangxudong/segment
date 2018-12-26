#include "ws_wrapper.h"
using namespace std;

namespace ws
{

  WSWrapper::WSWrapper(): m_bInited(false), m_pFactory(NULL), m_pTokenizer(NULL)
  {
    _productTags.insert(39); // 餐饮类型
    _productTags.insert(41); // 酒店类型
    _productTags.insert(42); // 休闲娱乐类型
    _productTags.insert(46); // 服务-产品-餐饮
    _productTags.insert(47); // 服务-产品-酒店
    _productTags.insert(48); // 服务-产品-休闲娱乐
    _productTags.insert(2054); // 专有名词-菜肴
    _productTags.insert(16386); // 吃喝玩乐-娱乐休闲
    _productTags.insert(22532); // 特殊商品-服务

    _poiTags.insert(34); // 医院
    _poiTags.insert(35); // 学校
    _poiTags.insert(37); // 机场
    _poiTags.insert(38); // 火车站
    _poiTags.insert(43); // 服务-品牌
    _poiTags.insert(16387); // 吃喝玩乐-服务场所

    _addressTags.insert(11); // 区划-省
    _addressTags.insert(12); // 区划-市
    _addressTags.insert(13); // 区划-区县
    _addressTags.insert(14); // 区划-乡镇村
    _addressTags.insert(15); // 道路
    _addressTags.insert(17); // 桥梁
    _addressTags.insert(44); // 商业地产
    _addressTags.insert(45); // 住宅小区
    _addressTags.insert(49); // 景点
    _addressTags.insert(5122); // 区划-国家及地区
    _addressTags.insert(5123); // 区划-有不可去后缀区划
    _addressTags.insert(5124); // 区划-有可去后缀区划
    _addressTags.insert(5125); // 区划-无后缀区划

    _importantTags.insert(21);
    _importantTags.insert(22);
    _importantTags.insert(21506);
    _importantTags.insert(21507);
    _importantTags.insert(21508);
    _importantTags.insert(30721);
    _importantTags.insert(29697);
    _importantTags.insert(7334);
    _importantTags.insert(23);
    _importantTags.insert(43);
    _importantTags.insert(9231);
    _importantTags.insert(9244);
    _importantTags.insert(9245);
    _importantTags.insert(7170);
    _importantTags.insert(7186);
    _importantTags.insert(7202);
    _importantTags.insert(7234);
    _importantTags.insert(7235);
  }

  WSWrapper::~WSWrapper()
  {
    if (NULL != m_pFactory) {
      delete m_pFactory;
      m_pFactory = NULL;
    }
  }

  bool WSWrapper::init(const char *ws_dict_path, const char *chn)
  {
    if (m_bInited) return true;

    m_pFactory = new AliTokenizerFactory();
    if (NULL == m_pFactory || !m_pFactory->Init(ws_dict_path))
    {
      fprintf(stderr, "load conf error: %s\n", ws_dict_path);
      return false;
    }

    m_pTokenizer = m_pFactory->GetAliTokenizer(chn);
    if (!m_pTokenizer)
    {
      fprintf(stderr, "tokenizer: %s load failed\n", chn);
      return false;
    }

    if (pthread_key_create(&_key, NULL) != 0)
    {   
      fprintf(stderr, "Error during creating pthread_key_t!\n");
      return false;
    }
    m_bInited = true;
    return true;
  }

  int WSWrapper::tokenize(string &strLineBuf, int grain, vector<string>& vecTerms, vector<vector<unsigned> >& vecTagIds, vector<vector<string> >& vecTags)
  {
    ws::SegResult *pSegResult = (ws::SegResult *)pthread_getspecific(_key);
    if (pSegResult == NULL)
    {
      pSegResult = m_pTokenizer->CreateSegResult();
      if (!pSegResult)
      {
        fprintf(stderr, "Create SegResult failed!");
        return -1;
      }
      if (pthread_setspecific(_key, pSegResult) != 0)
      {
        fprintf(stderr, "Error during pthread_setspecific!");
        return -1;
      }
    }
    int ret = m_pTokenizer->Segment(strLineBuf.c_str(), strLineBuf.size(), UTF8, SEG_TOKEN_RETRIEVE, pSegResult);
    if (ret != 0)
    {   
        fprintf(stderr, "ERROR: main(): segment error\n");
        return -1;
    }

    if (2 == grain)
    {
      //get minimum semantic token using iterator method
      for (SegResult::Iterator iter = pSegResult->Begin(SEMANTIC_ITERATOR);
        iter != pSegResult->End(SEMANTIC_ITERATOR); ++iter)
      {
        vecTerms.push_back(string((const char*)iter->pWord, iter->length));
        vector<unsigned> vecTagId;
        vector<string> vecTag;
        set<string> expandTag;
        for(int i = 0; i < iter->semanticTagNum; i++)
        {
          unsigned id = iter->pSemanticTag[i].id;
          vecTagId.push_back(id);
          vecTag.push_back(pSegResult->GetSemanticTagName(iter->pSemanticTag[i].id));
          if (_productTags.count(id) > 0)
            expandTag.insert("PRODUCT");
          if (_poiTags.count(id) > 0) 
            expandTag.insert("POI");
          if (_addressTags.count(id) > 0) 
            expandTag.insert("ADDRESS");
          if (_importantTags.count(id) > 0) 
            expandTag.insert("IMPORTANT");
        }
        for (set<string>::iterator it = expandTag.begin(); it != expandTag.end(); ++it)
          vecTag.push_back(*it);
        vecTagIds.push_back(vecTagId);
        vecTags.push_back(vecTag);
      }
    }
    else if (1 == grain)
    {
      //get retrieval token using iterator method
      for (SegResult::Iterator iter = pSegResult->Begin(RETRIEVE_ITERATOR);
        iter != pSegResult->End(RETRIEVE_ITERATOR); ++iter)
      {
        if (IsExtendRetrieveToken(iter->tokenType))
        {
          continue;
        }
        vecTerms.push_back(string((const char*)iter->pWord, iter->length));
        vector<unsigned> vecTagId;
        vector<string> vecTag;
        set<string> expandTag;
        for(int i = 0; i < iter->semanticTagNum; i++)
        {
          unsigned id = iter->pSemanticTag[i].id;
          vecTagId.push_back(id);
          vecTag.push_back(pSegResult->GetSemanticTagName(iter->pSemanticTag[i].id));
          if (_productTags.count(id) > 0)
            expandTag.insert("PRODUCT");
          if (_poiTags.count(id) > 0) 
            expandTag.insert("POI");
          if (_addressTags.count(id) > 0) 
            expandTag.insert("ADDRESS");
          if (_importantTags.count(id) > 0) 
            expandTag.insert("IMPORTANT");
        }
        for (set<string>::iterator it = expandTag.begin(); it != expandTag.end(); ++it)
          vecTag.push_back(*it);
        vecTagIds.push_back(vecTagId);
        vecTags.push_back(vecTag);
      }
    }
    else
    {
      //get semantic token
      SegToken* pToken= pSegResult->GetFirstToken(MAIN_LIST);
      while (pToken)
      {
        vecTerms.push_back(string((const char*)pToken->pWord, pToken->length));
        vector<unsigned> vecTagId;
        vector<string> vecTag;
        set<string> expandTag;
        for(int i = 0; i < pToken->semanticTagNum; i++)
        {
          unsigned id = pToken->pSemanticTag[i].id;
          vecTagId.push_back(id);
          vecTag.push_back(pSegResult->GetSemanticTagName(pToken->pSemanticTag[i].id));
          if (_productTags.count(id) > 0)
            expandTag.insert("PRODUCT");
          if (_poiTags.count(id) > 0) 
            expandTag.insert("POI");
          if (_addressTags.count(id) > 0) 
            expandTag.insert("ADDRESS");
          if (_importantTags.count(id) > 0) 
            expandTag.insert("IMPORTANT");
        }
        for (set<string>::iterator it = expandTag.begin(); it != expandTag.end(); ++it)
          vecTag.push_back(*it);
        vecTagIds.push_back(vecTagId);
        vecTags.push_back(vecTag);

        pToken = pToken->pRightSibling;
      }
    }
    return 0;
  }

  bool WSWrapper::getalltag(SegResult *pSegResult, SegToken* pToken, set<string>& tags, vector<unsigned> &vecTagId)
  {
    tags.clear();
    bool other = true;
    for(uint8_t j = 0; j < pToken->semanticTagNum; j++)
    {
      int16_t id = pToken->pSemanticTag[j].id;
      vecTagId.push_back(id);
      if (_productTags.count(id) > 0) {
        tags.insert("PRODUCT");
        other = false;
      }
      if (_poiTags.count(id) > 0) 
        tags.insert("POI");
      if (_addressTags.count(id) > 0) 
        tags.insert("ADDRESS");

      if (other && _importantTags.count(id) > 0) {
        other = false;
      }
    }
    if (other)
      tags.insert("OTHER");
    return true;
  }

  /*
   * level = 2 表示语义小粒度；其他表示检索粒度
   */
  int WSWrapper::tokenize(const std::string &keyword, std::vector<std::string>  &vecWord, int level)
  {
    vector<pair<string, vector<string> > > vec_pair_words;
    vector<set<string> > vec_tags;
    int ret = tokenize(keyword, vec_pair_words, vec_tags); 

    if (ret != 0)
    {
      return ret;
    }

    vecWord.clear();
    vector<pair<string, vector<string> > >::iterator iter;
    vector<string>::iterator iter2;
    iter = vec_pair_words.begin(); 
    for (; iter != vec_pair_words.end(); iter++)
    {
      if (level == 2)
      {
        vecWord.push_back(iter->first);
        continue;
      }

      iter2 = iter->second.begin();
      for (; iter2 != iter->second.end(); iter2++)
      {
        vecWord.push_back(*iter2);
      }
    }

    return 0;
  }

  int WSWrapper::tokenize(const std::string &keyword, std::vector<std::pair<std::string, std::vector<std::string> > > &vecWord,
    vector<set<string> >& vecTags) {
    vector<vector<unsigned> > vecTagIds;
    return tokenize(keyword, vecWord, vecTags, vecTagIds);
  }

  int WSWrapper::tokenize(const std::string &keyword, std::vector<std::pair<std::string, std::vector<std::string> > > &vecWord,
    vector<set<string> >& vecTags, vector<vector<unsigned> >& vecTagIds)
  {
    if (!m_bInited) {
      fprintf(stderr, "WSWrapper has not be inited!\n");
      return -1;
    }
    vecWord.clear();
    vecTags.clear();
    vecTagIds.clear();

    ws::SegResult *pSegResult = (ws::SegResult *)pthread_getspecific(_key);
    if (pSegResult == NULL)
    {
      pSegResult = m_pTokenizer->CreateSegResult();
      if (!pSegResult)
      {
        fprintf(stderr, "Create SegResult failed!\n");
        return -1;
      }
      if (pthread_setspecific(_key, pSegResult) != 0)
      {
        fprintf(stderr, "Error during pthread_setspecific!\n");
        return -1;
      }
    }

    int32_t pret = m_pTokenizer->Segment(keyword.c_str(), keyword.size(),
      UTF8, SEG_TOKEN_RETRIEVE_BASIC, pSegResult);
    if (0 != pret) {
      fprintf(stderr, "segment error for [%s]\n", keyword.c_str());
      return pret;
    }

    string semanticItem;
    string retrieveItem;
    vector<string> vecRetrieveItem;
    SegToken* p_sub_token = 0;
    
    set<string> tags;
    //遍历最小语义单元
    for (SegResult::Iterator iter = pSegResult->Begin(SEMANTIC_ITERATOR);
          iter != pSegResult->End(SEMANTIC_ITERATOR); ++iter)
    {   
      vecRetrieveItem.clear();
      semanticItem.assign(iter->pWord, iter->length); 
      if (semanticItem == " ")
        continue;
      vector<unsigned> vecTagId;
      getalltag(pSegResult, &(*iter), tags, vecTagId); // get tags
      vecTags.push_back(tags);
      vecTagIds.push_back(vecTagId);
      if (iter->subTokenNum > 0)
      {   
        for (int i = 0; i < iter->subTokenNum; ++i)
        {   
          p_sub_token = iter->pSubToken + i;
          retrieveItem.assign(p_sub_token->pWord, p_sub_token->length);
          if (retrieveItem == " ")
            continue;
          vecRetrieveItem.push_back(retrieveItem);
        }   
      }   
      else
      {   
        vecRetrieveItem.push_back(semanticItem);  
      }   
      
      //增加一条 <最小语义, 检索单元列表>
      vecWord.push_back(pair<string, vector<string> >(semanticItem, vecRetrieveItem));
    }
    return 0;
  }

  int WSWrapper::tokenize_with_pos_tag(string &strLineBuf, int grain, vector<string>& vecTerms, vector<unsigned>& vecTagIds, vector<string>& vecTags)
  {
    ws::SegResult *pSegResult = (ws::SegResult *)pthread_getspecific(_key);
    if (pSegResult == NULL)
    {
      pSegResult = m_pTokenizer->CreateSegResult();
      if (!pSegResult)
      {
        fprintf(stderr, "Create SegResult failed!");
        return -1;
      }
      if (pthread_setspecific(_key, pSegResult) != 0)
      {
        fprintf(stderr, "Error during pthread_setspecific!");
        return -1;
      }
    }
    int ret = m_pTokenizer->Segment(strLineBuf.c_str(), strLineBuf.size(), UTF8, SEG_TOKEN_RETRIEVE, pSegResult);
    if (ret != 0)
    {   
        fprintf(stderr, "ERROR: main(): segment error\n");
        return -1;
    }

    if (2 == grain)
    {
      //get minimum semantic token using iterator method
      for (SegResult::Iterator iter = pSegResult->Begin(SEMANTIC_ITERATOR);
        iter != pSegResult->End(SEMANTIC_ITERATOR); ++iter)
      {
        vecTerms.push_back(string((const char*)iter->pWord, iter->length));
        vecTagIds.push_back(iter->posTagId);
        const char * tag = pSegResult->GetPosTagName(iter->posTagId);
        if (tag) {
          vecTags.push_back(string(tag));
        }
        else {
          vecTags.push_back(string("unknown"));
        }
      }
    }
    else if (1 == grain)
    {
      //get retrieval token using iterator method
      for (SegResult::Iterator iter = pSegResult->Begin(RETRIEVE_ITERATOR);
        iter != pSegResult->End(RETRIEVE_ITERATOR); ++iter)
      {
        if (IsExtendRetrieveToken(iter->tokenType))
        {
          continue;
        }
        vecTerms.push_back(string((const char*)iter->pWord, iter->length));
        vecTagIds.push_back(iter->posTagId);
        const char * tag = pSegResult->GetPosTagName(iter->posTagId);
        if (tag) {
          vecTags.push_back(string(tag));
        }
        else {
          vecTags.push_back(string("unknown"));
        }
      }
    }
    else
    {
      //get semantic token
      SegToken* pToken= pSegResult->GetFirstToken(MAIN_LIST);
      while (pToken)
      {
        vecTerms.push_back(string((const char*)pToken->pWord, pToken->length));
        vecTagIds.push_back(pToken->posTagId);
        const char * tag = pSegResult->GetPosTagName(pToken->posTagId);
        if (tag) {
          vecTags.push_back(string(tag));
        }
        else {
          vecTags.push_back(string("unknown"));
        }
        pToken = pToken->pRightSibling;
      }
    }
    return 0;
  }
}
