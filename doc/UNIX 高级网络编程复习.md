# UNIX 高级网络编程复习提纲



### Chapter 1 

1. 考著名的人物： 
   1. c语言作者 ， unix 作者： Ken Thompson (Dennis M. Ritchie)， 
   2. GNU 创建者/Emacs 作者 Richard Matthew Stallman ； 
   3. linux 作者：Linus Benedict Torvalds
   4. vim 作者： Bram Moolenaar
   5. Tex 作者： Donald Knuth 高德纳

### Chapter 2 

​	TCP  状态转换图 ， 2.9 常用的Port Number ，2.10 Concurrent Server ， 2.12 services

```
21端口：FTP 文件传输服务
22端口：SSH 端口
23端口：TELNET 终端仿真服务
25端口：SMTP 简单邮件传输服务
53端口：DNS 域名解析服务
80端口：HTTP 超文本传输服务
110端口：POP3 “邮局协议版本3”使用的端口
443端口：HTTPS 加密的超文本传输服务
**********************************
1433端口：MS SQL*SERVER数据库 默认端口号
1521端口：Oracle数据库服务
1863端口：MSN Messenger的文件传输功能所使用的端口
3306端口：MYSQL 默认端口号
3389端口：Microsoft RDP 微软远程桌面使用的端口
5631端口：Symantec pcAnywhere 远程控制数据传输时使用的端口
5632端口：Symantec pcAnywhere 主控端扫描被控端时使用的端口
5000端口：MS SQL Server使用的端口
8000端口：腾讯QQ
```

### Chapter 3，4，5，6

#### Socket Address Structures

```c
struct in_addr {
	in_addr_t 	s_addr; 		/* 32-bit IPv4 address */
				  				/* network byte ordered */
};
struct sockaddr_in {
	uint8_t 		sin_len; 	/* length of structure (16) */
	sa_family_t 	sin_family; /* AF_INET */
	in_port_t		sin_port; 	/* 16-bit TCP or UDP port number */
								/* network byte ordered */
	struct in_addr  sin_addr; 	/* 32-bit IPv4 address */
								/* network byte ordered */
	char 			sin_zero[8];/* unused */
};
// IPV4
inet_addr inet_ntoa
// IPV4/6
inet_pton inet_ntop
```

* `in_addr` 是结构的原因是早期将其定义为 `union` 方便`A，B，C`类地址的访问
* `sin_zero` 置0

#### Generic socket address structure

```c
struct sockaddr {
	uint8_t 	sa_len;
	sa_family_t sa_family; 	 /* address family: AF_xxx value */
	char 		sa_data[14]; /* protocol-specific address */
};
// 用法
int bind(int, struct sockaddr *, socklen_t);

struct sockaddr_in serv; /* IPv4 socket address structure */
/* fill in serv{} */
bind(sockfd, (struct sockaddr *) &serv, sizeof(serv));
```

用于定义函数时适配各种不同类型的地址结构，

若不转换，编译器报 `warning: passign arg 2 of 'bind' from incompatible pointer type`

#### Sockaddr_in6

128-bits ipv6 地址

```c
struct in6_addr {
	uint8_t 	s6_addr[16]; /* 128-bit IPv6 address */
							 /* network byte ordered */
};

#define SIN6_LEN 			 /* required for compile-time tests */

struct sockaddr_in6 {
	uint8_t 		sin6_len; 		/* length of this struct (28) */
	sa_family_t 	sin6_family; 	/* AF_INET6 */
	in_port_t 		sin6_port; 		/* transport layer port# */
									/* network byte ordered */
	uint32_t 		sin6_flowinfo;  /* flow information, undefined */
	struct in6_addr sin6_addr; 		/* IPv6 address */
									/* network byte ordered */
	uint32_t 		sin6_scope_id;  /* set of interfaces for a scope */
};
```

#### Value-Result 

当我们把 SA* 从用户进程传入内核时长度作为值 value ，内核处理完返回时结构的大小可能会改变，因此长度作为一个结果 result 传回 , 引用是因为需要函数内部去赋值

