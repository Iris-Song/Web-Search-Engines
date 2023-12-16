#include "Result.h"
#include "Snippets.h"

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
    std::cout << "Here are the Top " << RESULT_NUM << " results:" << std::endl;
    for (int i = _resultList.size() - 1; i >= 0; i--)
    {
        std::cout << std::setw(2) << (_resultList.size() - i) << ": "
                  << _resultList[i].URL << " " << _resultList[i].score << " " << _resultList[i].docID << std::endl;
        std::cout << _resultList[i].snippets << std::endl;
        std::cout << std::endl;
    }
}

std::string ResultList::extractSnippets(std::string org, std::string bstr, std::string estr, std::vector<std::string> word_list, std::vector<uint32_t> word_docNum_list)
{
    size_t begin_tag_len = bstr.length();

    size_t start_pos = org.find(bstr) + begin_tag_len;
    size_t end_pos = org.find(estr);
    std::string text = org.substr(start_pos, end_pos - start_pos);
    start_pos = text.find("\n");
    text = text.substr(start_pos);

    std::string snippets;

    if (SNIPPETS_TYPE == LINEAR_SNIPPETS)
    {
        snippets = LinearMatchSnippets(text, word_list);
    }
    else if (SNIPPETS_TYPE == PREFIX_SNIPPETS)
    {
        snippets = PrefixSearchSnippets(text, word_list);
    }
    else if (SNIPPETS_TYPE == BM25_SNIPPETS || SNIPPETS_TYPE == VECTOR_SNIPPETS)
    {
        snippets = ScoreSnippets(text, word_list, word_docNum_list, SNIPPETS_TYPE);
    }
    else if (SNIPPETS_TYPE == KEYWORD_SNIPPETS)
    {
        snippets = KeywordSnippets(text, word_list, word_docNum_list);
    }
    else if (SNIPPETS_TYPE == WEIGHT_SNIPPETS)
    {
        snippets = WeightSnippets(text, word_list, word_docNum_list);
    }
    else if(SNIPPETS_TYPE == DUMP_SNIPPETS)
    {
        std::string snippetPath = SNIPPET_FOLDER;
        snippetPath += "snippet_" + std::to_string(snippet_no++) + ".txt";
        dumpSnippets(snippetPath,text, word_list, word_docNum_list);
    }
    
    return snippets;
}

bool ResultList::in_list(std::string word, std::vector<std::string> &word_list)
{
    for (int i = 0; i < word_list.size(); i++)
    {
        if (word == word_list[i])
        {
            return true;
        }
    }
    return false;
}

// final project functions
void ResultList::ReadFake(std::string fakefile_path, std::vector<std::string> &word_list,
                          std::vector<uint32_t> &word_docNum_list)
{
    std::ifstream infile;
    infile.open(fakefile_path);
    Clear();

    std::string word_line;
    getline(infile, word_line);
    word_list = splitLine(word_line);

    for (int i = 0; i < word_list.size(); i++)
    {
        uint32_t num;
        infile >> num;
        word_docNum_list.push_back(num);
    }

    if (IS_DEBUG)
    {
        std::cout << "word list: ";
        for (int i = 0; i < word_list.size(); i++)
        {
            std::cout << word_list[i] << " " << word_docNum_list[i] << std::endl;
        }
        std::cout << std::endl;
    }

    for (int i = 0; i < RESULT_NUM; i++)
    {
        Result res;
        std::string tmp;
        infile >> res.URL >> res.score >> res.docID;
        _resultList.insert(_resultList.begin(), res);
    }
    infile.close();

    return;
}

std::vector<std::string> ResultList::splitLine(std::string line)
{
    std::vector<std::string> res;
    std::string word;
    for (int i = 0; i < line.length(); i++)
    {
        if (line[i] == ' ')
        {
            res.push_back(word);
            word.clear();
        }
        else
        {
            word += line[i];
        }
    }
    if (word.length())
    {
        res.push_back(word);
    }
    return res;
}
