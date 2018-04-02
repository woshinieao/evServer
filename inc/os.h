#ifndef OS_INCLUDE_H
#define OS_INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h> 

#include "event.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include "event2/util.h"
#include "event2/event.h"
#include "version.h"
#include <sys/types.h>

#ifdef WIN32
#include <sys/types.h>
#include <stdarg.h>
#include <direct.h>
#include <TlHelp32.h>
#include <malloc.h>
#include <time.h>
#include <wchar.h>
#include <math.h>
#include  <io.h>
#include <wtypes.h>
#include <process.h>
#else
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>
#endif


#ifdef WIN32
typedef HANDLE  SEM_T;
typedef HANDLE  pthread_t;
typedef int     pid_t;
typedef  __int64            INT64;
typedef unsigned __int64    UINT64;
typedef unsigned long DWORD;
typedef HANDLE  MUTEX_T;
//typedef  DWORD pthread_t;


#define SPC_LOG "spc.log"

/* 二进制信号量初始化 */
#define MUTEX_INIT(a)           (a = CreateMutex(NULL, FALSE,NULL))
#define MUTEX_LOCK(a)           ((WaitForSingleObject(a, INFINITE) == WAIT_OBJECT_0)?0:-1)
#define MUTEX_LOCK_ASYNC(a)     ((WaitForSingleObject(a ,0)   == WAIT_OBJECT_0)?0:-1)
#define MUTEX_LOCK_TIMEOUT(a,t) (mutex_lock_timeout(&a,t))
#define MUTEX_UNLOCK(a)         (ReleaseMutex(a)?0:-1)
#define MUTEX_DESTROY(a)        (CloseHandle(a))

#define SEM_INIT(a, b, c)       (a = CreateSemaphore(NULL,b,c,NULL))
#define SEM_WAIT(a)             ((WaitForSingleObject(a, INFINITE) == WAIT_OBJECT_0)?0:-1)
#define SEM_WAIT_TIMEOUT(a,t)   (sem_wait_timeout(&a,t))
#define SEM_WAIT_ASYNC(a)       ((WaitForSingleObject(a ,0) == WAIT_OBJECT_0) ?0:-1)
#define SEM_POST(a)             (ReleaseSemaphore(a,1,NULL)?0:-1)
#define SEM_DESTROY(a)          (CloseHandle(a))

#define pthread_create(thrp, attr, func, arg)                               \
    	(((*(thrp) = CreateThread(NULL, 0,                                     \
  		(LPTHREAD_START_ROUTINE)(func), (arg), 0, NULL)) == NULL) ? -1 : 0)
#define pthread_join(thr, statusp)                                          \
    	((WaitForSingleObject((thr), INFINITE) == WAIT_OBJECT_0) &&            \
   		GetExitCodeThread((thr), (LPDWORD)(statusp)) ? 0 : -1)
#define pthread_cancel(thr)  TerminateThread(thr,0)
#define pthread_timedjoin_np(thr, statusp,t)                               \
    	((WaitForSingleObject((thr),t) == WAIT_OBJECT_0) &&            \
    	GetExitCodeThread((thr), (LPDWORD)(statusp)) ? 0 : -1)
#define pthread_self()		((pthread_t)GetCurrentThreadId())	
#define pipe(fd)   			CreatePipe( (HANDLE *)&fd[0], (HANDLE *)&fd[1], NULL, 0)
#define readlink(a,b,c) 	GetModuleFileName(NULL,(LPWSTR)b,c)
#define socket_close(a) 	closesocket(a)
#define shut_down(a) 		shutdown(a,2)
#define MSG_NOSIGNAL 		0
#define SHUT_RDWR   		SD_BOTH
#define SHUT_RD     		SD_RECEIVE
#define SHUT_WR     		SD_SEND
#define access 				_access
#define mkdir(path,mode)  	_mkdir(path)
typedef int socklen_t ;


#else  //LINUX
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>

#ifndef SPC_LOG
#define SPC_LOG  "/var/log/evServer.log"
#endif




typedef int					SOCKET;
typedef long long           INT64;
typedef unsigned long long  UINT64;
typedef unsigned int		COLORKEY;

#define GetSockErr()	    errno
#define GetLastError()      errno
#define closesocket	        close

typedef struct sockaddr_in  SOCKADDR_IN;
typedef struct sockaddr     SOCKADDR;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  -1
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR    -1
#endif
#define  WSAGetLastError() errno
#define  SS_PORT(ssp) (((struct sockaddr_in*)(ssp))->sin_port)
#define  ioctlsocket  ioctl
typedef struct addrinfo     ADDRINFO;


