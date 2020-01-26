#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "espressif/espconn.h"
#include "espressif/airkiss.h"
#include "usr_sntp.h"

const char *SntpServerNames[] = 
{
	"ntp1.aliyun.com",
	"ntp2.aliyun.com",
	"ntp3.aliyun.com",
	"ntp4.aliyun.com",
	"ntp5.aliyun.com",
	"ntp6.aliyun.com",
	"ntp7.aliyun.com"
};
/******************************************************************************
 * FunctionName : ATaskSntp
 * Description  : Sntp  
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ATaskSntp(void *pvParameters)
{
	u32_t time;
	STATION_STATUS status;
	/* �ж��Ƿ��ȡip��ַ */
	do
	{
		status = wifi_station_get_connect_status();
		vTaskDelay(100);
	}while(status != STATION_GOT_IP);


	printf("this is a ATaskSntp\n");
	printf("STATION_GOT_IP!\n");
	/* ����NTP������ */
	sntp_setservername(0, SntpServerNames[0]);
	sntp_setservername(1, SntpServerNames[1]);
	sntp_setservername(2, SntpServerNames[2]);
	/* SNTP��ʼ�� */
	sntp_init();
	for(;;)
	{
		/* ��ȡ��ǰʱ��� */
		time = sntp_get_current_timestamp();
		if(time)
		{
			printf("get current time is %s\n",sntp_get_real_time(time));
		}
		vTaskDelay(500);/* ��ʱ5s */
	}
	vTaskDelete(NULL);
}


/******************************************************************************
 * FunctionName : Sntp_init()
 * Description  : Sntp ���񴴽� 
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void Sntp_init(void)
{
	xTaskCreate(ATaskSntp, "ATaskSntp", 256, NULL, 4, NULL);
}

