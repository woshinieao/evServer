#ifndef _EV_SERVER_H
#define _EV_SERVER_H


#define EV_SDK_API
typedef int (*cbNotify)(char *buff,int len);

/**********************************************************************************
* Function:      EV_set_thread_num
* Description:   设置线程个数， 默认线程个数为5。
* Input:         int count  线程个数参数
* Output:        无
* Return:        -1表示失败；0 成功
************************************************************************************/ 
EV_SDK_API int EV_set_thread_num(int count);


/**********************************************************************************
* Function:      EV_start
* Description:   启动服务程序
* Input:         无
* Output:        无
* Return:        -1表示失败；0 成功
************************************************************************************/
EV_SDK_API int EV_start();	

/**********************************************************************************
* Function:      EV_set_notify
* Description:   设置服务程序数据回调
* Input:         cbMsg cbf 回调函数
* Output:        无
* Return:        -1表示失败；0 成功
************************************************************************************/
EV_SDK_API int EV_set_notify(cbNotify cbf);


/**********************************************************************************
* Function:      EV_set_timeout
* Description:   设置连接设备检查时间
* Input:         int check 时间间隔
* Output:        无
* Return:        -1表示失败；0 成功
************************************************************************************/
EV_SDK_API int EV_set_timeout(int check);


#endif
