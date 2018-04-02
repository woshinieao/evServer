#ifndef _EV_SERVER_H
#define _EV_SERVER_H


#define EV_SDK_API
typedef int (*cbNotify)(char *buff,int len);

/**********************************************************************************
* Function:      EV_set_thread_num
* Description:   �����̸߳����� Ĭ���̸߳���Ϊ5��
* Input:         int count  �̸߳�������
* Output:        ��
* Return:        -1��ʾʧ�ܣ�0 �ɹ�
************************************************************************************/ 
EV_SDK_API int EV_set_thread_num(int count);


/**********************************************************************************
* Function:      EV_start
* Description:   �����������
* Input:         ��
* Output:        ��
* Return:        -1��ʾʧ�ܣ�0 �ɹ�
************************************************************************************/
EV_SDK_API int EV_start();	

/**********************************************************************************
* Function:      EV_set_notify
* Description:   ���÷���������ݻص�
* Input:         cbMsg cbf �ص�����
* Output:        ��
* Return:        -1��ʾʧ�ܣ�0 �ɹ�
************************************************************************************/
EV_SDK_API int EV_set_notify(cbNotify cbf);


/**********************************************************************************
* Function:      EV_set_timeout
* Description:   ���������豸���ʱ��
* Input:         int check ʱ����
* Output:        ��
* Return:        -1��ʾʧ�ܣ�0 �ɹ�
************************************************************************************/
EV_SDK_API int EV_set_timeout(int check);


#endif
