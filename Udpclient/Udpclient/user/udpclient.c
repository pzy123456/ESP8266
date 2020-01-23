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
/* 可以像普通函数一样定义变量。用这个函数创建的每个任务实例都有一个属于自己的iVarialbleExample变
量。但如果iVariableExample被定义为static，这一点则不成立 – 这种情况下只存在一个变量，所有的任务实
例将会共享这个变量。 */
int iVariableExample = 0;
/* 定义一个状态变量 */
STATION_STATUS Status;
struct sockaddr_in socketaddr;
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
socketaddr.sin_addr.s_addr = inet_addr(SERVERIP);
socketaddr.sin_port = htons(SERVERPORT);
socketaddr.sin_len = sizeof(socketaddr);
/* 任务通常实现在一个死循环中。*/
for( ;; )
{
	/* 发送消息到server服务器 */
	sendto(fd, "I am udpclient", sizeof("I am udpclient"),\
     0,(struct sockaddr *)&socketaddr, (socklen_t)socketaddr.sin_len);
	printf("发完数据\n");
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
	/* 如果任务的具体实现会跳出上面的死循环，则此任务必须在函数运行完之前删除。传入NULL参数表示删除
	的是当前任务 */
	vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : UdpClient_init
 * Description  : UdpClient_init UDP客户端初始化
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void UdpClient_init(void)
{
	xTaskCreate(ATaskUdpClient, "ATaskUdpClient", 256, NULL, 4, NULL);
}

