import threading
import argparse
from queue import Queue,PriorityQueue
from os import getenv,path
from dotenv import load_dotenv
import json
import pycld2
import regex
import re
import math
import requests
from lxml import etree
import urllib
from urllib.robotparser import RobotFileParser
from urllib.parse import urlparse,urljoin
import logging
import pathlib
import time

link_queue = PriorityQueue()
seed_pages = []
BLACK_LIST = {'.jpg','.jpeg','.img','.png','.gif', 
            '.mp3', '.mp4', '.cgi','.asp','.aspx','.pdf',
            '.wav', '.avi', '.wmv', '.flv','.jsp','.js',
            '.php','.read','.do','.htm','.svg',
            '.py','.python','.iso'}

# Thread class to get seed pages
class SeedThreads(threading.Thread):
   def __init__(self,thread_id,seed_num,seed_lock):
      load_dotenv()
      threading.Thread.__init__(self) 
      self.thread_id = thread_id
      self.startIndex = thread_id*10 + 1
      self.seed_num = seed_num
      self.seed_lock = seed_lock
      
   def run(self):
      logger.debug(f'start thread {self.thread_id}')
      self.crawl()
      logger.debug(f'end thread {self.thread_id}')
   
   def crawl(self):
      GOOGLE_SEARCH_API_BASE = 'https://customsearch.googleapis.com/customsearch/v1'
      if self.seed_num-self.startIndex< 10:
         num = self.seed_num-self.startIndex + 1
         url = f"{GOOGLE_SEARCH_API_BASE}?key={getenv('GOOGLE_SEARCH_API_KEY')}&cx={getenv('GOOGLE_SEARCH_ENGINE_ID')}&q={query}&start={self.startIndex}&num={num}"
      else:
         url = f"{GOOGLE_SEARCH_API_BASE}?key={getenv('GOOGLE_SEARCH_API_KEY')}&cx={getenv('GOOGLE_SEARCH_ENGINE_ID')}&q={query}&start={self.startIndex}"
      try:
         self.seed_lock.acquire()
         content = requests.get(url,timeout=2)
         self.seed_lock.release()
         if content.status_code == 200:
            items = json.loads(content.text)['items']
            for item in items:
               seed_pages.append(item['link'])
      except Exception as e:
         logger.debug(f'error: {e}, url: {url}')
         # print(e)
         # print(url)
         # print(self.startIndex)
         self.seed_lock.release()

