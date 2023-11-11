#include "DataLoader.h"

// Function to decode a Varbyte encoded sequence into a uint32 value
std::vector<uint32_t> decodeVarbyte(const std::vector<uint8_t> &bytes)
{
    std::vector<uint32_t> decodedIntegers;
    int currentInt = 0;
    int shift = 0;

    for (uint8_t byte : bytes)
    {
        currentInt |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
        { // Check the high bit
            decodedIntegers.push_back(currentInt);
            currentInt = 0;
            shift = 0;
        }
        else
        {
            shift += 7;
        }
    }

    return decodedIntegers;
}

DataLoader::DataLoader()
{
}

DataLoader::~DataLoader()
{
}

std::string DataLoader::extractContent(std::string org, std::string bstr, std::string estr)
{
    size_t begin_tag_len = bstr.length();

    size_t start_pos = org.find(bstr);
    size_t end_pos = org.find(estr);

    return org.substr(start_pos + begin_tag_len, end_pos - start_pos - begin_tag_len);
}

std::string DataLoader::getFirstLine(std::string str)
{
    size_t endpos = str.find("\n");
    return str.substr(0, endpos);
}

uint32_t DataLoader::calcWordFreq(std::string text, uint32_t docID)
{
    // delete first row - url
    size_t beginpos = text.find("\n") + 1;
    text = text.substr(beginpos);

    std::string sep = " :;,.\t\v\r\n\f[]{}()<>+-=*&^%$#@!~`\'\"|\\/?·\"：“”";
    std::string word;
    SortedPosting sortedPosting;
    for (size_t i = 0; i < text.length(); i++)
    {
        if (sep.find(text[i]) == std::string::npos)
        {
            word += text[i];
        }
        else
        {
            if (word.length())
            {
                if (!sortedPosting._sortedList.count(word))
                {
                    sortedPosting._sortedList[word] = 1;
                }
                else
                {
                    sortedPosting._sortedList[word] += 1;
                }
            }
            word.clear();
        }
    }

    if (IS_DEBUG & 0)
    {
        sortedPosting.print();
    }

    for (std::map<std::string, uint32_t>::iterator it = sortedPosting._sortedList.begin();
         it != sortedPosting._sortedList.end(); it++)
    {
        _InvertedIndex.Insert(it->first, docID, it->second);
    }
    return sortedPosting._sortedList.size();
}

void DataLoader::ReadData(const char *filepath)
{
    gzFile gzfile = gzopen(filepath, "r");
    int size = INDEX_CHUNK;
    char *buffer = (char *)malloc(size + 1);

    uint32_t docID = 0;
    std::string docContent;

    if (!gzfile)
    {
        std::cout << "cannot find .trec file" << std::endl;
        return;
    }

    while (!gzeof(gzfile))
    {
        z_off_t gp = gztell(gzfile);
        int readlen = gzread(gzfile, buffer, size);
        buffer[readlen] = '\0';

        docContent += buffer;
        size_t pos;

        while ((pos = docContent.find("</DOC>")) != std::string::npos)
        {
            Document doc;
            const int CONST_DOC_LEN = 6; // len of '</DOC>'
            doc.docID = docID;
            doc.docNO = extractContent(docContent.substr(0, pos), "<DOCNO>", "</DOCNO>");
            std::string fullText = extractContent(docContent.substr(0, pos), "<TEXT>\n", "</TEXT>");
            doc.dataLen = fullText.length();
            doc.url = getFirstLine(fullText);
            doc.wordnums = calcWordFreq(fullText, docID);
            doc.gzp = gp;
            _DocTable.add(doc);

            if (IS_DEBUG)
            {
                if (docID % 1000 == 0)
                {
                    std::cout << docID << std::endl;
                };
            }

            docContent = docContent.substr(pos + CONST_DOC_LEN);
            docID += 1;
        }
        // if (IS_DEBUG && docID > 20000)
        // {
        //     break;
        // }
    }

    // if (!_InvertedIndex.HashWord.empty())
    // {
    //     _InvertedIndex.Write();
    //     _InvertedIndex.Clear();
    // }

    if (IS_DEBUG & 0)
    {
        _DocTable.print();
    }

    if (IS_WRITE_PAGE & IS_INDEX)
    {
        clock_t write_page_begin = clock();
        WriteDocTable();
        clock_t write_page_end = clock();
        clock_t write_page_time = write_page_end - write_page_end;
        std::cout << "Write Document Table Need " << double(write_page_time) / 1000000 << "s" << std::endl;
    }

    free(buffer);
    gzclose(gzfile);
    gzfile = NULL;
}

