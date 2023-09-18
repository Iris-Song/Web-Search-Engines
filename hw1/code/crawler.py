"""
mulithread 
"""
import threading
import argparse
import requests
from queue import Queue
from os import getenv,path
from dotenv import load_dotenv
import json
from lxml import etree
import pycld2
import regex
import random
from urllib.robotparser import RobotFileParser
from urllib.parse import urlparse,urljoin
import logging

logging.basicConfig(
   format='%(asctime)s - %(message)s',
   filename='crawl.log')
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

link_queue = Queue()
seed_pages = []
BLACK_LIST = {'.jpg','.jpeg','.img','.png','.gif', 
            '.mp3', '.mp4', '.cgi','.asp','.pdf',
            '.wav', '.avi', '.wmv', '.flv','.jsp',
            '.php','.read','.do','.htm','.svg'}

class SeedThreads(threading.Thread):
   def __init__(self,thread_id,seed_num,seed_lock):
      load_dotenv()
      threading.Thread.__init__(self) 
      self.thread_id = thread_id
      self.startIndex = thread_id*10 + 1
      self.seed_num = seed_num
      self.seed_lock = seed_lock
      
   def run(self):
      logger.info(f'start thread {self.thread_id}')
      self.crawl()
      logger.info(f'end thread {self.thread_id}')
   
   def crawl(self):
      GOOGLE_SEARCH_API_BASE = 'https://customsearch.googleapis.com/customsearch/v1'
      if self.seed_num-self.startIndex< 10:
         num = self.seed_num-self.startIndex + 1
         url = f"{GOOGLE_SEARCH_API_BASE}?key={getenv('GOOGLE_SEARCH_API_KEY')}&cx={getenv('GOOGLE_SEARCH_ENGINE_ID')}&q={query}&start={self.startIndex}&num={num}"
      else:
         url = f"{GOOGLE_SEARCH_API_BASE}?key={getenv('GOOGLE_SEARCH_API_KEY')}&cx={getenv('GOOGLE_SEARCH_ENGINE_ID')}&q={query}&start={self.startIndex}"
      try:
         self.seed_lock.acquire()
         content = requests.get(url,timeout=200)
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

class CrawlerThreads(threading.Thread):
   '''
   crawl threads
   '''
   def __init__(self,thread_id,queue,crawl_lock,ch_lock,pl_lock,es_lock):
      threading.Thread.__init__(self) 
      self.thread_id = thread_id
      self.queue = queue
      self.crawl_lock = crawl_lock
      self.ch_lock = ch_lock
      self.pl_lock = pl_lock
      self.es_lock = es_lock
   
   def run(self):
      logger.info(f'start thread {self.thread_id}')
      self.crawl()
      logger.info(f'end thread {self.thread_id}')

   def crawl(self):
      global link_num_left
      while True:
         if link_num_left <=0:
            logger.debug('added pages up to maximum pages')
            break
         if self.queue.empty():
            break
         else:
            self.crawl_lock.acquire()
            url = self.queue.get()
            self.crawl_lock.release()
            logger.info(f'thread {self.thread_id} : analyzing page {url}')
            try:
               content = requests.get(url,timeout= 200)
               if content.status_code != 200:
                  raise('status code is', content.status_code)
               self.get_lang(content.text)
               self.get_links(url,content.text)
            except Exception as e:
               logger.info(f'visited URL: {url}, size: {len(content.text)}, time of access: {content.elapsed.total_seconds()}, just traversed')
            else:
               link_num_left -= 1
               logger.info(f'visited URL: {url}, size: {len(content.text)}, time of access: {content.elapsed.total_seconds()}, successful added')
            
   def get_lang(self,str):

      def remove_bad_chars(text):
         RE_BAD_CHARS = regex.compile(r"\p{Cc}|\p{Cs}")
         return RE_BAD_CHARS.sub("", text)
      
      global CH_num
      global PL_num
      global ES_num
      str = remove_bad_chars(str)
      isReliable, textBytesFound, details = pycld2.detect(str)
      if isReliable:
         for d in details:
            if d[1]=='zh':
               self.ch_lock.acquire()
               CH_num += 1
               self.ch_lock.release()
            elif d[1]=='pl':
               self.pl_lock.acquire()
               PL_num += 1
               self.pl_lock.release()
            elif d[1]=='es':
               self.es_lock.acquire()
               ES_num += 1
               self.es_lock.release()
   
   def get_links(self,org_url,text):
      LIMIT_PAGES_PER_SITE = 10
      parsed_url = urlparse(org_url)
      robot_txt_path = urljoin(f"{parsed_url.scheme}://{parsed_url.netloc}", "robots.txt")
      rp = RobotFileParser(robot_txt_path)
      rp.read()
      raw_links = etree.HTML(text).xpath("//a/@href")
      # print('len of raw links',len(raw_links))
      extension = set()
      add_num = 0
      random.shuffle(raw_links)
      for link in raw_links:
         if add_num >= LIMIT_PAGES_PER_SITE:
            break
         if not link:
            continue
         parsed_link = urlparse(link)
         base_url = f"{parsed_url.scheme if parsed_url.scheme else parsed_link.scheme}://{parsed_link.netloc if parsed_link.netloc else parsed_url.netloc}"
         url = urljoin(base_url, parsed_link.path)
         file_name, file_extension = path.splitext(url)
         if file_extension.lower() in BLACK_LIST or not rp.can_fetch("*",url) or url in vis:
            continue
         if file_extension not in extension:
            extension.add(file_extension)
         vis.add(url)
         self.queue.put(url)
         add_num += 1


