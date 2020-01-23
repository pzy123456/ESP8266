#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "tcpclient.h"
#include <string.h>
  
#define SERVERIP    "192.168.0.191"
#define SERVERPORT  8000
/******************************************************************************
 * FunctionName : ATaskTcpClient
 * Description  : ATaskTcpClient
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ATaskTcpClient( void *pvParameters )
{
	/* ��������ͨ����һ��������������������������ÿ������ʵ������һ�������Լ���iVarialbleExample��
	���������iVariableExample������Ϊstatic����һ���򲻳��� �C ���������ֻ����һ�����������е�����ʵ
	�����Ṳ����������� */
	int iVariableExample = 0;
	/* ����һ��״̬���� */
	STATION_STATUS Status;
	struct sockaddr_in socketaddr;
	int fd = -1;//-1��ʾerror
	int ret = -1;
	unsigned char Msgbuff[48];
	int overtime = 5000;
	do{

		Status = wifi_station_get_connect_status();
		vTaskDelay(100);/* ���û�л�ȡ��ip�ͽ���CPU�ͷ�ռ��Ȩ */
	}while(Status != STATION_GOT_IP);
	/* ����socket */
	fd = socket(PF_INET, SOCK_STREAM, 0);
	if(-1 == fd)
	{
		printf("socket error\n");
		vTaskDelete(NULL);
		return;
	}

	/* ���ý��ճ�ʱʱ�� */
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &overtime, sizeof(int));

	memset(&socketaddr, 0, sizeof(socketaddr));
	/* ��ֵserver��Ϣ */
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_addr.s_addr = inet_addr(SERVERIP);
	socketaddr.sin_port = htons(SERVERPORT);
	socketaddr.sin_len = sizeof(socketaddr);


	/* �ͻ������ӷ����� */
	do
	{
		ret = connect(fd, (struct sockaddr *)&socketaddr,\
					(socklen_t)socketaddr.sin_len);
		if(-1 == ret)
		{
			printf("connect fail\n");
			vTaskDelay(100);
		}
	}while(ret != 0);/* ���ɹ�һֱ������ */
	/* ����ͨ��ʵ����һ����ѭ���С�*/
	for( ;; )
	{
		/* ������Ϣ��server������ */
		send(fd, "I am tcpclient", sizeof("I am tcpclient"),0);
		printf("��������\n");
		do
		{
			ret = recv(fd ,Msgbuff, 48, 0);
			if(ret > 0)
			{
				printf("tcpclient data is %s\n",Msgbuff);
			}else
			{
				printf("tcpclient no data\n");
				vTaskDelay(100);
			}
		}while(ret == -1);
		

}
	/* �������ľ���ʵ�ֻ������������ѭ���������������ں���������֮ǰɾ��������NULL������ʾɾ��
	���ǵ�ǰ���� */
	vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : TcpClient_init
 * Description  : TcpClient_init TCP�ͻ��˳�ʼ��
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void TcpClient_init(void)
{
	xTaskCreate(ATaskTcpClient, "ATaskTcpClient", 256, NULL, 4, NULL);
}


