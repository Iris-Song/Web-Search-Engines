# MultiThread Web Crawler

## Files
.
├── crawler.py (Python program source)
├── explain.txt
├── readme.txt
├── requirements.txt (python lib need to install)
├── test1.log (a output of logs)
└── test2.log (a output of logs)


## requirements
1. get [Google Custom Search JSON API](https://developers.google.com/custom-search/v1/overview)
2. virtual env (contain GOOGLE_SEARCH_API_KEY,GOOGLE_SEARCH_ENGINE_ID)
3. requirements.txt


## install python lib
$ pip install -r requirements.txt


## instructions
```
usage: crawler.py [-h] [-q QUERY] [-p PAGE] [-s SEED] [-d [DEBUG]] [-f FILE]

web crawler

options:
  -h, --help            show this help message and exit
  -q QUERY, --query QUERY
                        Search Query
  -p PAGE, --page PAGE  Estimated Pages Crawled in the sample
  -s SEED, --seed SEED  Number of Seed Page
  -d [DEBUG], --debug [DEBUG]
                        whether to add debug information in log
  -f FILE, --file FILE  log file path
```
E.G.: query 'Python', using 12 URL as seed pages from Google Search Engine, 
crawling 200 pages without debug, the log path is 'example.log'.
$ python crawler.py -q Python -s 12 -p 200 -f 'example.log'
you can get details in `example.log`.

if you want also see debug details in log.
$ python crawler.py -q Python -s 12 -p 200 -f 'example.log' -d


## suggestions
To ensure the efficiency of crawler, no less than 10 seeds, 
no more than 10000 pages if you want to finish in 1 hour.



