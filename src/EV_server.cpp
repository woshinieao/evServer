#include "EV_server.h"
#include "os.h"

static struct event_base* main_base;
static int m_ThreadCnt = 5;

static cbNotify cbnotify = NULL;
static LIBEVENT_THREAD *m_pThread;
static conn *m_pConnList= NULL;
static struct event main_event;
static int tid_pre=0;
static int timecheck = 180;


int ServerLog(const char *format ,... )
{
	static FILE *fpLog = NULL;
	int result = 0;
	struct stat buf;
	
	result = stat( SPC_LOG, &buf );
	if(result != 0)  //不存在
	{
		
		fpLog = fopen(SPC_LOG,"w");
		printf("file not exsist !\n");
	}
	else 
	{
		if( buf.st_size > (500*1024*1024))
		{
			if(fpLog !=NULL)
				fclose(fpLog);
			fpLog = fopen(SPC_LOG,"w");
		
		}
		else
		{
			if(fpLog == NULL)
				fpLog = fopen(SPC_LOG,"a");	
		}

	}

	if(fpLog == NULL)
		return -1;

	
	va_list args;
 
	char szDebugBuf[2048] = {0};
	int iPos = 0 ;
	int iLen;
	time_t timer;
	struct tm tm_now;
	char asctimeBuf[2048]={0};

	(void)time(&timer);
#ifdef WIN32

    ctime_s(asctimeBuf,2048,&timer);
#else
	localtime_r(&timer,&tm_now);
	sprintf(asctimeBuf,"%d-%02d-%02d-%02d-%02d-%02d",tm_now.tm_year+1900,tm_now.tm_mon+1,tm_now.tm_mday,tm_now.tm_hour,tm_now.tm_min,tm_now.tm_sec);
#endif
	
	va_start(args, format);
	iLen = vsprintf (szDebugBuf + iPos, format, args);
	if(iLen<0)
	{
		va_end (args);
		return -1;
	}
	va_end (args);

	iPos += iLen;
	iLen = sprintf(szDebugBuf+iPos, " %s\n",asctimeBuf);
	iPos += iLen;
	szDebugBuf[iPos] = '\0' ;

	if (fpLog != NULL)
	{
		fputs(szDebugBuf, fpLog);
	//	fputs("\n", fpLog);
		fflush(fpLog);
	}
	
	return (iLen + iPos);	
}




static void cq_init(CQ *cq) {
    MUTEX_INIT(cq->lock);
    //pthread_cond_init(&cq->cond, NULL);
    cq->head = NULL;
    cq->tail = NULL;
}

/*
 * Looks for an item on a connection queue, but doesn't block if there isn't
 * one.
 * Returns the item, or NULL if no item is available
 *出栈
 */
static CQ_ITEM *cq_pop(CQ *cq) {
    CQ_ITEM *item;

    MUTEX_LOCK(cq->lock);
    item = cq->head;
    if (NULL != item) {
        cq->head = item->next;
        if (NULL == cq->head)
            cq->tail = NULL;
    }
    MUTEX_UNLOCK(cq->lock);
	

    return item;
}

/*
 * Adds an item to a connection queue.
 * 进栈
 */
static void cq_push(CQ *cq, CQ_ITEM *item) 
{


    item->next = NULL;
    MUTEX_LOCK(cq->lock);
	
    // 鍦ㄩ槦灏炬坊鍔?
    if (NULL == cq->tail)
        cq->head = item;
    else
        cq->tail->next = item;
    cq->tail = item;
   //pthread_cond_signal(&cq->cond);
    MUTEX_UNLOCK(cq->lock);
}

/*
 * Returns a fresh connection queue item.
 */
static CQ_ITEM *cqi_new(void) {
    CQ_ITEM *item = NULL;
	if (NULL == item) 
	{
		/* Allocate a bunch of items at once to reduce fragmentation */
		item = (CQ_ITEM * )malloc(sizeof(CQ_ITEM) * ITEMS_PER_ALLOC);
		if (NULL == item)
		    return NULL;
	}

    return item;
}