```c
// 用户进程 => 内核
// bind connect sendto 
struct sockaddr_in serv;

/* fill in serv{} */
connect(sockfd, (SA *) &serv, sizeof(serv));

// 内核 => 用户进程
// accept recvfrom getsockname getpeername 
struct sockaddr_un cli; /* Unix domain */
socklen_t len;

len = sizeof(cli); 		/* len is a value */
getpeername(unixfd, (SA *) &cli, &len);
/* len may have changed */
```

#### 字节序 Byte Ordering

```c
// 字节序转换函数
#include <netinet/in.h>
uint16_t htons(uint16_t host16bitvalue);
uint32_t htonl(uint32_t host32bitvalue);
 									Both return: value in network byte order
uint16_t ntohs(uint16_t net16bitvalue);
uint32_t ntohl(uint32_t net32bitvalue);
									Both return: value in host byte order
```

n - network ,  h  - host ,  s - short 16 位, l - long 32 位

#### Byte Manipulation

```c
#include <strings.h>
void bzero(void *dest, size_t nbytes);

void bcopy(const void *src, void *dest, size_t nbytes);

int  bcmp(const void *ptr1, const void *ptr2, size_t nbytes);
									Returns: 0 if equal, nonzero if unequal
```

mem

```c
#include <string.h>
void *memset(void *dest, int c, size_t len);

void *memcpy(void *dest, const void *src, size_t nbytes);

int   memcmp(const void *ptr1, const void *ptr2, size_t nbytes);
						Returns: 0 if equal, <0 or >0 if unequal (see text)
```

#### 地址转换函数

**IPV4**

```c
#include <arpa/inet.h>
// converts the C character string pointed to by strptr into
// its 32-bit binary network byte ordered value, 
// which is stored through the pointer addrptr 
int inet_aton(const char *strptr, struct in_addr *addrptr);
					Returns: 1 if string was valid, 0 on error
in_addr_t inet_addr(const char *strptr);
					Returns: 32-bit binary network byte ordered IPv4 address; 							   									INADDR_NONE if error
char *inet_ntoa(struct in_addr inaddr);
					Returns: pointer to dotted-decimal string
```

`inet_aton` 将字符串 strptr 转换为 32比特二进制网络字节序地址 addrptr

`inet_addr` 作用同上，出错返回 `INADDR_NONE`255.255.255.255,所以不能处理该地址（被废弃）

`inet_ntoa` 32bit网络字节序到点分十进制IPV4字符串，储存在静态内存，不可重入

**IPV4/6**

```c
#include <arpa/inet.h>
int inet_pton(int family, const char *strptr, void *addrptr);
								Returns: 1 if OK, 0 if input not a valid presentation format, −1 on error

const char *inet_ntop(int family, const void *addrptr, char *strptr, size_t len);
								Returns: pointer to result if OK, NULL on error
```

p - presentation , n - numeric

`family` : AF_INET / AF_INET6 不支持 errno= `EAFNOSUPPORT`

`inet_pton` : 字符串 strptr 转换为 addrptr 二进制地址结果

`inet_ntop` :  相反，len 位 strptr 大小，防止溢出 - len太小，返回空指针 errno=`ENOSPC`

#### 读写函数

```c
#include "unp.h"
ssize_t readn(int filedes, void *buff, size_t nbytes);
ssize_t writen(int filedes, const void *buff, size_t nbytes);
ssize_t readline(int filedes, void *buff, size_t maxlen);
					All return: number of bytes read or written, −1 on error
```

`readline` 每次读一个字符，极端地慢

### Chapter 4

基本 TCP 套接字编程

#### Socket 函数

```c
#include <sys/socket.h>
int socket(int family, int type, int protocol);
				Returns: non-negative descriptor if OK, −1 on error
```

|                | AF_INET   | AF_INET   | AF_LOCAL | AF_ROUTE | AF_KEY |
| -------------- | --------- | --------- | -------- | -------- | ------ |
| SOCK_STREAM    | TCP\|SCTP | TCP\|SCTP | YES      |          |        |
| SOCK_DGRAM     | UDP       | UDP       | YES      |          |        |
| SOCK_SEQPACKET | SCTP      | SCTP      | YES      |          |        |
| SOCK_RAW       | IPV4      | IPV6      |          | YES      | YES    |