void DataLoader::mergeIndex(uint32_t fileIndex1, uint32_t fileIndex2)
{
    std::string dst_mergePath = _InvertedIndex.getIndexFilePath();
    std::ifstream infile1;
    std::ifstream infile2;
    std::ofstream outfile;
    std::string file1 = _InvertedIndex.getIndexFilePath(fileIndex1);
    std::string file2 = _InvertedIndex.getIndexFilePath(fileIndex2);
    std::cout << fileIndex1 << " " << fileIndex2 << " " << dst_mergePath << std::endl;

    if (FILEMODE == FILEMODE_ASCII)
    {
        infile1.open(file1);
        infile2.open(file2);
        outfile.open(dst_mergePath);
    }
    else if (FILEMODE == FILEMODE_BIN)
    {
        infile1.open(file1, std::ifstream::binary);
        infile2.open(file2, std::ifstream::binary);
        outfile.open(dst_mergePath, std::ofstream::binary);
    }
    std::string line1, line2;
    std::string word1, word2;
    while (infile1 && infile2)
    {
        if (word1.empty() && word2.empty())
        {
            std::getline(infile1, line1);
            std::getline(infile2, line2);
            word1 = line1.substr(0, line1.find(":"));
            word2 = line2.substr(0, line2.find(":"));
        }
        if (word1.compare(word2) == 0)
        {
            // merge word1 and word2
            std::string arr1 = line1.substr(line1.find(":") + 1);
            std::string arr2 = line2.substr(line2.find(":") + 1);
            std::string docID1 = arr1.substr(0, arr1.find(" "));
            std::string docID2 = arr2.substr(0, arr2.find(" "));
            if (stoi(docID1) < stoi(docID2))
            {
                outfile << word1 << ":" << arr1 << "," << arr2 << std::endl;
            }
            else
            {
                outfile << word1 << ":" << arr2 << "," << arr1 << std::endl;
            }

            // update line1 and line2
            std::getline(infile1, line1);
            std::getline(infile2, line2);
            word1 = line1.substr(0, line1.find(":"));
            word2 = line2.substr(0, line2.find(":"));
        }
        else if (word1.compare(word2) < 0)
        {
            // write line1 only
            outfile << line1 << std::endl;
            // update line1
            std::getline(infile1, line1);
            word1 = line1.substr(0, line1.find(":"));
        }
        else
        {
            // write line2 only
            outfile << line2 << std::endl;
            // update line2
            std::getline(infile2, line2);
            word2 = line2.substr(0, line2.find(":"));
        }
    }

    if (infile1)
    {
        outfile << line1 << std::endl;
    }
    if (infile2)
    {
        outfile << line2 << std::endl;
    }

    while (infile1)
    {
        std::getline(infile1, line1);
        outfile << line1 << std::endl;
    }
    while (infile2)
    {
        std::getline(infile2, line2);
        outfile << line2 << std::endl;
    }
    infile1.close();
    infile2.close();
    outfile.close();
}

void DataLoader::mergeIndexToOne()
{
    uint32_t mergedIndexNum = 0;
    uint32_t leftIndexNum;

    while ((leftIndexNum = _InvertedIndex.indexFileNum - mergedIndexNum) > 1)
    {
        mergeIndex(mergedIndexNum, mergedIndexNum + 1);
        mergedIndexNum += 2;
    }
}