/*
 * Frees a connection queue item (adds it to the freelist.)
 */
static void cqi_free(CQ_ITEM *item) {
	if(item !=NULL)
		delete(item);
	item = NULL;
}


SOCKET create_socket(char *ip,int port)
{
	
	int sock;
	struct sockaddr_in server_addr;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock <0)
	{
	perror( "socket error" ) ; 
	return -1; 

	}
	int yes = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(int));

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	if(ip == NULL)
		server_addr.sin_addr.s_addr = INADDR_ANY;
	else
		server_addr.sin_addr.s_addr = inet_addr(ip);
	if(bind(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr))<0)
	{
		perror( "listen error" ) ; 
		return -1; 

	}
	if(listen(sock, 20) <0)
	{
	    ServerLog("Server Listen Failed!");
	   	return -1;
	}
	return sock;

}


int set_nonblock(int fd)  
{  
     
#ifdef WIN32
	unsigned long ul = 1; 
    ioctlsocket(fd, FIONBIO, &ul);
#else
	int flags; 
    if ((flags = fcntl(fd, F_GETFL)) == -1) {  
        return -1;  
    }  
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {  
        return -1;  
    }
#endif
    return 0;  
} 



/*子线程接受数据处理*/
static void thread_recv_data(const int fd, const short which, void *arg) 
{

	conn *c = (conn *)arg;
	int nread=0;
	unsigned char buff[1024];
	char buffs[5];	
	
	memset(buff,0,1024);
	nread = recv(fd,(char *)buff,1024,0);
	if(nread<0)
		return;
	memcpy(buffs,buff,5);
	if((nread == 0)||(nread== 5 &&strcmp(buffs,"close") == 0) )
	{
		ServerLog("【handler】 id:%lu disconnect :%d\n",pthread_self(),c->sfd);
		MUTEX_LOCK(c->lock);
		event_del(c->event);
		//SocketClose(c->sfd);
		shut_down(c->sfd);
		c->sfd=0;
		c->sock_state = conn_closing;
		
		MUTEX_UNLOCK(c->lock);  
	}
	else if((nread== 5) && strcmp(buffs,"alive") == 0)
	{
		MUTEX_LOCK(c->lock);
		c->timeval = time(NULL);
		ServerLog("銆恆live  銆? connect sock :%d\n",c->sfd);
		MUTEX_UNLOCK(c->lock);
	
	}
	else 
	{
		if(cbnotify != NULL)
			cbnotify(buffs,nread);

	}
    return;
}


static void work_tasklet(int fd, short which, void *arg) 
{
    LIBEVENT_THREAD *me = (LIBEVENT_THREAD *)arg;
    CQ_ITEM *item;
    char buf[1];
	//DWORD dwRead; 

	if(recv(fd, buf, 1, 0) !=1)
         ServerLog("Can't read from libevent pipe\n");	
	item = cq_pop(me->new_conn_queue);	
    switch (buf[0]) 
	{
		case 'd':
			// 取出一个任务
			if (NULL != item) 
			{
				item->connection->event = event_new(me->base, item->sfd, EV_READ|EV_PERSIST, thread_recv_data,(void *)(item->connection));
				event_add(item->connection->event,NULL);
			}
			break;
		case 'c':

			MUTEX_LOCK(item->connection->lock);
			event_del(item->connection->event);
			if(item->connection->sock_state != conn_closing)
			{
				socket_close(item->sfd);
				item->connection->sock_state = conn_closing;
				ServerLog("銆恜rocess銆? id:%lu disconnect :%d\n",pthread_self(),item->sfd);
				//ServerLog("pthread close socket:%d pthread:%d notify_fd:%d \n",item->sfd,item->connection->pthread_number,m_pThread[item->connection->pthread_number].notify_send_fd);
				
			}
			free(item->connection);
			MUTEX_UNLOCK(item->connection->lock); 
			
			break;
		default:
			break;
	}
	cqi_free(item);
}


