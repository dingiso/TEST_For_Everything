# UNIX 高级网络编程复习提纲

[复习瞎写完整版](https://github.com/dingiso/TEST_For_Everything/blob/master/doc/UNIX%20%E9%AB%98%E7%BA%A7%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B%E5%A4%8D%E4%B9%A0.md)

### Chapter 1 

1. 考著名的人物： 
   1. c语言作者 ， unix 作者： Ken Thompson (Dennis M. Ritchie)， 
   2. GNU 创建者/Emacs 作者 Richard Matthew Stallman ； 
   3. linux 作者：Linus Benedict Torvalds
   4. vim 作者： Bram Moolenaar
   5. Tex 作者： Donald Knuth 高德纳

### Chapter 2 

2.6图很重要，结合代码（尤其是上半部分，一直到ESTABLISHED）	

TCP  状态转换图 ， 2.9 常用的Port Number ，2.10 Concurrent Server ， 2.12 services

### Chapter 3，4，5，6

​	重点基本都要求掌握 、 5.8，5.9，5.13不要求掌握、SIGPIPE不要求

​	6.3 掌握select

​	6.9，6.20 不要求掌握

### Chapter 7

7.1 introduction 基本原理要看，代码繁琐不要求背
7.5 SO_LINGER，SO_KEEPALIVE,SO_DONTROUTE了解
7.6 IP_HDRINCL掌握，IP_TTL掌握
7.9 IP_MAXSEG掌握	

sockopt :掌握 7.2 原理 -  SO_LINGER / SO_KEEPALIVE / SO_DONTROUTE

 	IP : IP_HDRINCL / IP_TTL 

​	TCP : TCP_MAXSEG

### Chapter 8

​	 TCP，UDP调用的区别，并发程序设计的区别 8.1图

​	8.1 图 ， 并发程序设计 - 重点注意区别

### Chapter 11

​	简单前面部分 DNS 操作 11.3、11.4、11.5,services，host，port，前面讲过的部分

## Part 3 Advanced Sockets

12，13，14，15 ，17，18.5,20,21,22,23,24，30，31不考  Daemon Process 了解一下

13了解，inet_d,SSH_D

17 ioctl可以控制socket option

18.1~18.4 网卡操作的不需要

19 Introduction

25 掌握，结合第 5.8看

26 掌握 基本概念，线程和进程区别

27 结合前面的 Options 一起看，简单看看。

28  重点掌握

29 掌握 基本概念，如何抓包



A.3 C.1  netstat tcpdump 

简答题（解释名词） ， 编程题（补充小的片段）上机+上课 ， 实验分析题（分析实验的结果）例如： 服务端没起来 报什么错误
