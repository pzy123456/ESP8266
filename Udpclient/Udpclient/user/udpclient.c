#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "udpclient.h"
#include <string.h>

#define SERVERIP    "192.168.0.191"
#define SERVERPORT  8000
/******************************************************************************
 * FunctionName : ATaskUdpClient
 * Description  : ATaskUdpClient
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ATaskUdpClient( void *pvParameters )
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
socketaddr.sin_addr.s_addr = inet_addr(SERVERIP);
socketaddr.sin_port = htons(SERVERPORT);
socketaddr.sin_len = sizeof(socketaddr);
/* ����ͨ��ʵ����һ����ѭ���С�*/
for( ;; )
{
	/* ������Ϣ��server������ */
	sendto(fd, "I am udpclient", sizeof("I am udpclient"),\
     0,(struct sockaddr *)&socketaddr, (socklen_t)socketaddr.sin_len);
	printf("��������\n");
	do
	{
		ret = recvfrom(fd, Msgbuff, 48, 0,\
	     (struct sockaddr *)&socketaddr, (socklen_t *)&socketaddr.sin_len);
		if(ret > 0)
		{
			printf("data is %s\n",Msgbuff);
		}else
		{
			printf("no data\n");
		}
	}while(ret == -1);
	

}
	/* �������ľ���ʵ�ֻ������������ѭ���������������ں���������֮ǰɾ��������NULL������ʾɾ��
	���ǵ�ǰ���� */
	vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : UdpClient_init
 * Description  : UdpClient_init UDP�ͻ��˳�ʼ��
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void UdpClient_init(void)
{
	xTaskCreate(ATaskUdpClient, "ATaskUdpClient", 256, NULL, 4, NULL);
}