# Thread class to crawl pages
class CrawlerThreads(threading.Thread):
   '''
   crawl threads
   '''
   def __init__(self,thread_id,queue,crawl_lock,link_num_lock,ch_lock,pl_lock,es_lock,hash_lock):
      threading.Thread.__init__(self) 
      self.thread_id = thread_id
      self.queue = queue
      self.crawl_lock = crawl_lock
      self.link_num_lock = link_num_lock
      self.ch_lock = ch_lock
      self.pl_lock = pl_lock
      self.es_lock = es_lock
      self.hash_lock = hash_lock
   
   def run(self):
      logger.debug(f'start thread {self.thread_id}')
      self.crawl()
      logger.debug(f'end thread {self.thread_id}')

   def crawl(self):
      global link_num_left
      while True:
         # check if pages up to maximum pages
         self.link_num_lock.acquire(timeout = 25)
         try:
            print('link num left', link_num_left)
            if link_num_left <=0:
               logger.debug('added pages up to maximum pages')
               break
         finally:
            self.link_num_lock.release()
         
         # get URL to crawl
         self.crawl_lock.acquire(timeout = 25)
         try:
            if self.queue.empty():
               break
            _, url = self.queue.get()
            vis_url.add(url)
         except Exception as e:
            logger.debug(f'error: {e}')
            continue
         finally:
            self.crawl_lock.release()

         logger.debug(f'thread {self.thread_id} : analyzing page {url}')
         try:
            content = requests.get(url,timeout= 2)
            if content.status_code != 200:
               logger.info(f'visited_url URL: {url}, status_code: {content.status_code}, not in the sample')
         except Exception as e:
            logger.debug(f'error: {e}, url: {url}')
            logger.info(f'visited_url URL: {url}, error when get request, not in the sample')
         else:
            if content.status_code != 200:
               continue
            try:
               isin = self.get_lang(content.text)
               self.get_links(url,content.text) 
            except Exception as e:
               logger.info(f'visited_url URL: {url}, status_code: {content.status_code}, size: {len(content.text)}, time of access: {content.elapsed.total_seconds()}, error when get language, not in the sample')
            else:
               if content.status_code == 200:
                  self.link_num_lock.acquire(timeout = 25)
                  link_num_left -= 1
                  self.link_num_lock.release()   
                  if isin:
                     logger.info(f'visited_url URL: {url},  status_code: {content.status_code}, size: {len(content.text)}, time of access: {content.elapsed.total_seconds()}, in the sample, in the three language')
                  else:
                     logger.info(f'visited_url URL: {url},  status_code: {content.status_code}, size: {len(content.text)}, time of access: {content.elapsed.total_seconds()}, in the sample, not in the three language')
            
   # get language of URL's content
   def get_lang(self,str):

      def remove_bad_chars(text):
         RE_BAD_CHARS = regex.compile(r"\p{Cc}|\p{Cs}")
         return RE_BAD_CHARS.sub("", text)
      
      global CH_num
      global PL_num
      global ES_num
      isin = False
      try:
         str = remove_bad_chars(str)
         isReliable, textBytesFound, details = pycld2.detect(str)
      except Exception as e:
         logging.debug(f'error: {e}')
         return False
      if isReliable:
         for d in details:
            if d[1]=='zh':
               isin = True
               self.ch_lock.acquire(timeout = 2)
               CH_num += 1
               self.ch_lock.release()
            elif d[1]=='pl':
               isin = True
               self.pl_lock.acquire(timeout = 2)
               PL_num += 1
               self.pl_lock.release()
            elif d[1]=='es':
               isin = True
               self.es_lock.acquire(timeout = 2)
               ES_num += 1
               self.es_lock.release()
      return isin
   
   # get number of LIMIT_PAGES_PER_SITE children URL of a URL
   def get_links(self,org_url,text):
      global url_num
      global domain_num
      LIMIT_PAGES_PER_SITE = 50
      parsed_url = urlparse(org_url)

      robot_txt_path = urljoin(f"{parsed_url.scheme}://{parsed_url.netloc}", "robots.txt")
      try:
         # fix rp.read bug
         f = urllib.request.urlopen(robot_txt_path,timeout = 2)
         rp = RobotFileParser(robot_txt_path)
         rp.read()
      except Exception as e:
         logging.debug(f'error: {e}, url: {org_url}')
         return
      
      try:
         raw_links = etree.HTML(text).xpath("//a/@href")
      except Exception as e:
         logging.debug(f'error: {e}, url: {org_url}')
         return

      # check if can crawl according to robots.txt & in blacklist
      tmp_priority_q = PriorityQueue()
      tmp_urls = set()
      for idx,link in enumerate(raw_links):
         if len(link)<=1:
            continue
         parsed_link = urlparse(link)
         base_url = f"{parsed_link.scheme if parsed_link.scheme else parsed_url.scheme}://{parsed_link.netloc if parsed_link.netloc else parsed_url.netloc}"
         url = raw_links[idx] = urljoin(base_url, parsed_link.path).rstrip('/')
         file_name, file_extension = path.splitext(url)
         self.crawl_lock.acquire(timeout = 25)
         try:
            if file_extension.lower() in BLACK_LIST or not rp.can_fetch("*",url) or not re.match(r'^https?:/{2}\w.+$', url) or url in vis_url:
               continue
         finally:
            self.crawl_lock.release()

         self.hash_lock.acquire(timeout = 25)
         try:
            url_num[url] = 1 if url not in url_num else url_num[url] + 1
            if parsed_link.netloc:
               domain_num[parsed_link.netloc] = 1 if parsed_link.netloc not in domain_num else domain_num[parsed_link.netloc] + 1
            else:
               domain_num[parsed_url.netloc] = 1 if parsed_url.netloc not in domain_num else domain_num[parsed_url.netloc] + 1
         finally:
            self.hash_lock.release()
         tmp_urls.add(url)
      # print(len(tmp_urls))
      
      # calc score
      for link in tmp_urls:
         if len(link)<=1:
            continue
         file_name, file_extension = path.splitext(link)
         self.crawl_lock.acquire(timeout = 25)
         try:
            if file_extension.lower() in BLACK_LIST or not rp.can_fetch("*",url) \
               or not re.match(r'^https?:/{2}\w.+$', url) or url in vis_url:
               continue
         finally:
            self.crawl_lock.release()
         
         self.hash_lock.acquire(timeout = 25)
         score = self.get_score(link)
         self.hash_lock.release()
         tmp_priority_q.put([-score,link])

      # select LIMIT_PAGES_PER_SITE children URLs   
      p_num = 0
      self.crawl_lock.acquire(timeout = 25)
      while p_num < LIMIT_PAGES_PER_SITE:
         if tmp_priority_q.empty():
            break
         p = tmp_priority_q.get()
         if p[1] in vis_url:
            continue
         link_queue.put(p)
         p_num += 1
      self.crawl_lock.release()
   
   # calc score
   def get_score(self,url):
      # score = url_num/layer_link + layer_link/domain_num
      # print(url)
      domain = urlparse(url).netloc
      score = url_num[url]/len(url_num) + len(domain_num)/domain_num[domain]
      return score

