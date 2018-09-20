#!usr/bim/python3
#-*- coding: utf-8 -*-

from sys import stdin,stdout

dic = {}
while True:
	line = stdin.readline().strip()

	if line == "":
		break
	key,value=line.split("\t")
	key = key.lower()
	if key in dic.keys():
		dic[key] +=1
	else:
		dic[key]=1
li = []
for i in dic:
	li.append(i+"\t%d\n"%dic[i])
li.sort()
for i in li:
	stdout.write(i)