#### Connect 函数

```c
#include <sys/socket.h>
int connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen);
									Returns: 0 if OK, −1 on error
```

client 用于与 server 连接，内核会自己选择临时端口

* 75 s 无响应后返回 `ETIMEDOUT`
* 若相应 RST 则马上返回 `ECONNREFUSED` - 指定端口没有等待连接
* 目的不可达，返回 `EHOSTUNREACH`, `ENETUNREACH`

错误

* 如果给不存在的机器发送，因为没有 ARP reply , `ETIMEOUT`

  ```bash
  connect error: Connection timed out
  ```
  
* 如果给未运行 server 的机器发送，收到 RST ， `ECONNREFUSED`
  
  ```
  connect error: Connection refused
  ```
  
* 给不可达发送， 收到 ICMP 不可达错误， `EHOSTUNREACH`

  ```
  connect error: No route to host
  ```

每次 connect 失败后，都需要关闭 sockfd 重新调用 socket 函数

#### bind 函数

```c
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
									Returns: 0 if OK, −1 on error
```

32b ipv4 / 128b ipv6 + 16b TCP/UDP port number

`Servers` 会在启动时调用 bind 端口（程序定义） ，若没有则当调用 connect 或 listen 时，内核会选择一个临时端口  或 根据 SYN 的目的地址

`Client` 通常不会bind 而是 connect 时由内核根据路径选择

##### IP地址

wildcard 通配符

ipv4 : `INADDR_ANY` 0.0.0.0，内核等到TCP连接，UDP报文发送后选择ip地址

ipv6 : in6addr_any 由系统预先分配并置为`IN6ADDR_ANY_INIT`

RPC  例外，会通过 端口映射器注册

##### 错误

`EADDRINUSE` ： `Address already in use` 地址已使用

#### listen 函数

**Server** ： convert unconnected socket into a passive socket 

```c
#include <sys/socket.h>
int listen(int sockfd, int backlog);
					Returns: 0 if OK, −1 on error
```

`backlog` : 内核队列中排队的最大连接数 

调用时间： socket bind 后， accept 前

为 listening socket 保持两个队列

* `incomplete connection queue` 未完成连接队列 ， 未完成握手，`SYN_RCVD` 态
* `completed connection queue`  已完成连接队列 ， 完成握手 ， `ESTABLISHED` 态

两队之和不超过 backlog

#### accept 函数

 返回已完成连接队列队头，如果为空，进程睡眠

```c
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
						Returns: non-negative descriptor if OK, −1 on error
```

* `sockfd` : listening socket    监听
* `return` : connected socket 已连接
* `cliaddr` & `addrlen` ：对端的地址和长度 
* 一对多的关系，监听socket 保持打开，连接socket完成对一个客户的服务就关闭

##### 错误

* 非超级用户：

  ```bash
  bind error: Permission denied
  ```

#### fork 和 exec 函数

fork 是唯一生成新进程的函数

```c
#include <unistd.h>
pid_t fork(void);
			Returns: 0 in child, process ID of child in parent, −1 on error
```



重点基本都要求掌握 、 5.13不要求掌握、

6.9，6.20 不要求掌握

### Chapter 7

​	sockopt :掌握 7.2 原理 -  SO_LINGER / SO_KEEPALIVE / SO_DONTROUTE

 	IP : IP_HDRINCL / IP_TTL 

​	TCP : TCP_MAXSEG

### Chapter 8

​	8.1 图 ， 并发程序设计 - 重点注意区别

### Chapter 11

​	简单前面部分 DNS 操作 11.3、11.4、11.5

## Part 3 Advanced Sockets

12，13，14，15 ，17，18.5,20,21,22,23,24，30，31不考  Daemon Process 了解一下

19 Introduction

25 掌握，结合第 5 章看

26 掌握 基本概念，线程和进程区别

27 结合前面的 Options 一起看，简单看看。

28  重点掌握

29 掌握 基本概念，如何抓包



A.3 C.1  netstat tcpdump 

简答题（解释名词） ， 编程题（补充小的片段）上机+上课 ， 实验分析题（分析实验的结果）例如： 服务端没起来 报什么错误