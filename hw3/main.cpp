#include "DataLoader.h"
int main()
{
    DataLoader dl;
    if (IS_INDEX)
    {
        clock_t index_begin = clock();
        dl.read_data(DATA_SOURCE_PATH);
        clock_t index_end = clock();
        clock_t index_time = index_end-index_begin;
        std::cout<<"Build Postings and TEMP Inverted Index Need "<<double(index_time)/1000000<<"s"<<std::endl;
    }

    if (IS_MERGE)
    {
        clock_t merge_begin = clock();
        dl.mergeIndexToOne();
        clock_t merge_end = clock();
        clock_t merge_time = merge_end-merge_begin;
        std::cout<<"Merge Inverted Index Need "<<double(merge_time)/1000000<<"s"<<std::endl;
    }

    if(IS_WRITE_LEXICON)
    {
        clock_t write_lexicon_begin = clock();
        dl.WriteLexicon();
        clock_t write_lexicon_end = clock();
        clock_t lexicon_time = write_lexicon_end-write_lexicon_begin;
        std::cout<<"Write Lexicon Structure Need "<<double(lexicon_time)/1000000<<"s"<<std::endl;        
    }

    if(IS_RELOAD)
    {
        clock_t reload_begin = clock();
        dl._DocTable.LoadDocTable();
        dl._Lexicon.LoadLexiconList();
        clock_t reload_end = clock();
        clock_t reload_time = reload_end - reload_begin;
        std::cout<<"Start up DocTable and Lexicon Structure Need "<<double(reload_time)/1000000<<"s"<<std::endl; 
    }

    if(IS_QUERY)
    {
        dl.QueryLoop();
    }
    
}