void DataLoader::BuildLexicon()
{
    std::string path = _InvertedIndex.getIndexFilePath(_InvertedIndex.indexFileNum - 1);
    _Lexicon.Build(path);
}

void DataLoader::WriteDocTable()
{
    _DocTable.Write();
}

void DataLoader::WriteLexicon()
{
    _Lexicon.Write();
}

// calculate BM25
double DataLoader::BM25_t_q(std::string term, uint32_t docID, uint32_t freq)
{
    double k1 = 1.2;
    double b = 0.75;
    Document doc = _DocTable._DocTable[docID];
    double K = k1 * ((1 - b) + b * doc.dataLen / _DocTable._avg_data_len);
    int N = _DocTable._totalDoc;
    double f_t = _Lexicon._lexiconList[term].docNum;
    uint32_t f_dt = freq;
    double score = 0;
    score = log((N - f_t + 0.5) / (f_t + 0.5)) * (k1 + 1) * f_dt / (K + f_dt);
    return score;
}


void DataLoader::openList(uint32_t offset, uint32_t &metadata_size,
                          std::vector<uint32_t> &lastdocID_list, std::vector<uint32_t> &docIDsize_list,
                          std::vector<uint32_t> &freqSize_list)
{
    lastdocID_list.clear();
    docIDsize_list.clear();
    freqSize_list.clear();
    int index_fd = open(_Lexicon.IndexPath.c_str(), O_RDONLY);
    off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    size_t length = MAX_METASIZE;
    struct stat sb;
    if (offset + length > sb.st_size)
        length = sb.st_size - offset;
    // Execute mmap
    char *ret = (char *)mmap(NULL, length + offset - pa_offset, PROT_READ, MAP_PRIVATE, index_fd, pa_offset);
    if (ret == MAP_FAILED)
    {
        close(index_fd);
        perror("mmap");
        return;
    }

    // read metadata
    int startIndex = offset - pa_offset;
    std::memcpy(&metadata_size, ret + startIndex, sizeof(metadata_size));
    startIndex += sizeof(metadata_size);
    for (int i = 0; i < metadata_size; i++)
    {
        uint32_t lastdocID;
        std::memcpy(&lastdocID, ret + startIndex, sizeof(lastdocID));
        lastdocID_list.push_back(lastdocID);
        startIndex += sizeof(lastdocID);
    }
    for (int i = 0; i < metadata_size; i++)
    {
        uint32_t docIDsize;
        std::memcpy(&docIDsize, ret + startIndex, sizeof(docIDsize));
        docIDsize_list.push_back(docIDsize);
        startIndex += sizeof(docIDsize);
    }
    for (int i = 0; i < metadata_size; i++)
    {
        uint32_t freqSize;
        std::memcpy(&freqSize, ret + startIndex, sizeof(freqSize));
        freqSize_list.push_back(freqSize);
        startIndex += sizeof(freqSize);
    }
    munmap(ret, length + offset - pa_offset);
    close(index_fd);
}

uint32_t DataLoader::getFreq(std::string term, uint32_t docID)
{
    return 0;
}

