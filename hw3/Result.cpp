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

void ResultList::FindSnippets(std::vector<std::string> word_list)
{
    std::vector<uint32_t> docID_list;
    for(int i=0;i<_resultList.size();i++)
    {
        docID_list.push_back(_resultList[i].docID);
    }
    sort(docID_list.begin(), docID_list.end()); 

    //read .gz
    gzFile gzfile = gzopen(DATA_SOURCE_PATH, "r");
    int size = INDEX_CHUNK;
    char *buffer = (char *)malloc(size + 1);

    uint32_t docID = 0;
    std::string docContent;
    int pointer = 0;

    if (!gzfile)
    {
        std::cout << "cannot find .trec file" << std::endl;
        return;
    }

    while (pointer<docID_list.size()&&!gzeof(gzfile) )
    {
        int readlen = gzread(gzfile, buffer, size);
        buffer[readlen] = '\0';

        uint32_t docIDNow = docID_list[pointer];
        bool is_find = false;

        docContent += buffer;
        size_t pos;

        while ((pos = docContent.find("</DOC>")) != std::string::npos)
        {
            const int CONST_DOC_LEN = 6; // len of '</DOC>'
            if(docID==docIDNow)
            {
                //get snippets
                std::string snippets = extractSnippets(docContent.substr(0, pos), "<TEXT>\n", "</TEXT>", word_list);
                //update snippets in resultList
                updateDocIDSnippets(docIDNow,snippets);
                pointer += 1;
                if(pointer<docID_list.size()){
                    docIDNow = docID_list[pointer];
                }else{
                    break;
                }
            }
            docContent = docContent.substr(pos + CONST_DOC_LEN);
            docID += 1;
        }
    }

    free(buffer);
    gzclose(gzfile);
    gzfile = NULL;
    
}

void ResultList::updateDocIDSnippets(uint32_t docID, std::string snippets)
{
    for(int i=0;i<_resultList.size();i++)
    {
        if(_resultList[i].docID==docID){
            _resultList[i].snippets = snippets;
            break;
        }
    }
}

std::string ResultList::extractSnippets(std::string org, std::string bstr, std::string estr, std::vector<std::string> word_list)
{
    size_t begin_tag_len = bstr.length();

    size_t start_pos = org.find(bstr);
    size_t end_pos = org.find(estr);
    std::string text = org.substr(start_pos + begin_tag_len, end_pos - start_pos - begin_tag_len);

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