#define _strnicmp  strncasecmp


typedef sem_t   SEM_T;
typedef pthread_mutex_t  MUTEX_T;

/* 二进制信号量初始化,采用默认属性*/
#define MUTEX_INIT(a)           (pthread_mutex_init(&(a), NULL))
#define MUTEX_LOCK(a)           (pthread_mutex_lock(&(a)))
#define MUTEX_LOCK_ASYNC(a)     (pthread_mutex_trylock(&(a)))
#define MUTEX_LOCK_TIMEOUT(a,t) (mutex_lock_timeout(&a,t))
#define MUTEX_UNLOCK(a)         (pthread_mutex_unlock(&(a)))
#define MUTEX_DESTROY(a)        (pthread_mutex_destroy(&(a)))

#define SEM_INIT(a, b, c)       (sem_init(&(a), 0, b))
#define SEM_WAIT(a)             (sem_wait(&(a)))
#define SEM_WAIT_TIMEOUT(a,t)   (sem_wait_timeout(&a,t))
#define SEM_WAIT_ASYNC(a)       (sem_trywait(&(a)))
#define SEM_POST(a)             (sem_post(&(a)))
#define SEM_DESTROY(a)          (sem_destroy(&(a)))

#define Sleep(mSec)             usleep(mSec*1000)

#define MAKEWORD(a, b)          ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)          ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)               ((WORD)(l))
#define HIWORD(l)               ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)               ((BYTE)(w))
#define HIBYTE(w)               ((BYTE)(((WORD)(w) >> 8) & 0xFF))


#define shut_down(a) 			shutdown(a,2)
#define socket_close(a) 		close(a)


#endif //LINUX




#define ITEMS_PER_ALLOC 64


enum item_lock_types {
    ITEM_LOCK_GRANULAR = 0,
    ITEM_LOCK_GLOBAL
};
enum conn_states {
	conn_listening,  /**< the socket which listens for connections */
	conn_new_cmd,	 /**< Prepare connection for next command */
	conn_waiting,	 /**< waiting for a readable socket */
	conn_read,		 /**< reading in a command line */
	conn_parse_cmd,  /**< try to parse a command from the input buffer */
	conn_write, 	 /**< writing out a simple response */
	conn_nread, 	 /**< reading in a fixed number of bytes */
	conn_swallow,	 /**< swallowing unnecessary bytes w/o storing */
	conn_closing,	 /**< closing this connection */
	conn_mwrite,	 /**< writing out many items sequentially */
	conn_max_state	 /**< Max state value (used for assertion) */
};



typedef struct conn {
    int    sfd;  //socket
	int pthread_number;
    conn_states sock_state;
    struct event *event;
    short  ev_flags;
	time_t timeval;
	struct sockaddr_in client_addr;
	MUTEX_T lock;
	struct conn *next;
}conn;



typedef struct conn_queue_item 
 {
    int               sfd; //socket
    conn			  *connection; //鐩墠鏈捣浣滅敤
    int               event_flags;
  //  int               read_buffer_size;   
    struct conn_queue_item          *next;
}CQ_ITEM;




typedef struct conn_queue {
    CQ_ITEM *head;
    CQ_ITEM *tail;
    MUTEX_T lock;
   // pthread_cond_t  cond;
}CQ;



typedef struct {  
    //绾跨▼ID  
    pthread_t thread_id;        /* unique ID of this thread */  
    //绾跨▼鐨? event_base锛屾瘡涓嚎绋嬮兘鏈夎嚜宸辩殑event_base  
    struct event_base *base;    /* libevent handle this thread uses */  
    //寮傛event浜嬩欢  
    struct event notify_event;  /* listen event for notify pipe */  
    //绠￠亾鎺ユ敹绔?  
    int notify_receive_fd;      /* receiving end of notify pipe */  
    //绠￠亾鍙戦?佺  
    int notify_send_fd;         /* sending end of notify pipe */  
    //绾跨▼鐘舵??  
  //  struct thread_stats stats;  /* Stats generated by this thread */  
    //鏂拌繛鎺ラ槦鍒楃粨鏋?  
    struct conn_queue *new_conn_queue; /* queue of new connections to handle */  
  
 
} LIBEVENT_THREAD;  


typedef struct {
    pthread_t thread_id;        /* unique ID of this thread */
    struct event_base *base;    /* libevent handle this thread uses */
} LIBEVENT_DISPATCHER_THREAD;




#endif

