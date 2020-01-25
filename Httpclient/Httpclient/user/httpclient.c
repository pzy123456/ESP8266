
#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "httpclient.h"
#include <string.h>
  
//#define SERVERIP    "192.168.0.191"
#define SERVERPORT  80

const char GetStr[] = "GET /index.html HTTP/1.1\r\n" 
"Host: www.baidu.com\r\n\r\n";

#define HTTP_RCV_LEN   50000
unsigned char Msgbuff[50000];
                
/******************************************************************************
 * FunctionName : ATaskHttpClient
 * Description  : ATaskHttpClient
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ATaskHttpClient( void *pvParameters )
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
	unsigned char i;
	struct hostent* phostname;
	int overtime = 5000;
	char *ipaddr = NULL;
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

	/* ͨ��������ȡ�ҵ�ַ */
	phostname = gethostbyname("www.baidu.com");
	if(NULL == phostname)
	{
		printf("host fail\n");
		vTaskDelete(NULL);
		return;
	}
	else
	{
		/* ���ָ�����鲻Ϊ��,��ӡ������İٶ�ip��ַ */
		for(i=0;phostname->h_addr_list[i];i++)
		{
			/* �������ֽ���ת���ɵ�ָ�ʽip */
			/* *(struct in_addr*) */
			ipaddr= inet_ntoa(*(struct in_addr*)phostname->h_addr);
			if(NULL != ipaddr)
			{
				printf("host name is %s\n",ipaddr);
				break;
			}
			if(NULL == ipaddr)
			{
				printf("get ip error\n");
				vTaskDelete(NULL);
				return;
			}
		}
	}

	memset(&socketaddr, 0, sizeof(socketaddr));
	/* ��ֵserver��Ϣ */
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_addr.s_addr = inet_addr(ipaddr);
	socketaddr.sin_port = htons(SERVERPORT);
	socketaddr.sin_len = sizeof(socketaddr);

	/* �ͻ������ӷ������� */
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

	malloc(HTTP_RCV_LEN);
	/* �������ݵ�server�� */
	send(fd, GetStr, strlen(GetStr), 0);
	
	/* ����ͨ��ʵ����һ����ѭ���С�*/
	for( ;; )
	{
		do
		{
			ret = recv(fd ,Msgbuff, 50000, 0);
			if(ret > 0)
			{
				printf("Msgbuff data is %s\n",Msgbuff);
			}else
			{
				printf("httpclient no data\n");
				vTaskDelay(100);
			}
		}while(ret == -1);
		

}
	/* �������ľ���ʵ�ֻ������������ѭ���������������ں���������֮ǰɾ��������NULL������ʾɾ��
	���ǵ�ǰ���� */
	vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : HttpClient_init
 * Description  : HttpClient_init HTTP�ͻ��˳�ʼ��
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void HttpClient_init(void)
{
	xTaskCreate(ATaskHttpClient, "ATaskHttpClient", 256, NULL, 4, NULL);
}

