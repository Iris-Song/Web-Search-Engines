# MultiThread Web Crawler

## requirements
+ [Google Custom Search JSON API](https://developers.google.com/custom-search/v1/overview)
+ virtual env
+ requirements.txt

## install
```
pip install -r requirements.txt
```

## instructions
E.G.: query *Python*, using *12* URL as seed pages from Google Search Engine, crawling *200* pages.
```
python crawler.py -q Python -s 12 -p 200
```
you can get details in `crawl.log`

## stategy
1. using Priority Queue `q` to traverse the URL `u` with highest score
2. check if `u` is legal
3. get `u`'s information
4. select `LIMIT_PAGES_PER_SITE` links in `u` and add these links to `q`

### score calculation

$$ score = \frac{N_{URL}}{N_{URLs}} + \frac{N_{DOMAINs}}{N_{DOMAIN}} $$
