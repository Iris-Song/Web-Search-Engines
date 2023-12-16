#include "Snippets.h"

bool in_list(std::string word, std::vector<std::string> &word_list)
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

bool prefix_in_list(std::string word, std::vector<std::string> &word_list)
{
    for (int i = 0; i < word_list.size(); i++)
    {
        if (word.find(word_list[i]) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

std::string concatDeque(std::deque<std::string> &word_deque)
{
    std::string sep = ":;,.[]{}()<>+-=*&^%$#@!~`\'\"|\\/?·\"：“”";
    std::string res;
    for (int i = 0; i < word_deque.size(); ++i)
    {
        if (sep.find(word_deque[i][word_deque[i].length() - 1]) == std::string::npos)
        {
            res += word_deque[i] + " ";
        }
        else
        {
            res += word_deque[i];
        }
    }
    return res;
}

std::string LinearMatchSnippets(std::string text, std::vector<std::string> &word_list)
{
    std::string snippets;
    std::string sep = SEPARATOR;
    std::string ed = "\t\v\r\n\f ";
    std::string word;

    bool findOne = false;
    bool findTwo = false;
    bool containfirst = true;
    int deque_max_size = TERM_NUM + 1;
    std::deque<std::string> word_deque;

    for (int i = 0; i < text.length(); i++)
    {
        if (sep.find(text[i]) == std::string::npos)
        {
            word += text[i];
        }
        else
        {
            if (word.length() && ed.find(text[i]) == std::string::npos)
            {
                word += text[i];
            }

            if (word_deque.size() < deque_max_size)
            {
                word_deque.push_back(word);
            }
            else
            {
                word_deque.pop_front();
                containfirst = false;
                word_deque.push_back(word);
                if (findOne && !findTwo && !snippets.length())
                {
                    if (!containfirst)
                    {
                        snippets += "... ";
                    }
                    snippets += concatDeque(word_deque) + " ...";
                    word_deque.clear();
                    deque_max_size = TERM_NUM;
                }
                else if (findTwo)
                {
                    if (!containfirst && !snippets.length())
                    {
                        snippets += "... ";
                    }
                    snippets += concatDeque(word_deque) + "...";
                    return snippets;
                }
            }

            if (in_list(word, word_list))
            {
                if (!findOne)
                {
                    findOne = true;
                    deque_max_size = word_deque.size() + TERM_NUM;
                }
                else if (!findTwo)
                {
                    findTwo = true;
                    deque_max_size = word_deque.size() + TERM_NUM;
                }
            }
            word.clear();
        }
    }

    return snippets;
}

std::string PrefixSearchSnippets(std::string text, std::vector<std::string> &word_list)
{
    std::string snippets;
    std::string sep = SEPARATOR;
    std::string ed = "\t\v\r\n\f ";
    std::string word;

    bool findOne = false;
    bool findTwo = false;
    bool containfirst = true;
    int deque_max_size = TERM_NUM + 1;
    std::deque<std::string> word_deque;

    for (int i = 0; i < text.length(); i++)
    {
        if (sep.find(text[i]) == std::string::npos)
        {
            word += text[i];
        }
        else
        {
            if (word.length() && ed.find(text[i]) == std::string::npos)
            {
                word += text[i];
            }

            if (word_deque.size() < deque_max_size)
            {
                word_deque.push_back(word);
            }
            else
            {
                word_deque.pop_front();
                containfirst = false;
                word_deque.push_back(word);
                if (findOne && !findTwo && !snippets.length())
                {
                    if (!containfirst)
                    {
                        snippets += "... ";
                    }
                    snippets += concatDeque(word_deque) + " ...";
                    word_deque.clear();
                    deque_max_size = TERM_NUM;
                }
                else if (findTwo)
                {
                    if (!containfirst && !snippets.length())
                    {
                        snippets += "... ";
                    }
                    snippets += concatDeque(word_deque) + "...";
                    return snippets;
                }
            }

            if (prefix_in_list(word, word_list))
            {
                if (!findOne)
                {
                    findOne = true;
                    deque_max_size = word_deque.size() + TERM_NUM;
                }
                else if (!findTwo)
                {
                    findTwo = true;
                    deque_max_size = word_deque.size() + TERM_NUM;
                }
            }
            word.clear();
        }
    }

    return snippets;
}

std::map<std::string, std::vector<size_t>> getWordPos(std::string text, std::vector<std::string> &word_list)
{
    std::string sep = SEPARATOR;
    std::map<std::string, std::vector<size_t>> word_pos;
    std::string word;

    // init hashtable
    for (int i = 0; i < word_list.size(); i++)
    {
        word_pos[word_list[i]] = std::vector<size_t>();
    }

    for (int i = 0; i < text.length(); i++)
    {
        if (sep.find(text[i]) == std::string::npos)
        {
            word += text[i];
        }
        else
        {
            if (word.length() && word_pos.count(word))
            {
                word_pos[word].push_back(i - word.length());
            }
            word.clear();
        }
    }

    return word_pos;
}

double BM25_t_q(uint32_t freq, uint32_t docNum, uint32_t dataLen)
{
    double k1 = 1.2;
    double b = 0.75;

    double K = k1 * ((1 - b) + b * dataLen / DOC_AVG_LEN);
    int N = DOC_NUM;
    double f_t = docNum;
    uint32_t f_dt = freq;
    double score = 0;
    score = log((N - f_t + 0.5) / (f_t + 0.5)) * (k1 + 1) * f_dt / (K + f_dt);
    return score > 0 ? score : 0;
}

double vector_t_q(uint32_t freq, uint32_t docNum, uint32_t dataLen)
{
    int N = DOC_NUM;
    double f_t = docNum;
    double w_d_t = 1 + log(freq);
    double w_q_t = log(1 + N / f_t);
    double score = w_d_t * w_q_t / sqrt(dataLen);
    return score;
}

std::map<std::string, double> getWordScore(std::vector<std::string> &word_list, std::map<std::string, std::vector<size_t>> &word_pos, std::vector<uint32_t> &word_docNum_list, uint32_t dataLen, int type = BM25_SNIPPETS)
{
    std::map<std::string, double> word_score;
    for (int i = 0; i < word_list.size(); i++)
    {
        std::string term = word_list[i];
        if (type == BM25_SNIPPETS)
        {
            word_score[term] = BM25_t_q(word_pos[term].size(), word_docNum_list[i], dataLen);
        }
        else if (type == VECTOR_SNIPPETS)
        {
            word_score[term] = vector_t_q(word_pos[term].size(), word_docNum_list[i], dataLen);
        }
    }
    return word_score;
}

std::string getSnippetsByPos(std::string text, std::vector<size_t> pos)
{
    std::string res;
    std::string sep = SEPARATOR;
    std::string ed = "\t\v\r\n\f ";
    size_t endpos = pos.back();
    if (endpos - pos[0] < SNIPPETS_RANGE * 2)
    {
        endpos = pos[0] + SNIPPETS_RANGE * 2;
    }
    for (; endpos < text.length(); endpos++)
    {
        if (sep.find(text[endpos]) != std::string::npos)
        {
            break;
        }
    }

    for (size_t i = pos[0]; i < endpos; i++)
    {
        if (ed.find(text[i]) != std::string::npos)
        {
            if (res.length() > MAX_SNIPPETS)
                break;
            res += " ";
        }

        else
            res += text[i];
    }

    if (pos[0] != 0)
    {
        res = "..." + res;
    }
    return res;
}

std::string ScoreSnippets(std::string text, std::vector<std::string> &word_list, std::vector<uint32_t> &word_docNum_list, int type)
{
    std::string snippets;

    // get position of each word in word list
    std::map<std::string, std::vector<size_t>> word_pos = getWordPos(text, word_list);
    // get score of each word in word list occur 1 time
    std::map<std::string, double> word_score = getWordScore(word_list, word_pos, word_docNum_list, text.length(), type);

    if (IS_DEBUG & 0)
    {
        for (std::map<std::string, double>::const_iterator it = word_score.begin();
             it != word_score.end(); ++it)
        {
            std::cout << it->first << " " << it->second << "\n";
        }
    }

    // get pos in order and cluster
    struct QueuePos
    {
        std::string term;
        size_t value; // pos
        int index;
        QueuePos(size_t v, uint32_t i, std::string t) : value(v), index(i), term(t) {}

        // Overload the comparison operator for the priority queue
        bool operator<(const QueuePos &other) const
        {
            return -value < -other.value;
        }
    };

    std::priority_queue<QueuePos> minQueue;

    const int CLUSTER_SIZE = 2;
    struct QueueCluster
    {
        double score; // BM25
        std::vector<size_t> pos;

        QueueCluster(double v, std::vector<size_t> p) : score(v), pos(p) {}

        // Overload the comparison operator for the priority queue
        bool operator<(const QueueCluster &other) const
        {
            return -score < -other.score;
        }
    };

    std::priority_queue<QueueCluster> maxCluster;
    QueueCluster qc(0, std::vector<size_t>());

    for (std::map<std::string, std::vector<size_t>>::const_iterator it = word_pos.begin();
         it != word_pos.end(); ++it)
    {
        if (it->second.size() > 0)
            minQueue.push(QueuePos(it->second[0], 0, it->first));
    }

    while (!minQueue.empty())
    {
        std::string term = minQueue.top().term;
        int index = minQueue.top().index;
        size_t val = minQueue.top().value;
        // std::cout << minQueue.top().value << " ";
        minQueue.pop();
        if (index + 1 < word_pos[term].size())
        {
            minQueue.push(QueuePos(word_pos[term][index + 1], index + 1, term));
        }

        if (qc.pos.size() == 0 || val - qc.pos.back() < SNIPPETS_RANGE)
        {
            qc.pos.push_back(val);
            qc.score += word_score[term];
        }
        else
        {
            maxCluster.push(qc);
            qc.pos = std::vector<size_t>(1, val);
            qc.score = word_score[term];
            if (maxCluster.size() > CLUSTER_SIZE)
            {
                maxCluster.pop();
            }
        }
    }

    if (qc.pos.size() > 0)
    {
        maxCluster.push(qc);
        if (maxCluster.size() > CLUSTER_SIZE)
        {
            maxCluster.pop();
        }
    }

    // if (IS_DEBUG)
    // {
    //     while (!maxCluster.empty())
    //     {
    //         std::vector<size_t> pos = maxCluster.top().pos;
    //         double score = maxCluster.top().score;
    //         std::cout<<"[";
    //         for(int i=0;i<pos.size();i++){
    //             std::cout<<pos[i]<<" ";
    //         }
    //         std::cout<<"]";
    //         maxCluster.pop();
    //     }
    //     std::cout<<std::endl;
    // }

    // get snippets by pos
    while (!maxCluster.empty())
    {
        std::vector<size_t> pos = maxCluster.top().pos;
        double score = maxCluster.top().score;
        maxCluster.pop();
        if (snippets.length() > MAX_SNIPPETS)
            break;
        snippets += getSnippetsByPos(text, pos);
    }
    if(snippets.length())
    {
        snippets += "...";
    }

    return snippets;
}

std::map<std::string, double> getWordWeight(std::vector<std::string> &word_list, std::map<std::string, std::vector<size_t>> &word_pos, std::vector<uint32_t> &word_docNum_list)
{
    std::map<std::string, double> word_weight;
    for (int i = 0; i < word_list.size(); i++)
    {
        std::string term = word_list[i];
        word_weight[term] = word_pos[term].size() * log(RESULT_NUM / double(word_docNum_list[i]));
        std::cout << term << " " << word_weight[term] << std::endl;
    }
    return word_weight;
}

std::map<std::string, double> getKeywordWeight(std::map<std::string, double> &word_weight)
{
    int disambNum = std::max(int(ceil(word_weight.size() * KEYWORD_PERCENT)), MIN_KEYWORD);

    // select disambNum term
    struct QueueWeight
    {
        std::string term;
        double weight;
        QueueWeight(double w, std::string t) : weight(w), term(t) {}

        // Overload the comparison operator for the priority queue
        bool operator<(const QueueWeight &other) const
        {
            return -weight < -other.weight;
        }
    };

    std::priority_queue<QueueWeight> maxWeightTermQueue;

    for (std::map<std::string, double>::const_iterator it = word_weight.begin();
         it != word_weight.end(); ++it)
    {
        maxWeightTermQueue.push(QueueWeight(it->second, it->first));
        if (maxWeightTermQueue.size() > disambNum)
        {
            maxWeightTermQueue.pop();
        }
    }

    std::map<std::string, double> keyword_weight;

    while (!maxWeightTermQueue.empty())
    {
        std::string term = maxWeightTermQueue.top().term;
        std::cout << term << "*";
        double weight = maxWeightTermQueue.top().weight;

        keyword_weight[term] = weight;
        maxWeightTermQueue.pop();
    }

    return keyword_weight;
}

std::string WeightSnippets(std::string text, std::vector<std::string> &word_list, std::vector<uint32_t> &word_docNum_list)
{
    std::string snippets;

    // get position of each word in word list
    std::map<std::string, std::vector<size_t>> word_pos = getWordPos(text, word_list);
    // get weight of each term
    std::map<std::string, double> word_weight = getWordWeight(word_list, word_pos, word_docNum_list);
    // get weight of each keyword
    std::map<std::string, double> keyword_weight = getKeywordWeight(word_weight);

    // get pos in order and cluster
    struct QueuePos
    {
        std::string term;
        size_t value; // pos
        int index;
        QueuePos(size_t v, uint32_t i, std::string t) : value(v), index(i), term(t) {}

        // Overload the comparison operator for the priority queue
        bool operator<(const QueuePos &other) const
        {
            return -value < -other.value;
        }
    };

    std::priority_queue<QueuePos> minQueue;

    const int CLUSTER_SIZE = 2;
    struct QueueCluster
    {
        double score; 
        std::vector<size_t> pos;

        QueueCluster(double v, std::vector<size_t> p) : score(v), pos(p) {}

        // Overload the comparison operator for the priority queue
        bool operator<(const QueueCluster &other) const
        {
            return -score < -other.score;
        }
    };

    std::priority_queue<QueueCluster> maxCluster;
    QueueCluster qc(0, std::vector<size_t>());

    for (std::map<std::string, double>::const_iterator it = keyword_weight.begin();
         it != keyword_weight.end(); ++it)
    {
        std::string term = it->first;
        if (word_pos[term].size() > 0)
            minQueue.push(QueuePos(word_pos[term][0], 0, term));
    }

    while (!minQueue.empty())
    {
        std::string term = minQueue.top().term;
        int index = minQueue.top().index;
        size_t val = minQueue.top().value;
        minQueue.pop();
        if (index + 1 < word_pos[term].size())
        {
            minQueue.push(QueuePos(word_pos[term][index + 1], index + 1, term));
        }

        if (qc.pos.size() == 0 || val - qc.pos.back() < SNIPPETS_RANGE)
        {
            qc.pos.push_back(val);
            qc.score += keyword_weight[term];
        }
        else
        {
            maxCluster.push(qc);
            qc.pos = std::vector<size_t>(1, val);
            qc.score = keyword_weight[term];
            if (maxCluster.size() > CLUSTER_SIZE)
            {
                maxCluster.pop();
            }
        }
    }

    if (qc.pos.size() > 0)
    {
        maxCluster.push(qc);
        if (maxCluster.size() > CLUSTER_SIZE)
        {
            maxCluster.pop();
        }
    }

    // if (IS_DEBUG)
    // {
    //     while (!maxCluster.empty())
    //     {
    //         std::vector<size_t> pos = maxCluster.top().pos;
    //         double score = maxCluster.top().score;
    //         std::cout<<"[";
    //         for(int i=0;i<pos.size();i++){
    //             std::cout<<pos[i]<<" ";
    //         }
    //         std::cout<<"]";
    //         maxCluster.pop();
    //     }
    //     std::cout<<std::endl;
    // }

    // get snippets by pos
    while (!maxCluster.empty())
    {
        std::vector<size_t> pos = maxCluster.top().pos;
        double score = maxCluster.top().score;
        maxCluster.pop();
        if (snippets.length() > MAX_SNIPPETS)
            break;
        snippets += getSnippetsByPos(text, pos);
    }
    if(snippets.length())
    {
        snippets += "...";
    }
    

    return snippets;
}

std::vector<std::string> getKeyword(std::map<std::string, double> &word_weight)
{
    int disambNum = std::max(int(ceil(word_weight.size() * KEYWORD_PERCENT)), MIN_KEYWORD);

    // select disambNum term
    struct QueueWeight
    {
        std::string term;
        double weight;
        QueueWeight(double w, std::string t) : weight(w), term(t) {}

        // Overload the comparison operator for the priority queue
        bool operator<(const QueueWeight &other) const
        {
            return -weight < -other.weight;
        }
    };

    std::priority_queue<QueueWeight> maxWeightTermQueue;

    for (std::map<std::string, double>::const_iterator it = word_weight.begin();
         it != word_weight.end(); ++it)
    {
        maxWeightTermQueue.push(QueueWeight(it->second, it->first));
        if (maxWeightTermQueue.size() > disambNum)
        {
            maxWeightTermQueue.pop();
        }
    }

    std::vector<std::string> keyword_list;
    while (!maxWeightTermQueue.empty())
    {
        std::string term = maxWeightTermQueue.top().term;
        keyword_list.push_back(term);
        maxWeightTermQueue.pop();
    }

    return keyword_list;
}

std::string KeywordSnippets(std::string text, std::vector<std::string> &word_list, std::vector<uint32_t> &word_docNum_list)
{
    std::string snippets;

    // get position of each word in word list
    std::map<std::string, std::vector<size_t>> word_pos = getWordPos(text, word_list);
    // get weight of each term
    std::map<std::string, double> word_weight = getWordWeight(word_list, word_pos, word_docNum_list);
    // get weight of each keyword
    std::vector<std::string> keyword_list = getKeyword(word_weight);

    std::string sep = SEPARATOR;
    std::string ed = "\t\v\r\n\f ";
    std::string word;

    bool findOne = false;
    bool findTwo = false;
    bool containfirst = true;
    int deque_max_size = TERM_NUM + 1;
    std::deque<std::string> word_deque;

    for (int i = 0; i < text.length(); i++)
    {
        if (sep.find(text[i]) == std::string::npos)
        {
            word += text[i];
        }
        else
        {
            if (word.length() && ed.find(text[i]) == std::string::npos)
            {
                word += text[i];
            }

            if (word_deque.size() < deque_max_size)
            {
                word_deque.push_back(word);
            }
            else
            {
                word_deque.pop_front();
                containfirst = false;
                word_deque.push_back(word);
                if (findOne && !findTwo && !snippets.length())
                {
                    if (!containfirst)
                    {
                        snippets += "... ";
                    }
                    snippets += concatDeque(word_deque) + " ...";
                    word_deque.clear();
                    deque_max_size = TERM_NUM;
                }
                else if (findTwo)
                {
                    if (!containfirst && !snippets.length())
                    {
                        snippets += "... ";
                    }
                    snippets += concatDeque(word_deque) + "...";
                    return snippets;
                }
            }

            if (in_list(word, keyword_list))
            {
                if (!findOne)
                {
                    findOne = true;
                    deque_max_size = word_deque.size() + TERM_NUM;
                }
                else if (!findTwo)
                {
                    findTwo = true;
                    deque_max_size = word_deque.size() + TERM_NUM;
                }
            }
            word.clear();
        }
    }

    return snippets;
}

// matric, word list can be keyword
double calcWordPercent(std::string snippets, std::vector<std::string> &word_list)
{
    std::string sep = SEPARATOR;
    std::string word;
    size_t wordLen = 0;

    for (int i = 0; i < snippets.length(); i++)
    {
        if (sep.find(snippets[i]) == std::string::npos)
        {
            word += snippets[i];
        }
        else
        {
            if (word.length() && in_list(word, word_list))
            {
                wordLen += word.length();
            }
            word.clear();
        }
    }

    return wordLen / snippets.length();
}

// write document and snippets into file
void dumpSnippets(std::string file, std::string text, std::vector<std::string> &word_list, std::vector<uint32_t> &word_docNum_list)
{
    std::ofstream ofile;
    ofile.open(file);
    std::string snippets;

    ofile << text << std::endl;
    ofile << LinearMatchSnippets(text, word_list) << std::endl;
    ofile << PrefixSearchSnippets(text, word_list) << std::endl;
    ofile << ScoreSnippets(text, word_list, word_docNum_list, BM25_SNIPPETS) << std::endl;
    ofile << ScoreSnippets(text, word_list, word_docNum_list, VECTOR_SNIPPETS) << std::endl;
    ofile << KeywordSnippets(text, word_list, word_docNum_list) << std::endl;
    ofile << WeightSnippets(text, word_list, word_docNum_list) << std::endl;
    ofile.close();
}