#include "esp_common.h"
#include "Esp_libc.h"
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "tcpserver.h"
#include "relay.h"
#include <string.h>
#include "gpio.h"

#define SERVERIP    "192.168.0.191"
#define SERVERPORT  8000


/******************************************************************************
 * FunctionName : ATaskTcpServer
 * Description  : ATaskTcpServer
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ATaskTcpServer( void *pvParameters )
{
	/* ��������ͨ����һ��������������������������ÿ������ʵ������һ�������Լ���iVarialbleExample��
	���������iVariableExample������Ϊstatic����һ���򲻳��� �C ���������ֻ����һ�����������е�����ʵ
	�����Ṳ����������� */
	int iVariableExample = 0;
	/* ����һ��״̬���� */
	STATION_STATUS Status;
	struct sockaddr_in socketaddr;
	struct sockaddr ClientAddr;
	socklen_t ClientAddrLen = sizeof(struct sockaddr);
	int fd = -1;//-1��ʾerror
	int ret = -1;
	int cmd;
	int cfd = -1;
	unsigned char Msgbuff[48];//��Ϊ�������ݽ϶����Բ���ջ���ö�
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
	printf("socket ok\r\n");
	/* ���ý��ճ�ʱʱ�� */
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &overtime, sizeof(int));

	memset(&socketaddr, 0, sizeof(socketaddr));
	/* ��ֵserver��Ϣ */
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_addr.s_addr = INADDR_ANY;
	socketaddr.sin_port = htons(SERVERPORT);
	socketaddr.sin_len = sizeof(socketaddr);

	if(bind(fd, (struct sockaddr *)&socketaddr, (socklen_t) socketaddr.sin_len) != 0)
	{
		printf("bind error\n");
		vTaskDelete(NULL);
		return;
	}
	printf("bind ok\r\n");

	if(listen(fd, 10) != 0)
	{
		printf("listen error\n");
		vTaskDelete(NULL);
		return;
	}
	printf("listen ok\r\n");

	/* ����ͨ��ʵ����һ����ѭ���С�*/
	for( ;; )
	{
		/* ���������տͻ��˷��������,���ص��ǿͻ��˵��׽��� */
		cfd= accept(fd, &ClientAddr,&ClientAddrLen);
		if(-1 != cfd)
		{
			ret = recv(cfd,Msgbuff, 48, 0);
			if(ret > 0)
			{
				printf("accept ok\n");
				printf("tcpclient data is %s\n",Msgbuff);
				/* �и�bug,�������Ƿ�relayon �������жϵ����ַ�����relayoff,��Ȼ�յ����ַ�����relayon*/
				if(strncmp((char *)Msgbuff, "relayon", 7))
				{
					//Relay_Control(0);
					gpio16_output_set(0);//Ĭ�ϵ���
					printf("led on\n");
					/* ������Ϣ��server������ */
					send(cfd, "relayon", strlen("relayon"), 0);
				}
				else if(strncmp((char *)Msgbuff, "relayoff", strlen("relayoff")))
				{
					//Relay_Control(1);
					gpio16_output_set(1);
					printf("led off\n");
					/* ������Ϣ��server������ */
					send(cfd, "relayoff", strlen("relayoff"), 0);
				}
				else
					/* ������Ϣ��server������ */
					send(cmd, "cmd error", strlen("cmd error"), 0);
			}else
			{
				printf("tcpclient no data\n");
				vTaskDelay(100);
			}
		}	
		/* �����close��accept��-1 */
		close(cfd);
	}
	/* �������ľ���ʵ�ֻ������������ѭ���������������ں���������֮ǰɾ��������NULL������ʾɾ��
	���ǵ�ǰ���� */
	vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : TcpServer_init
 * Description  : TcpServer_init TCP��������ʼ��
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void TcpServer_init(void)
{
	xTaskCreate(ATaskTcpServer, "ATaskTcpServer", 256, NULL, 4, NULL);
}