static void setup_thread(LIBEVENT_THREAD *me)
{
	me->base = event_init();
	if (! me->base) {
		ServerLog("Can't allocate event base\n");
		exit(1);
	}

	// 在线程数据结构初始化的时候, 为 me->notify_receive_fd 读管道注册读事件, 回调函数是 work_task_process()
	event_set(&me->notify_event, me->notify_receive_fd,EV_READ | EV_PERSIST, work_tasklet, me);
	event_base_set(me->base, &me->notify_event);

	if (event_add(&me->notify_event, 0) == -1) {
		fprintf(stderr, "Can't monitor libevent notify pipe\n");
		exit(1);
	}
	// 初始化该线程的工作队列
	me->new_conn_queue =  (struct conn_queue *)malloc(sizeof(struct conn_queue));
	if (me->new_conn_queue == NULL) {
		ServerLog("Failed to allocate memory for connection queue\n");
		exit(EXIT_FAILURE);
	}

	cq_init(me->new_conn_queue);
	// 初始化该线程的状态互斥量

	
}


static void *set_notify_fd(void *arg)
{
	SOCKET *info = (SOCKET *)arg;
	SOCKET sock = *info;
	int numAccept = 0;
	struct sockaddr_in client_addr;
	socklen_t sin_size = sizeof(struct sockaddr_in);
	
	while(1)
	{
		int accept_fd = accept(sock, (struct sockaddr*)&client_addr, (socklen_t *)&sin_size);
		set_nonblock(accept_fd) ;
		m_pThread[numAccept].notify_send_fd = accept_fd;
		numAccept++;
		if(numAccept>=m_ThreadCnt)
			return NULL;
	}

	ServerLog(" set_notify_fd return! \n");
	
	
}


static void *check_state_thread(void *arg)
{
	conn *tmp=NULL ;
	conn *pre=NULL ;
	char buff[1];
	buff[0] = 'c';
	Sleep(10*1000);
	 while(1)
	{	
	
		pre = m_pConnList;
		tmp = pre;
		Sleep(timecheck*1000);
		time_t now = time(NULL);
		/*socket瓒呮椂妫?鏌?*/
		do{
			tmp= tmp->next;	
			if(tmp ==NULL)
				break;				
			if((now-tmp->timeval)>(timecheck) && tmp->sfd >0 )
			{
				CQ_ITEM *item = cqi_new();
				item->sfd = tmp->sfd;
				item->connection = tmp;
				cq_push(m_pThread[tmp->pthread_number].new_conn_queue, item);
				ServerLog("【beat   】 timeout socket:%d \n",tmp->sfd);
				
				if (send(m_pThread[tmp->pthread_number].notify_send_fd, buff, 1,0) != 1) 
				{
				  perror("Writing to thread notify pipe");
				}			
			}
			pre->next =  tmp->next;
		}while(tmp !=NULL);
	}

	return NULL;
}



/* 工作线程启动libevent监听*/
static void *worker_libevent_start(void *arg) 
{
    LIBEVENT_THREAD *me = (LIBEVENT_THREAD *)arg;
	me->thread_id = (pthread_t)pthread_self();
	ServerLog("event_base_loop worker thread ! %ld \n",pthread_self());
    event_base_loop(me->base, 0);
    return NULL;
}


static void create_thread(void *(*func)(void *), void *arg) {
    pthread_t       thread;
  //  pthread_attr_t  attr;
    int             ret;

   // pthread_attr_init(&attr);

    if ((ret = pthread_create(&thread, NULL, func, arg)) != 0) {
        fprintf(stderr, "Can't create thread: %s\n",
                strerror(ret));
        exit(1);
    }
}



