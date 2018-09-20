import requests
from bs4 import BeautifulSoup
root = "http://cspro.sogang.ac.kr/~cse20121611/"
r = requests.get(root)
soup = BeautifulSoup(r.content,"html.parser")
count = 1
list = []
url = open("URL.txt","w")
def crawl(text):
	global count
	global root
	global r
	global soup
	global url
	global list
	if('#' in text):
		leaf = root
	elif('?' in text):
		leaf = root
	elif('index.html' in text):
		leaf = root
	elif('http://' not in text):
		leaf = root + text
	else:
		leaf = text
	if(leaf not in list):
		leaf_r = requests.get(leaf)
		if(leaf_r.ok is True):
			leaf_soup = BeautifulSoup(leaf_r.content,"html.parser")
			leaf_link = leaf_soup.find_all('a')				
			list.append(leaf)
			name = "Output_%04d.txt" %count
			f = open(name,"w")
			f.write(leaf_soup.get_text())
			count+=1
			url.write(leaf)
			url.write('\n')
			for i in leaf_link:
				crawl(i.get('href'))
	
list.append(root)
name = "Output_%04d.txt" %(count)
f = open(name,"w")
f.write(soup.get_text())
count+=1
link =  soup.find_all('a')
url.write(root)
url.write('\n')
for i in link:
	crawl(i.get('href'))


