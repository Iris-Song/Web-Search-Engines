#include "config.h"

std::string LinearMatchSnippets(std::string, std::vector<std::string> &);

std::string PrefixSearchSnippets(std::string, std::vector<std::string> &);

std::string ScoreSnippets(std::string, std::vector<std::string> &, std::vector<uint32_t> &, int type = BM25_SNIPPETS);

std::string WeightSnippets(std::string, std::vector<std::string> &, std::vector<uint32_t> &);

std::string KeywordSnippets(std::string, std::vector<std::string> &, std::vector<uint32_t> &);

void dumpSnippets(std::string, std::string, std::vector<std::string> &, std::vector<uint32_t> &);