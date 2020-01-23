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
	/* 可以像普通函数一样定义变量。用这个函数创建的每个任务实例都有一个属于自己的iVarialbleExample变
	量。但如果iVariableExample被定义为static，这一点则不成立 – 这种情况下只存在一个变量，所有的任务实
	例将会共享这个变量。 */
	int iVariableExample = 0;
	/* 定义一个状态变量 */
	STATION_STATUS Status;
	struct sockaddr_in socketaddr;
	struct sockaddr from;/* 客户端信息 */
	socklen_t fromlen = sizeof(struct sockaddr);
	int fd = -1;//-1表示error
	int ret = -1;
	unsigned char Msgbuff[48];
	int overtime = 5000;
	do{

		Status = wifi_station_get_connect_status();
		vTaskDelay(100);/* 如果没有获取到ip就进行CPU释放占用权 */
	}while(Status != STATION_GOT_IP);
	/* 创建socket */
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(-1 == fd)
	{
		printf("socket error\n");
		vTaskDelete(NULL);
		return;
	}

	/* 设置接收超时时间 */
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &overtime, sizeof(int));

	memset(&socketaddr, 0, sizeof(socketaddr));
	/* 赋值server信息 */
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_addr.s_addr = INADDR_ANY;//服务器端接收任何地址
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
	/* 任务通常实现在一个死循环中。*/
	for( ;; )
	{
		do
		{
			/* 接收客户端的连接 */
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

		/* 发送消息到client客户端 */
		sendto(fd, "I am udpserver", sizeof("I am udpserver"),\
	     0,&from, fromlen);
		printf("发完数据\n");

	}
		/* 如果任务的具体实现会跳出上面的死循环，则此任务必须在函数运行完之前删除。传入NULL参数表示删除
		的是当前任务 */
		vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : UdpServer_init
 * Description  : UdpServer_init UDP服务器初始化
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void UdpServer_init(void)
{
	xTaskCreate(ATaskUdpServer, "ATaskUdpServer", 256, NULL, 4, NULL);
}


