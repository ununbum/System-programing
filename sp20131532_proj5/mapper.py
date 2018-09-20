#!/usr/bin/python3
# _*_ coding: utf-8 _*_

from sys import stdin,stdout

while True:
	line = stdin.readline().strip()
	if line =="":
		break
	word=[]
	word = line.split()
	for i in range(len(word)):
		if(i < len(word)-1):
				stdout.write(word[i]+" " +word[i+1]+"\t1\n")
#else:
#			stdout.write(word[i]+"\t1\n")		
