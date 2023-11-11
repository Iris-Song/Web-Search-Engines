#include "Result.h"

Result::Result()
{
}

Result::Result(uint32_t docID, std::string URL, double score, std::string snippets)
{
    this->docID = docID;
    this->URL = URL;
    this->score = score;
    this->snippets = snippets;
}

Result::~Result()
{
}

void ResultList::Insert(uint32_t docID, std::string URL, double score, std::string snippets)
{
    Result res(docID, URL, score, snippets);
    _resultList.push_back(res);
}

void ResultList::Clear()
{
    _resultList.clear();
}

void ResultList::Print()
{
    std::cout<<"Here are the Top "<<RESULT_NUM<<" results:"<<std::endl;
    for(int i=_resultList.size()-1;i>=0;i--)
    {
        std::cout<<std::setw(2)<<(_resultList.size()-i)<<": "\
        <<_resultList[i].URL<<" "<<_resultList[i].score<<std::endl;
        std::cout<<_resultList[i].snippets<<std::endl;
        std::cout<<std::endl;
    }
}


std::string ResultList::extractSnippets(std::string org, std::string bstr, std::string estr, std::vector<std::string> word_list)
{
    size_t begin_tag_len = bstr.length();

    size_t start_pos = org.find(bstr) + begin_tag_len;
    size_t end_pos = org.find(estr);
    std::string text = org.substr(start_pos, end_pos - start_pos);
    start_pos = text.find("\n");
    text = text.substr(start_pos);

    std::string sep = " :;,.\t\v\r\n\f[]{}()<>+-=*&^%$#@!~`\'\"|\\/?·\"：“”";
    std::string word;
    std::string snippets;

    for (size_t i = 0; i < text.length(); i++)
    {
        if (sep.find(text[i]) == std::string::npos)
        {
            word += text[i];
        }
        else
        {
            if (word.length()&&in_list(word,word_list))
            {
                size_t s_begin = int(i-word.length()-SNIPPETS_RANGE)>0 ? int(i-word.length()-SNIPPETS_RANGE):0;
                size_t s_end = i+SNIPPETS_RANGE<text.length()?i+SNIPPETS_RANGE:text.length();
                snippets = text.substr(s_begin, s_end - s_begin);
                break;
            }
            word.clear();
        }
    }
    return snippets;
}

bool ResultList::in_list(std::string word, std::vector<std::string> &word_list)
{
    for(int i=0;i<word_list.size();i++)
    {
        if(word==word_list[i]){
            return true;
        }
    }
    return false;
}