def parse_args():
   parser = argparse.ArgumentParser(description='web crawler')
   parser.add_argument("-q", "--query", help="Search Query", default="Python")
   parser.add_argument("-p", "--page", help="Estimated Pages Crawled in the sample", default = 100)
   parser.add_argument("-s", "--seed", help="Number of Seed Page", default=10)
   parser.add_argument("-d", "--debug", help="whether to add debug information in log", default=False, nargs='?')
   parser.add_argument("-f", "--file", help="log file path", default='crawl.log',type=pathlib.Path)
   
   if int(parser.parse_args().seed) <= 0:
      exit('Error! Number of seed pages should greater than 0')
   if int(parser.parse_args().seed) > 100:
      exit('Error! Number of seed pages should no more than 100')
   if int(parser.parse_args().page) <= 0:
      exit('Error! Number of pages crawled should greater than 0')
   elif int(parser.parse_args().seed) > int(parser.parse_args().page):
      exit('Error! Number of seed pages is greater than page crawled.')
   return parser.parse_args()


if __name__ == "__main__":
   # 1. get args & start time & logs
   start_time = time.time()
   query, page, seed, isdebug,logpath = parse_args().query, int(parse_args().page), \
      int(parse_args().seed), parse_args().debug, parse_args().file
   
   print(logpath)
   logging.basicConfig(
      format='%(asctime)s - %(message)s',
      filename=logpath,
      filemode='w',
      level = logging.INFO)
   logger = logging.getLogger(__name__)
   if isdebug!= False:
      logger.setLevel(logging.DEBUG)
   
   
   # 2. get seed pages
   seed_threads_num = math.ceil(seed/10) 
   seed_threads = []
   seed_lock = threading.Lock()
   for i in range(seed_threads_num):
      seed_thread = SeedThreads(
         thread_id = i,
         seed_num = seed,
         seed_lock = seed_lock
      )
      seed_thread.start()
      seed_threads.append(seed_thread)

   for thread in seed_threads:
      thread.join()

   print(seed_pages)
   print(len(seed_pages))
   # seed_pages = ['https://www.canada.ca/en.html','https://en.wikipedia.org/wiki/Chinese_language', 'https://www.tclchinesetheatres.com/', 'https://en.wikipedia.org/wiki/China', 'https://english.cas.cn/', 'https://www.un.org/zh/observances/chinese-language-day/english', 'https://www.cnn.com/world/china', 'https://www.pandaexpress.com/', 'https://camla.org/', 'https://chineselaundry.com/', 'https://chsa.org/']
   
   vis_url = set()
   domain_num = {} #num of domian
   url_num = {} #num of url
   for e in seed_pages:
      link_queue.put([-1000,e])
   # print(link_queue)

   # 3.crawl
   number_of_threads = 5
   crawler_threads = []
   crawl_lock = threading.Lock()
   link_num_lock = threading.Lock()
   ch_lock = threading.Lock()
   pl_lock = threading.Lock()
   es_lock = threading.Lock()
   hash_lock = threading.Lock()
   link_num_left = page
   CH_num = 0
   PL_num = 0
   ES_num = 0
   for i in range(number_of_threads):
      crawler = CrawlerThreads(
         thread_id = i,
         queue = link_queue,
         crawl_lock = crawl_lock,
         link_num_lock =link_num_lock,
         ch_lock = ch_lock,
         pl_lock = pl_lock,
         es_lock = es_lock,
         hash_lock = hash_lock,
      )
    
      crawler.start()
      crawler_threads.append(crawler)

   for thread in crawler_threads:
      thread.join()
   
   ## log basic statistics at the end of the crawl
   page -= link_num_left
   time_shift = time.time() - start_time
   logger.info(f'{page} URLs included in the sample, using {time_shift} seconds, visited {len(vis_url)} URLs')
   logger.info('percentage of pages in Spanish {:%}'.format(ES_num/page))
   logger.info('percentage of pages in Chinese {:%}'.format(CH_num/page))
   logger.info('percentage of pages in Polish {:%}'.format(PL_num/page))

   # print('len_vis',len(vis_url))
   # print('page',page)