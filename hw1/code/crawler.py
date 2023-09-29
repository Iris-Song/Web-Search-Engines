import threading
import argparse
from queue import PriorityQueue
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
import socket


# global var used in crawler
seed_pages = []
link_queue = PriorityQueue()
CH_num = 0
PL_num = 0
ES_num = 0
link_num_left = 0
BLACK_LIST = {'.jpg','.jpeg','.img','.png','.gif', 
            '.mp3', '.mp4', '.cgi','.asp','.aspx','.pdf',
            '.wav', '.avi', '.wmv', '.flv','.jsp','.js',
            '.php','.read','.do','.htm','.svg',
            '.py','.python','.iso'}
vis_url = set()
domain_num = {} #num of domian
url_num = {} #num of url
crawl_lock = threading.Lock()


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
         if content.status_code == 200:
            items = json.loads(content.text)['items']
            for item in items:
               seed_pages.append(item['link'])
      except Exception as e:
         logger.debug(f'error: {e}, thread {self.thread_id}, url: {url}')
      finally:
         self.seed_lock.release()


# Thread class to crawl pages
class CrawlerThreads(threading.Thread):
   '''
   crawl threads
   '''
   def __init__(self,thread_id):
      threading.Thread.__init__(self) 
      self.thread_id = thread_id
   
   def run(self):
      logger.debug(f'start thread {self.thread_id}')
      self.crawl()
      logger.debug(f'end thread {self.thread_id}')

   def crawl(self):
      global link_num_left
      global vis_url
      global link_queue
      global crawl_lock

      while link_num_left >0 and not link_queue.empty():
         # check if pages up to maximum pages
         print('link num left', link_num_left)

         try:
            # get link
            crawl_lock.acquire()
            _, url = link_queue.get()
            vis_url.add(url)
            crawl_lock.release()

            try:
               content = requests.get(url,timeout= 2)
            except Exception as e:
               logger.error(f'error: {e}, url: {url}')
               logger.critical(f'visited_url URL: {url}, error when get request, not in the sample')
               continue

            if content.status_code != 200:
               logger.critical(f'visited_url URL: {url}, status_code: {content.status_code}, not in the sample')
               continue

            try:
               isin = self.get_lang(content.text)
               self.get_links(url,content.text) 
            except Exception as e:
               continue

            crawl_lock.acquire()
            link_num_left -= 1
            crawl_lock.release()

            if isin:
               logger.critical(f'visited_url URL: {url},  status_code: {content.status_code}, size: {len(content.text)}, time of access: {content.elapsed.total_seconds()}, in the sample, in the three language')
            else:
               logger.critical(f'visited_url URL: {url},  status_code: {content.status_code}, size: {len(content.text)}, time of access: {content.elapsed.total_seconds()}, in the sample, not in the three language')

         except Exception as e:
            logger.debug(f'error: {e}, thread {self.thread_id}')
            break
         finally:
            try:
               crawl_lock.release()
            except:
               pass
                
   # get language of URL's content
   def get_lang(self,str):

      def remove_bad_chars(text):
         RE_BAD_CHARS = regex.compile(r"\p{Cc}|\p{Cs}")
         return RE_BAD_CHARS.sub("", text)
      
      isin = False
      global CH_num 
      global PL_num 
      global ES_num 
      try:
         str = remove_bad_chars(str)
         isReliable, textBytesFound, details = pycld2.detect(str)
         if isReliable:
            crawl_lock.acquire()
            for d in details:
               if d[1]=='zh':
                  isin = True
                  CH_num += 1
               elif d[1]=='pl':
                  isin = True
                  PL_num += 1
               elif d[1]=='es':
                  isin = True
                  ES_num += 1
      except:
         return False
      finally:
            try:
               crawl_lock.release()
            except Exception as e:
               return False
      return isin
   
   # get number of LIMIT_PAGES_PER_SITE children URL of a URL
   def get_links(self,org_url,text):

      LIMIT_PAGES_PER_SITE = 50
      global vis_url
      global domain_num
      global url_num

      tmp_priority_q = PriorityQueue()
      tmp_urls = set()

      try:
         parsed_url = urlparse(org_url)
         robot_txt_path = urljoin(f"{parsed_url.scheme}://{parsed_url.netloc}", "robots.txt")
         # fix rp.read bug
         f = urllib.request.urlopen(robot_txt_path,timeout = 2)
         rp = RobotFileParser(robot_txt_path)
         rp.read()
         raw_links = etree.HTML(text).xpath("//a/@href")
      except:
         return

      # check if can crawl according to robots.txt & in blacklist
      try:
         for idx,link in enumerate(raw_links):
            if len(link)<=1 or link[0]=='#':
               continue

            try:
               parsed_link = urlparse(link)
               base_url = f"{parsed_link.scheme if parsed_link.scheme else parsed_url.scheme}://{parsed_link.netloc if parsed_link.netloc else parsed_url.netloc}"
               url = raw_links[idx] = urljoin(base_url, parsed_link.path).rstrip('/')
               file_name, file_extension = path.splitext(url)
            except:
               continue

            try:
               crawl_lock.acquire()
               if file_extension.lower() in BLACK_LIST or not rp.can_fetch("*",url) \
                  or not re.match(r'^https?:/{2}\w.+$', url) or url in vis_url:
                  continue
               url_num[url] = 1 if url not in url_num else url_num[url] + 1
               if parsed_link.netloc:
                  domain_num[parsed_link.netloc] = 1 if parsed_link.netloc not in domain_num else domain_num[parsed_link.netloc] + 1
               else:
                  domain_num[parsed_url.netloc] = 1 if parsed_url.netloc not in domain_num else domain_num[parsed_url.netloc] + 1
               tmp_urls.add(url)
            finally:
               try:
                  crawl_lock.release()
               except:
                  pass
      except:
         return
      
      # calc score
      for link in tmp_urls:
         if len(link)<=1:
            continue
         
         try:
            file_name, file_extension = path.splitext(link)
            crawl_lock.acquire()
            if file_extension.lower() in BLACK_LIST or not rp.can_fetch("*",url) \
               or not re.match(r'^https?:/{2}\w.+$', url) or url in vis_url:
               continue
         except:
            continue
         finally:
            try:
               crawl_lock.release()
            except Exception as e:
               pass
         
         try:
            score = self.get_score(link)
            tmp_priority_q.put([-score,link])
         except Exception as e:
            continue

      # select LIMIT_PAGES_PER_SITE children URLs 
      p_num = 0  
      try:
         crawl_lock.acquire()
         while p_num < LIMIT_PAGES_PER_SITE and not tmp_priority_q.empty():
            p = tmp_priority_q.get()
            if p[1] in vis_url:
               continue
            link_queue.put(p)
            p_num += 1
      finally:
         try:
            crawl_lock.release()
         except Exception as e:
            pass
   
   # calc score
   def get_score(self,url):
      # score = url_num/layer_link + layer_link/domain_num
      domain = urlparse(url).netloc
      score = url_num[url]/len(url_num) + len(domain_num)/domain_num[domain]
      return score