void thread_driver(int sock, short event, void* arg)
{
	
//	struct sockaddr_in cli_addr;
    int accept_fd, sin_size;
	char buf[1];
	buf[0] = 'd';
	conn *c = (conn *)malloc(sizeof(conn));
	MUTEX_INIT(c->lock);  
	c->sock_state = conn_listening;
	/*鎺ュ彈杩炴帴*/
	sin_size = sizeof(struct sockaddr_in);
	accept_fd = accept(sock, (struct sockaddr*)&c->client_addr, (socklen_t *)&sin_size);
	set_nonblock(accept_fd) ; 

	/*分配工作线程*/
	int tid = (tid_pre + 1) % m_ThreadCnt;
	LIBEVENT_THREAD *tmpthread = m_pThread+tid;// 定位到下一个线程信息
	tid_pre = tid;

	/*创建连接信息节点，并加入到链表*/
	c->timeval = time(NULL);
	c->sfd = accept_fd;
	c->pthread_number = tid;
	c->next = m_pConnList->next;
	m_pConnList->next=c;

	

	/*创建工作队列item，入队列 */
	CQ_ITEM *item = cqi_new();
	item->sfd = accept_fd;
	item->connection = c;
	cq_push(tmpthread->new_conn_queue, item);
	if(send(tmpthread->notify_send_fd, buf, 1, 0)!=1)
		perror("Writing to thread notify pipe");
	
}





EV_SDK_API int EV_start()
{
    int i;

#ifdef WIN32
	WSADATA wsaData;
    int result;
    
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if ( result != 0 ) 
      return  -1;
#endif
    m_pThread = (LIBEVENT_THREAD *)calloc(m_ThreadCnt, sizeof(LIBEVENT_THREAD)); // LIBEVENT_THREAD 是结合 libevent 使用的结构体, event_base, 读写管道
    if (! m_pThread) 
	{
        perror("Can't allocate thread descriptors");
        return  -1;
    }
	m_pConnList = (struct conn *)malloc(sizeof(struct conn));
	if (! m_pConnList) 
	{
        perror("Can't allocate thread descriptors");
       return  -1;
    }
	memset(m_pConnList,0,sizeof(struct conn));
	main_base = event_base_new();


	/*创建任务通知服务器的套接字*/
	SOCKET sockTask = create_socket(NULL, 13000);
	if(sockTask<0)
	{
		perror("Server Socket :");	
		return  -1;
	}


	/*获取通知线程的socket*/
	create_thread(set_notify_fd, &sockTask);
	
    //main_base 分发任务的线程, 即主线程
    //管道, libevent 通知用的
    for (i = 0; i < m_ThreadCnt; i++) 
	{
      	

		SOCKET sk =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   
		SOCKADDR_IN addrSrv;  
		int err;
		addrSrv.sin_family = AF_INET;
#ifdef WIN32		
		addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");      // 本地回路地址是127.0.0.1;   
#else
		addrSrv.sin_addr.s_addr=inet_addr("127.0.0.1");
#endif
		addrSrv.sin_port = htons(13000);  
		if((err = connect(sk, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))) == SOCKET_ERROR)
		{ 	
			perror("connect:");
			closesocket(sk); 
			return  -1;
		}

        m_pThread[i].notify_receive_fd = sk;// 读管道
	//	ServerLog("\n------------------------\nnotify_receive_fd[%d]:%d\n",i,sk);
        setup_thread(&m_pThread[i]);  // 初始化线程信息数据结构, 其中就将 event 结构体的回调函数设置为 thread_libevent_process()
    }

	
	
	/*创建工作线程*/
    for (i = 0; i < m_ThreadCnt; i++)
        create_thread(worker_libevent_start, &m_pThread[i]);
	/*创建心跳及状态检查线程*/
	create_thread(check_state_thread, m_pConnList);


	/*启动主线程libevent监听是否有设备连接*/
	int sock =create_socket(NULL, 12000);
	if(sock<0)
	{
		perror("Server Socket :");	
		exit(-1);
	}
	event_set(&main_event, sock,  EV_READ|EV_PERSIST, thread_driver, (void *)&m_ThreadCnt);
	event_base_set(main_base, &main_event);
	event_add(&main_event,NULL);
	event_base_dispatch(main_base);

	return 0;
}




EV_SDK_API int EV_set_thread_num(int count)
{
	m_ThreadCnt = count;
	return 0;
}


EV_SDK_API int EV_set_notify(cbNotify cbf)
{
	cbnotify = cbf;
	return 0;
}


EV_SDK_API int EV_set_timeout(int checkTime)
{
	timecheck = checkTime;
	return 0;

}