void DataLoader::TAATQuery(std::vector<std::string> word_list, int type)
{
    _resultList.Clear();

    if (type == DISJUNCTIVE)
    {
        std::vector<double> score_array(_DocTable._totalDoc, 0);
        for (int i = 0; i < word_list.size(); i++)
        {
            std::string term = word_list[i];
            try
            {
                decodeBlocks(term, score_array);
            }
            catch (...)
            {
                std::cerr << "exception" << std::endl;
            }
        }
        findTopKscores(score_array, RESULT_NUM);
    }
    else if (type == CONJUNCTIVE)
    {
        // find minterm
        std::string minterm = word_list[0];
        uint32_t mindocNum = _Lexicon._lexiconList[minterm].docNum;
        for (int i = 1; i < word_list.size(); i++)
        {
            std::string term = word_list[i];
            if (_Lexicon._lexiconList[term].docNum < mindocNum)
            {
                minterm = term;
                mindocNum = _Lexicon._lexiconList[term].docNum;
            }
        }

        // build score_hash
        std::map<uint32_t, double> score_hash;
        decodeBlocks(minterm, score_hash, true);

        // update score_hash
        for (std::string word : word_list)
        {
            if (word == minterm)
                continue;
            try
            {
                updateScoreHash(word, score_hash, false);
            }
            catch (...)
            {
                std::cerr << "exception" << std::endl;
            }
        }

        findTopKscores(score_hash, RESULT_NUM);
    }
    // index_infile.close();
}

std::vector<std::string> DataLoader::splitQuery(std::string query)
{
    std::vector<std::string> word_list;
    std::string sep = " :;,.\t\v\r\n\f[]{}()<>+-=*&^%$#@!~`\'\"|\\/?·\"：“”";
    std::string word;

    for (size_t i = 0; i < query.length(); i++)
    {
        if (sep.find(query[i]) == std::string::npos)
        {
            word += query[i];
        }
        else
        {
            if (word.length())
            {
                word_list.push_back(word);
            }
            word.clear();
        }
    }
    if (word.length())
    {
        word_list.push_back(word);
    }
    return word_list;
}
void DataLoader::TestQuery()
{
    std::string query = "cat dog mouse";
    clock_t query_start = clock();
    std::vector<std::string> word_list = splitQuery(query);
    if (!word_list.size())
    {
        std::cout << "unlegal query" << std::endl;
        return;
    }
    TAATQuery(word_list, 0);
    clock_t query_end = clock();
    clock_t query_time = query_end - query_start;
    std::cout << "search using " << std::setiosflags(std::ios::fixed)
              << std::setprecision(2) << double(query_time) / 1000000 << "s" << std::endl;
    clock_t snippet_begin = clock();
    updateSnippets(word_list);
    clock_t snippet_end = clock();
    clock_t snippet_time = snippet_end - snippet_begin;
    std::cout << "find snippets using " << double(snippet_time) / 1000000 << "s" << std::endl;
    _resultList.Print();
}

void DataLoader::QueryLoop()
{
    std::cout << "Welcome to my search engine!" << std::endl;
    std::cout << "input 'exit' to exit." << std::endl;
    std::cout << "you can input a query like 'hello world'." << std::endl;
    std::cout << "you can select search type, conjunctive(0) or disjunctive(1) queries." << std::endl;
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(2);

    while (true)
    {
        std::cout << "query>>";
        std::string query;
        getline(std::cin, query);
        if (query == "exit")
        {
            break;
        }
        std::string typestr;
        int type;
        std::cout << "conjunctive(0) or disjunctive(1)>>";
        getline(std::cin, typestr);

        if (typestr == "0" || typestr == "conjunctive")
        {
            type = CONJUNCTIVE;
        }
        else if (typestr == "1" || typestr == "conjunctive")
        {
            type = DISJUNCTIVE;
        }
        else
        {
            std::cout << "cannot recognize query type" << std::endl;
            continue;
        }

        clock_t query_start = clock();
        std::vector<std::string> word_list = splitQuery(query);
        if (!word_list.size())
        {
            std::cout << "unlegal query" << std::endl;
            continue;
        }
        TAATQuery(word_list, type);
        clock_t query_end = clock();
        clock_t query_time = query_end - query_start;
        std::cout << "search using " << double(query_time) / 1000000 << "s" << std::endl;
        clock_t snippet_begin = clock();
        updateSnippets(word_list);
        clock_t snippet_end = clock();
        clock_t snippet_time = snippet_end - snippet_begin;
        std::cout << "find snippets using " << double(snippet_time) / 1000000 << "s" << std::endl;

        _resultList.Print();
    }
}