def parse_args():
   parser = argparse.ArgumentParser(description='web crawler')
   parser.add_argument("-q", "--query", help="Search Query", default="python")
   parser.add_argument("-d", "--depth", help="Maximum crawling depth", default=3)
   parser.add_argument("-p", "--page", help="Pages Crawled", default=100)
   parser.add_argument("-s", "--seed", help="Number of Seed Page", default=10)
   
   if parser.parse_args().seed <= 0:
      exit('Error! Number of seed pages should greater than 0')
   if parser.parse_args().seed > 100:
      exit('Error! Number of seed pages should no more than 100')
   if parser.parse_args().page <= 0:
      exit('Error! Number of pages crawled should greater than 0')
   elif parser.parse_args().seed > parser.parse_args().page:
      exit('Error! Number of seed pages is greater than page crawled.')
   return parser.parse_args()

def get_seed_page(query, seed_num):
   load_dotenv()
   seed_page = []
   startIndex = 1
   GOOGLE_SEARCH_API_BASE = 'https://customsearch.googleapis.com/customsearch/v1'
   while seed_num > 0:
      if seed_num > 10:
         url = f"{GOOGLE_SEARCH_API_BASE}?key={getenv('GOOGLE_SEARCH_API_KEY')}&cx={getenv('GOOGLE_SEARCH_ENGINE_ID')}&q={query}&start={startIndex}"
      else:
         url = f"{GOOGLE_SEARCH_API_BASE}?key={getenv('GOOGLE_SEARCH_API_KEY')}&cx={getenv('GOOGLE_SEARCH_ENGINE_ID')}&q={query}&start={startIndex}&num={seed_num}"
      seed_num -= 10
      startIndex += 10
      content = requests.get(url,timeout=200)
      items = json.loads(content.text)['items']
      for item in items:
         seed_page.append(item['link'])
   # print(seed_page)
   # print(len(set(seed_page)))
   return seed_page


if __name__ == "__main__":
   # 1. get args
   query, depth, page, seed = parse_args().query, parse_args().depth , parse_args().page, int(parse_args().seed)
   
   # 2. get seed pages
   # seed_threads_num = math.ceil(seed/10) 
   # seed_threads = []
   # seed_lock = threading.Lock()
   # for i in range(seed_threads_num):
   #    seed_thread = SeedThreads(
   #       thread_id = i,
   #       seed_num = seed,
   #       seed_lock = seed_lock
   #    )
   #    seed_thread.start()
   #    seed_threads.append(seed_thread)

   # for thread in seed_threads:
   #    thread.join()

   # print(seed_pages)
   # print(len(seed_pages))

   # seed_pages = 'https://pl.wikipedia.org/wiki/J%C4%99zyk_hiszpa%C5%84ski',
   seed_pages = [
      'https://www.python.org/', 'https://en.wikipedia.org/wiki/Python_(programming_language)', 'https://www.python.org/downloads/', 'https://www.w3schools.com/python/', 'https://docs.python.org/3/tutorial/index.html', 'https://www.codecademy.com/catalog/language/python', 'https://docs.python.org/', 'https://marketplace.visualstudio.com/items?itemName=ms-python.python', 'https://docs.python.org/3/library/index.html', 'https://pypi.org/']
   vis = set()
   for e in seed_pages:
      link_queue.put(e)
   # print(link_queue)

   # 3.crawl
   number_of_threads = 5
   crawler_threads = []
   crawl_lock = threading.Lock()
   ch_lock = threading.Lock()
   pl_lock = threading.Lock()
   es_lock = threading.Lock()
   link_num_left = page
   CH_num = 0
   PL_num = 0
   ES_num = 0
   for i in range(number_of_threads):
      crawler = CrawlerThreads(
         thread_id = i,
         queue = link_queue,
         crawl_lock = crawl_lock,
         ch_lock = ch_lock,
         pl_lock = pl_lock,
         es_lock = es_lock,
      )
    
      crawler.start()
      crawler_threads.append(crawler)

   for thread in crawler_threads:
      thread.join()
   
   
   logger.info('percentage of pages in Spanish {:%}'.format(ES_num/len(vis)))
   logger.info('percentage of pages in Chinese {:%}'.format(CH_num/len(vis)))
   logger.info('percentage of pages in Polish {:%}'.format(PL_num/len(vis)))

   # seed_page = get_seed_page(query, seed)
   # get_language(seed_page)