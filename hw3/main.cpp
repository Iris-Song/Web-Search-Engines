#include "DataLoader.h"
int main()
{
    DataLoader dl;
    
    if (IS_INDEX)
    {
        clock_t index_begin = clock();
        dl.ReadData(DATA_SOURCE_PATH);
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

    if(IS_BUILD)
    {
        clock_t build_lexicon_begin = clock();
        dl.BuildLexicon();
        clock_t build_lexicon_end = clock();
        clock_t lexicon_time = build_lexicon_end-build_lexicon_begin;
        std::cout<<"Build Lexicon Structure Need "<<double(lexicon_time)/1000000<<"s"<<std::endl;  
    }

    if(IS_BUILD && IS_WRITE_LEXICON)
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
        std::cout<<"Start up DocTable Structure Need "<<double(reload_time)/1000000<<"s"<<std::endl; 
    }

    if(IS_QUERY)
    {
        dl.QueryLoop();
        // dl.TestQuery();
    }

    if(IS_DEBUG && IS_SNIPPETS)
    {
        clock_t reload_begin = clock();
        dl._DocTable.LoadDocTable();
        clock_t reload_end = clock();
        clock_t reload_time = reload_end - reload_begin;
        std::cout<<"Start up DocTable and Lexicon Structure Need "<<double(reload_time)/1000000<<"s"<<std::endl;         
        dl.TestSnippets(FAKE_RESULT_PATH);
    }
    
}