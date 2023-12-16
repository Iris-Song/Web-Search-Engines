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
    bool in_list(std::string, std::vector<std::string> &);
    // final project
    std::vector<std::string> splitLine(std::string);
    int snippet_no = 400;
    

public:
    std::vector<Result> _resultList;
    void Insert(uint32_t, std::string, double, std::string);
    void Clear();
    void Print();
    std::string extractSnippets(std::string, std::string, std::string, std::vector<std::string>,  std::vector<uint32_t>);

    // final project
    void ReadFake(std::string, std::vector<std::string> &, std::vector<uint32_t> &);
};