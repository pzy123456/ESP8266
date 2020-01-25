
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
	/* 可以像普通函数一样定义变量。用这个函数创建的每个任务实例都有一个属于自己的iVarialbleExample变
	量。但如果iVariableExample被定义为static，这一点则不成立 – 这种情况下只存在一个变量，所有的任务实
	例将会共享这个变量。 */
	int iVariableExample = 0;
	/* 定义一个状态变量 */
	STATION_STATUS Status;
	struct sockaddr_in socketaddr;
	int fd = -1;//-1表示error
	int ret = -1;
	unsigned char i;
	struct hostent* phostname;
	int overtime = 5000;
	char *ipaddr = NULL;
	do{

		Status = wifi_station_get_connect_status();
		vTaskDelay(100);/* 如果没有获取到ip就进行CPU释放占用权 */
	}while(Status != STATION_GOT_IP);
	/* 创建socket */
	fd = socket(PF_INET, SOCK_STREAM, 0);
	if(-1 == fd)
	{
		printf("socket error\n");
		vTaskDelete(NULL);
		return;
	}

	/* 设置接收超时时间 */
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &overtime, sizeof(int));

	/* 通过域名获取我地址 */
	phostname = gethostbyname("www.baidu.com");
	if(NULL == phostname)
	{
		printf("host fail\n");
		vTaskDelete(NULL);
		return;
	}
	else
	{
		/* 如果指针数组不为空,打印出分配的百度ip地址 */
		for(i=0;phostname->h_addr_list[i];i++)
		{
			/* 将网络字节序转换成点分格式ip */
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
	/* 赋值server信息 */
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_addr.s_addr = inet_addr(ipaddr);
	socketaddr.sin_port = htons(SERVERPORT);
	socketaddr.sin_len = sizeof(socketaddr);

	/* 客户端连接服务器端 */
	do
	{
		ret = connect(fd, (struct sockaddr *)&socketaddr,\
					(socklen_t)socketaddr.sin_len);
		if(-1 == ret)
		{
			printf("connect fail\n");
			vTaskDelay(100);
		}
	}while(ret != 0);/* 不成功一直阻塞连 */

	malloc(HTTP_RCV_LEN);
	/* 发送数据到server端 */
	send(fd, GetStr, strlen(GetStr), 0);
	
	/* 任务通常实现在一个死循环中。*/
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
	/* 如果任务的具体实现会跳出上面的死循环，则此任务必须在函数运行完之前删除。传入NULL参数表示删除
	的是当前任务 */
	vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : HttpClient_init
 * Description  : HttpClient_init HTTP客户端初始化
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void HttpClient_init(void)
{
	xTaskCreate(ATaskHttpClient, "ATaskHttpClient", 256, NULL, 4, NULL);
}

