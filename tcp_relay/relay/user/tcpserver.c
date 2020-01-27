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
	/* 可以像普通函数一样定义变量。用这个函数创建的每个任务实例都有一个属于自己的iVarialbleExample变
	量。但如果iVariableExample被定义为static，这一点则不成立 – 这种情况下只存在一个变量，所有的任务实
	例将会共享这个变量。 */
	int iVariableExample = 0;
	/* 定义一个状态变量 */
	STATION_STATUS Status;
	struct sockaddr_in socketaddr;
	struct sockaddr ClientAddr;
	socklen_t ClientAddrLen = sizeof(struct sockaddr);
	int fd = -1;//-1表示error
	int ret = -1;
	int cmd;
	int cfd = -1;
	unsigned char Msgbuff[48];//因为处理数据较多所以不用栈，用堆
	int overtime = 5000;
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
	printf("socket ok\r\n");
	/* 设置接收超时时间 */
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &overtime, sizeof(int));

	memset(&socketaddr, 0, sizeof(socketaddr));
	/* 赋值server信息 */
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

	/* 任务通常实现在一个死循环中。*/
	for( ;; )
	{
		/* 服务器接收客户端方向的连接,返回的是客户端的套接字 */
		cfd= accept(fd, &ClientAddr,&ClientAddrLen);
		if(-1 != cfd)
		{
			ret = recv(cfd,Msgbuff, 48, 0);
			if(ret > 0)
			{
				printf("accept ok\n");
				printf("tcpclient data is %s\n",Msgbuff);
				/* 有个bug,服务器是发relayon 服务器判断到的字符串是relayoff,虽然收到的字符串是relayon*/
				if(strncmp((char *)Msgbuff, "relayon", 7))
				{
					//Relay_Control(0);
					gpio16_output_set(0);//默认点亮
					printf("led on\n");
					/* 发送消息到server服务器 */
					send(cfd, "relayon", strlen("relayon"), 0);
				}
				else if(strncmp((char *)Msgbuff, "relayoff", strlen("relayoff")))
				{
					//Relay_Control(1);
					gpio16_output_set(1);
					printf("led off\n");
					/* 发送消息到server服务器 */
					send(cfd, "relayoff", strlen("relayoff"), 0);
				}
				else
					/* 发送消息到server服务器 */
					send(cmd, "cmd error", strlen("cmd error"), 0);
			}else
			{
				printf("tcpclient no data\n");
				vTaskDelay(100);
			}
		}	
		/* 如果不close会accept是-1 */
		close(cfd);
	}
	/* 如果任务的具体实现会跳出上面的死循环，则此任务必须在函数运行完之前删除。传入NULL参数表示删除
	的是当前任务 */
	vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : TcpServer_init
 * Description  : TcpServer_init TCP服务器初始化
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void TcpServer_init(void)
{
	xTaskCreate(ATaskTcpServer, "ATaskTcpServer", 256, NULL, 4, NULL);
}




