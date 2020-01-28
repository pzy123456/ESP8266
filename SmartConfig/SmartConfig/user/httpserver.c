
#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "httpserver.h"
#include <string.h>
  
#define SERVERIP    "192.168.0.191"
#define SERVERPORT  80
/*��ʾ���ĸ�ʽgbk-2312*/
const char *page = 
"<html>"
"<head>"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gbk-2312\" />"
"<title>��ӭ����html����</title>"
"</head>"
"<body>"
"<P>�����һ����Ȥ��ʵ��</P>"	
"</body>"
"</html>";

/******************************************************************************
 * FunctionName : file_ok
 * Description  : file_ok ����״̬ͷ��״̬��
 * Parameters   : �ͻ����׽��֣��ͷ��ͳ���
 * Returns      : none
*******************************************************************************/
void file_ok(int cfd,int len)
{
	char *SendBuff = (char *)malloc(sizeof(char)*100);
	sprintf(SendBuff,"HTTP/1.1 200 OK\r\n");
	send(cfd, SendBuff, strlen(SendBuff), 0);
	
	sprintf(SendBuff,"Connection: keep-alive\r\n");
	send(cfd, SendBuff, strlen(SendBuff), 0);

	sprintf(SendBuff,"Content-Length: %d\r\n",len);
	send(cfd, SendBuff, strlen(SendBuff), 0);

	sprintf(SendBuff,"Content-Type: text/html; charset=gbk-2312\r\n");
	send(cfd, SendBuff, strlen(SendBuff), 0);

	sprintf(SendBuff,"\r\n");
	send(cfd, SendBuff, strlen(SendBuff), 0);
	
}

/******************************************************************************
 * FunctionName : ATaskHttpServer
 * Description  : ATaskHttpServer
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ATaskHttpServer( void *pvParameters )
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
	int cfd = -1;
	//unsigned char Msgbuff[48];���������������
	char *HttpMsg = NULL;
	int overtime = 2000;
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
	socketaddr.sin_addr.s_addr = INADDR_ANY;
	socketaddr.sin_port = htons(SERVERPORT);
	socketaddr.sin_len = sizeof(socketaddr);

	if(bind(fd, (struct sockaddr *)&socketaddr, (socklen_t) socketaddr.sin_len) != 0)
	{
		printf("bind error\n");
		vTaskDelete(NULL);
		return;
	}
	if(listen(fd, 10) != 0)
	{
		printf("listen error\n");
		vTaskDelete(NULL);
		return;
	}

	/* ����һ�����ջ����������������� */
	HttpMsg = (char *)malloc(sizeof(char)*1000);
	/* ����ͨ��ʵ����һ����ѭ���� */
	for( ;; )
	{
		/* ���������տͻ��˷��������,���ص��ǿͻ��˵��׽��� */
		cfd= accept(fd, &ClientAddr,&ClientAddrLen);
		if(-1 != cfd)
		{
			ret = recv(cfd,HttpMsg, 1000, 0);
			if(ret > 0)
			{
				printf("httpclient data is %s\n",HttpMsg);

				/*����״̬ͷ��״̬��*/
				file_ok(cfd, strlen(page));
				/* ������Ϣ��server������ */
				send(cfd, page, strlen(page),0);
				
			}else
			{
				printf("httpclient no data\n");
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
 * FunctionName : HttpServer_init
 * Description  : HttpServer_init HTTP��������ʼ��
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void HttpServer_init(void)
{
	xTaskCreate(ATaskHttpServer, "ATaskHttpServer", 256, NULL, 4, NULL);
}

