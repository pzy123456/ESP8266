#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "udpserver.h"
#include <string.h>

#define SERVERIP    "192.168.0.191"
#define SERVERPORT  8000
/******************************************************************************
 * FunctionName : ATaskUdpServer
 * Description  : ATaskUdpServer
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ATaskUdpServer( void *pvParameters )
{
	/* ��������ͨ����һ��������������������������ÿ������ʵ������һ�������Լ���iVarialbleExample��
	���������iVariableExample������Ϊstatic����һ���򲻳��� �C ���������ֻ����һ�����������е�����ʵ
	�����Ṳ����������� */
	int iVariableExample = 0;
	/* ����һ��״̬���� */
	STATION_STATUS Status;
	struct sockaddr_in socketaddr;
	struct sockaddr from;/* �ͻ�����Ϣ */
	socklen_t fromlen = sizeof(struct sockaddr);
	int fd = -1;//-1��ʾerror
	int ret = -1;
	unsigned char Msgbuff[48];
	int overtime = 5000;
	do{

		Status = wifi_station_get_connect_status();
		vTaskDelay(100);/* ���û�л�ȡ��ip�ͽ���CPU�ͷ�ռ��Ȩ */
	}while(Status != STATION_GOT_IP);
	/* ����socket */
	fd = socket(PF_INET, SOCK_DGRAM, 0);
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
	socketaddr.sin_addr.s_addr = INADDR_ANY;//�������˽����κε�ַ
	socketaddr.sin_port = htons(SERVERPORT);
	socketaddr.sin_len = sizeof(socketaddr);

	ret = bind(fd, (struct sockaddr *)&socketaddr, \
		(socklen_t)socketaddr.sin_len);
	if(ret == -1)
	{
		printf("bind error\n");
		vTaskDelete(NULL);
		return;
	}
	/* ����ͨ��ʵ����һ����ѭ���С�*/
	for( ;; )
	{
		do
		{
			/* ���տͻ��˵����� */
			ret = recvfrom(fd, Msgbuff, 48, 0,\
		     (struct sockaddr *)&from, (socklen_t *)&fromlen);
			if(ret > 0)
			{
				printf("udpclient data is %s\n",Msgbuff);
			}else
			{
				printf("udpclient no data\n");
			}
		}while(ret == -1);

		/* ������Ϣ��client�ͻ��� */
		sendto(fd, "I am udpserver", sizeof("I am udpserver"),\
	     0,&from, fromlen);
		printf("��������\n");

	}
		/* �������ľ���ʵ�ֻ������������ѭ���������������ں���������֮ǰɾ��������NULL������ʾɾ��
		���ǵ�ǰ���� */
		vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : UdpServer_init
 * Description  : UdpServer_init UDP��������ʼ��
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void UdpServer_init(void)
{
	xTaskCreate(ATaskUdpServer, "ATaskUdpServer", 256, NULL, 4, NULL);
}