void DataLoader::decodeBlocks(std::string term, std::vector<double> &score_array)
{
    uint32_t beginp = _Lexicon._lexiconList[term].beginp;
    uint32_t endp = _Lexicon._lexiconList[term].endp;
    uint32_t block_num = _Lexicon._lexiconList[term].blockNum;
    for (int i = 0; i < block_num; i++)
    {
        decodeBlock(term, beginp, score_array);
    }
}

void DataLoader::decodeBlock(std::string term, uint32_t &beginp, std::vector<double> &score_array)
{
    uint32_t metadata_size;
    std::vector<uint32_t> lastdocID_list, docIDsize_list, freqSize_list;
    openList(beginp, metadata_size, lastdocID_list, docIDsize_list, freqSize_list);

    // decode chunk
    std::vector<uint32_t> docID64, freq64;
    uint32_t metabyte = 4 + 3 * (metadata_size)*4;
    uint32_t docIDp = beginp + metabyte;
    uint32_t freqp = docIDp + docIDsize_list[0];

    for (int i = 0; i < metadata_size; i++)
    {
        docID64 = decodeChunk(docIDp, docIDp + docIDsize_list[i]);
        freq64 = decodeChunk(freqp, freqp + freqSize_list[i]);
        if (i != metadata_size - 1)
        {
            docIDp += docIDsize_list[i] + freqSize_list[i];
            freqp += freqSize_list[i] + docIDsize_list[i + 1];
        }

        uint32_t prev_docID = 0;
        for (int i = 0; i < docID64.size(); i++)
        {
            prev_docID += docID64[i];
            score_array[prev_docID] += BM25_t_q(term, prev_docID, freq64[i]);
        }
    }
    
    beginp += calcMetaSize(metadata_size, lastdocID_list, docIDsize_list, freqSize_list);
}

std::vector<uint32_t> DataLoader::decodeChunk(uint32_t offset, uint32_t endp)
{
    int index_fd = open(_Lexicon.IndexPath.c_str(), O_RDONLY);
    off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    size_t length = endp - offset;
    struct stat sb;
    if (offset + length > sb.st_size)
        length = sb.st_size - offset;
    // Execute mmap
    char *ret = (char *)mmap(NULL, length + offset - pa_offset, PROT_READ, MAP_PRIVATE, index_fd, pa_offset);
    if (ret == MAP_FAILED)
    {
        close(index_fd);
        perror("mmap");
    }

    std::vector<uint32_t> decodedIntegers;
    uint32_t currentInt = 0;
    int shift = 0;
    uint8_t byte;
    int startIndex = offset - pa_offset;

    for (uint32_t i = offset; i < endp; i++)
    {
        std::memcpy(&byte, ret + startIndex, sizeof(byte));
        startIndex += sizeof(byte);
        currentInt |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
        { // Check the high bit
            decodedIntegers.push_back(currentInt);
            currentInt = 0;
            shift = 0;
        }
        else
        {
            shift += 7;
        }
    }

    munmap(ret, length + offset - pa_offset);
    close(index_fd);

    return decodedIntegers;
}

void DataLoader::findTopKscores(std::vector<double> &score_array, int k)
{
    struct QueueDouble
    {
        double value;
        uint32_t index;
        QueueDouble(double v, uint32_t i) : value(v), index(i) {}

        // Overload the comparison operator for the priority queue
        bool operator<(const QueueDouble &other) const
        {
            return -value < -other.value;
        }
    };

    std::priority_queue<QueueDouble> maxQueue;

    for (int i = 0; i < score_array.size(); i++)
    {
        maxQueue.push(QueueDouble(score_array[i], i));
        if (maxQueue.size() > k)
        {
            maxQueue.pop();
        }
    }

    // The top k elements in the maxQueue are the maximum ten numbers
    while (!maxQueue.empty())
    {

        uint32_t docID = maxQueue.top().index;
        double score = maxQueue.top().value;
        _resultList.Insert(docID, _DocTable._DocTable[docID].url, score, "");
        maxQueue.pop();
    }
}

