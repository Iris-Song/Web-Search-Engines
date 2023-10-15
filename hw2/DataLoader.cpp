#include "DataLoader.h"

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

void DataLoader::read_data(const char *filepath)
{
    gzFile gzfile = gzopen(filepath, "r");
    int size = INDEX_CHUNK;
    char *buffer = (char *)malloc(size + 1);

    uint32_t docID = 0;
    std::string docContent;

    while (!gzeof(gzfile))
    {
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
            doc.dataLen = sizeof(fullText);
            doc.url = getFirstLine(fullText);
            doc.wordnums = calcWordFreq(fullText, docID);
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
            // break;
        }
        // break;
        if (IS_DEBUG && docID > 20000)
        {
            break;
        }
    }

    if (!_InvertedIndex.HashWord.empty())
    {
        _InvertedIndex.Write();
        _InvertedIndex.Clear();
    }

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
            if (docID1 < docID2)
            {
                outfile << word1 << ":" << arr1 << " " << arr2 << std::endl;
            }
            else
            {
                outfile << word1 << ":" << arr2 << " " << arr1 << std::endl;
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
    while ((leftIndexNum = _InvertedIndex.indexFileNum - mergedIndexNum) > 2)
    {
        mergeIndex(mergedIndexNum, mergedIndexNum + 1);
        mergedIndexNum += 2;
    }
    if (leftIndexNum == 2)
    {
        std::string path1 = _InvertedIndex.getIndexFilePath(mergedIndexNum);
        std::string path2 = _InvertedIndex.getIndexFilePath(mergedIndexNum + 1);
        _Lexicon.Build(path1, path2);
    }
}

void DataLoader::WriteDocTable()
{
    _DocTable.Write();
}

void DataLoader::WriteLexicon()
{
    _Lexicon.Write();
}