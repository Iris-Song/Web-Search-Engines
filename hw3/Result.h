#include "config.h"

class Result
{
public:
    std::string URL;
    double score;
    std::string snippets;
    Result();
    Result(std::string,double,std::string);
    ~Result();
};

class ResultList
{
public:
    std::vector<Result> _resultList;
    void Insert(std::string, double, std::string);
    void Clear();
    void Print();
};