void DataLoader::decodeBlocks(std::string term, std::map<uint32_t, double> &score_hash, bool is_init)
{
    uint32_t beginp = _Lexicon._lexiconList[term].beginp;
    uint32_t endp = _Lexicon._lexiconList[term].endp;
    uint32_t block_num = _Lexicon._lexiconList[term].blockNum;

    for (int i = 0; i < block_num; i++)
    {
        decodeBlock(term, beginp, score_hash, is_init);
    }
}

void DataLoader::decodeBlock(std::string term, uint32_t &beginp,
                             std::map<uint32_t, double> &score_hash, bool is_init)
{
    uint32_t metadata_size;
    std::vector<uint32_t> lastdocID_list, docIDsize_list, freqSize_list;
    openList(beginp, metadata_size, lastdocID_list, docIDsize_list, freqSize_list);

    // decode chunk
    std::vector<uint32_t> docID64, freq64;
    uint32_t metabyte = 4 + 3 * (metadata_size)*4;
    uint32_t docIDp = beginp + metabyte;
    uint32_t freqp = docIDp + docIDsize_list[0];

    for (int i = 0; i < metadata_size; i++)
    {
        docID64 = decodeChunk(docIDp, docIDp + docIDsize_list[i]);
        freq64 = decodeChunk(freqp, freqp + freqSize_list[i]);
        if (i != metadata_size - 1)
        {
            docIDp += docIDsize_list[i] + freqSize_list[i];
            freqp += freqSize_list[i] + docIDsize_list[i + 1];
        }

        // init score
        if (is_init)
        {
            uint32_t prev_docID = 0;
            for (int i = 0; i < docID64.size(); i++)
            {
                prev_docID += docID64[i];
                score_hash[prev_docID] = BM25_t_q(term, prev_docID, freq64[i]);
            }
        }
    }

    beginp += calcMetaSize(metadata_size, lastdocID_list, docIDsize_list, freqSize_list);
}

