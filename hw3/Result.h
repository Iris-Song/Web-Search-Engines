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
    void updateDocIDSnippets(uint32_t, std::string);
    std::string extractSnippets(std::string, std::string, std::string, std::vector<std::string>);
    bool in_list(std::string, std::vector<std::string>&);

public:
    std::vector<Result> _resultList;
    void Insert(uint32_t, std::string, double, std::string);
    void Clear();
    void Print();
    void FindSnippets(std::vector<std::string>);
};