#include "Result.h"

Result::Result()
{
}

Result::Result(std::string URL, double score, std::string snippets)
{
    this->URL = URL;
    this->score = score;
    this->snippets = snippets;
}

Result::~Result()
{
}

void ResultList::Insert(std::string URL, double score, std::string snippets)
{
    Result res(URL, score, snippets);
    _resultList.push_back(res);
}

void ResultList::Clear()
{
    _resultList.clear();
}

void ResultList::Print()
{
    std::cout<<"Here are the Top "<<RESULT_NUM<<" results:"<<std::endl;
    for(int i=0;i<_resultList.size();i++)
    {
        std::cout<<std::setw(2)<<(i+1)<<": "\
        <<_resultList[i].URL<<" "<<_resultList[i].score<<std::endl;
        std::cout<<_resultList[i].snippets<<std::endl;
    }
}