void DataLoader::findTopKscores(std::map<uint32_t, double> &score_hash, int k)
{
    struct QueueDouble
    {
        double value;
        uint32_t index;
        QueueDouble(double v, uint32_t i) : value(v), index(i) {}

        // Overload the comparison operator for the priority queue
        bool operator<(const QueueDouble &other) const
        {
            return -value < -other.value;
        }
    };

    std::priority_queue<QueueDouble> maxQueue;

    for (std::map<uint32_t, double>::iterator it = score_hash.begin(); it != score_hash.end(); ++it)
    {

        maxQueue.push(QueueDouble(it->second, it->first));
        if (maxQueue.size() > k)
        {
            maxQueue.pop();
        }
    }

    // The top k elements in the maxQueue are the maximum ten numbers
    while (!maxQueue.empty())
    {
        uint32_t docID = maxQueue.top().index;
        double score = maxQueue.top().value;

        _resultList.Insert(docID, _DocTable._DocTable[docID].url, score, "");
        maxQueue.pop();
    }
}
uint32_t DataLoader::calcMetaSize(uint32_t metadata_size, std::vector<uint32_t> &lastdocID_list,
                                  std::vector<uint32_t> &docIDsize_list, std::vector<uint32_t> &freqSize_list)
{
    uint32_t size = 0;
    size += sizeof(metadata_size) + 3 * sizeof(uint32_t) * metadata_size;
    for (int i = 0; i < metadata_size; i++)
    {
        size += docIDsize_list[i] + freqSize_list[i];
    }
    return size;
}
void DataLoader::updateScoreHash(std::string term, std::map<uint32_t, double> &score_hash, bool is_init)
{
    uint32_t beginp = _Lexicon._lexiconList[term].beginp;
    uint32_t endp = _Lexicon._lexiconList[term].endp;
    uint32_t block_num = _Lexicon._lexiconList[term].blockNum;

    uint32_t metadata_size;
    std::vector<uint32_t> lastdocID_list, docIDsize_list, freqSize_list;
    openList(beginp, metadata_size, lastdocID_list, docIDsize_list, freqSize_list);
    int index = 0;
    int docIDp = 0;
    std::vector<uint32_t> docID64, freq64;
    bool needDecode = true;

    for (std::map<uint32_t, double>::iterator it = score_hash.begin(); beginp < endp && it != score_hash.end();)
    {
        uint32_t nextdocID = (std::next(it) == score_hash.end()) ? MAX_DOCID : std::next(it)->first;
        uint32_t docID = it->first;

        if (lastdocID_list.back() < docID)
        {
            beginp += calcMetaSize(metadata_size, lastdocID_list, docIDsize_list, freqSize_list);
            if (beginp < endp)
            {
                openList(beginp, metadata_size, lastdocID_list, docIDsize_list, freqSize_list);
                index = 0;
                needDecode = true;
            }
            else
            {
                break;
            }
            continue;
        }
        else
        {
            // open block and find if docID in
            // 1. find docID block index
            int previndex = index;
            for (; index < lastdocID_list.size(); index++)
            {
                if (lastdocID_list[index] >= docID)
                    break;
            }

            if (previndex != index)
            {
                needDecode = true;
            }

            // decode chunk
            if (needDecode)
            {
                uint32_t metabyte = 4 + 3 * (metadata_size)*4;
                uint32_t docIDp = beginp + metabyte;
                uint32_t freqp = docIDp + docIDsize_list[0];
                for (int i = 0; i < index; i++)
                {
                    docIDp += docIDsize_list[i] + freqSize_list[i];
                    freqp += freqSize_list[i] + docIDsize_list[i + 1];
                }
                docID64 = decodeChunk(docIDp, docIDp + docIDsize_list[index]);
                uint32_t prevdocID = 0;
                for (int i = 0; i < docID64.size(); i++)
                {
                    prevdocID += docID64[i];
                    docID64[i] = prevdocID;
                }
                freq64 = decodeChunk(freqp, freqp + freqSize_list[index]);
                docIDp = 0;
            }

            // find if docID in

            for (; docIDp < docID64.size(); docIDp++)
            {

                if (docID64[docIDp] == docID)
                {
                    score_hash[docID] += BM25_t_q(term, docID, freq64[docIDp]);
                    break;
                }
                // end early, maintain beginp
                if (docID64[docIDp] >= nextdocID)
                {
                    break;
                }
            }
            it++;
        }
    }
}

void DataLoader::updateSnippets(std::vector<std::string> word_list)
{
    for (int i = 0; i < _resultList._resultList.size(); i++)
    {
        std::string s = findSnippets(_resultList._resultList[i].docID, word_list);
        _resultList._resultList[i].snippets = s;
    }
}

std::string DataLoader::findSnippets(uint32_t docID, std::vector<std::string> word_list)
{
    // read .gz
    std::ifstream infile;
    infile.open(SNIPPETS_SOURCE_PATH);
    infile.seekg(_DocTable._DocTable[docID].gzp);
    std::string snippets;

    size_t size = docID + 1 < _DocTable._DocTable.size() ? _DocTable._DocTable[docID + 1].gzp - _DocTable._DocTable[docID].gzp : INDEX_CHUNK;
    char *buffer = (char *)malloc(size + 1);
    infile.read(buffer, size + 1);
    buffer[size] = '\0';
    std::string docContent;
    docContent = buffer;

    snippets = _resultList.extractSnippets(docContent, "<TEXT>\n", "</TEXT>", word_list);

    free(buffer);
    return snippets;
}