def parse_args():
   parser = argparse.ArgumentParser(description='web crawler')
   parser.add_argument("-q", "--query", help="Search Query", default="Python")
   parser.add_argument("-p", "--page", help="Estimated Pages Crawled in the sample", default = 4000)
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
   
   # print(logpath)
   logging.basicConfig(
      format='%(asctime)s - %(message)s',
      filename=logpath,
      filemode='w',
      level = logging.CRITICAL)
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

   for e in seed_pages:
      link_queue.put([-1000,e])
   
   # 3.crawl
   number_of_threads = len(seed_pages)
   crawler_threads = []
   
   link_num_left = page
   threading.TIMEOUT_MAX = 30
   socket.setdefaulttimeout(2)
   for i in range(number_of_threads):
      crawler = CrawlerThreads(
         thread_id = i
      )
      
      crawler.start()
      crawler_threads.append(crawler)

   for thread in crawler_threads:
      thread.join()

   
   ## log basic statistics at the end of the crawl
   page -= link_num_left
   time_shift = time.time() - start_time
   logger.critical(f'{page} URLs included in the sample, using {time_shift} seconds, visited {len(vis_url)} URLs')
   logger.critical('percentage of pages in Spanish {:%}'.format(ES_num/page))
   logger.critical('percentage of pages in Chinese {:%}'.format(CH_num/page))
   logger.critical('percentage of pages in Polish {:%}'.format(PL_num/page))