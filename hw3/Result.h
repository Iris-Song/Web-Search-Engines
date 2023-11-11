#include "config.h"

class Result
{
public:
    uint32_t docID;
    std::string URL;
    double score;
    std::string snippets;
    Result();
    Result(uint32_t, std::string, double, std::string);
    ~Result();
};

class ResultList
{
private:
    bool in_list(std::string, std::vector<std::string>&);

public:
    std::vector<Result> _resultList;
    void Insert(uint32_t, std::string, double, std::string);
    void Clear();
    void Print();
    std::string extractSnippets(std::string, std::string, std::string, std::vector<std::string>);
};