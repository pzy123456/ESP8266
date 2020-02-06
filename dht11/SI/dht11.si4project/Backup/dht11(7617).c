
#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "espressif/espconn.h"
#include "espressif/airkiss.h"
#include "dht11.h"
#include "gpio.h"


#define DHT11_PIN_NUM 5
#define DHT11_PIN_NUM GPIO_Pin_5

#define DHT11_SET    1
#define DHT11_RESET  0


void delay_ms(u16 ms)
{
    for(;ms>0;ms--)
		os_delay_us(1000);
}
/******************************************************************************
 * FunctionName : DHT11_Data_OUT
 * Description  : 做主机，数据脚输出
 * Parameters   : mask做高低电平
 * Returns      : void
*******************************************************************************/
static void DHT11_Data_OUT(u8 value)
{
    // GPIO5设为普通IO口
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);
    /* 设置输出 */
    //GPIO_AS_OUTPUT(GPIO_Pin_5);                            //设置IO12为输出模式
    // IO5设为输出=X
    GPIO_OUTPUT_SET(GPIO_ID_PIN(5), value);
}
/******************************************************************************
 * FunctionName : DHT11_Data_IN
 * Description  : 从机输出，数据脚做输入
 * Parameters   : void?
 * Returns      : void=
*******************************************************************************/
static void DHT11_Data_IN(void)
{
    // GPIO5设为IO口
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,	FUNC_GPIO5);
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(5));
    /* 读取状态D5 */
    //if(GPIO_INPUT_GET(GPIO_ID_PIN(5)))
		//return 1;
	//else
		//return 0;
}

/******************************************************************************
 * FunctionName : DHT11_Rst
 * Description  : 主机发出起始信号
 * Parameters   : void
 * Returns      : void
*******************************************************************************/

static void DHT11_Rst(void)
{
	
	DHT11_Data_OUT(0);
	delay_ms(20);
	DHT11_Data_OUT(1);
	os_delay_us(30);
}

/******************************************************************************
 * FunctionName : DHT11_Check
 * Description  : 主机检测DHT11的应答
 * Parameters   : void
 * Returns      : 应答: 1 非应答: 0
*******************************************************************************/
static u8 DHT11_Check(void)
{
	int retry = 0;
	DHT11_Data_IN();
	while(GPIO_INPUT_GET(GPIO_ID_PIN(5))&&retry<100)//延迟80us
	{
		retry++;
		os_delay_us(1);
	}
	if(retry >= 100)
		return 1;
	else
		retry = 0;
	while(!GPIO_INPUT_GET(GPIO_ID_PIN(5))&&retry<100)//延迟80us
	{
		retry++;
		os_delay_us(1);
	}
	if(retry >= 100)
		return 1;
	else
		return 0;
}

/******************************************************************************
 * FunctionName : DHT11_ReadBit
 * Description  : 读取一位
 * Parameters   : void
 * Returns      : void
*******************************************************************************/
static u8 DHT11_ReadBit(void)
{
	u8 retry = 0;
    DHT11_Data_IN();
	while(GPIO_INPUT_GET(GPIO_ID_PIN(5))&&retry < 150)
	{
		retry++;
		os_delay_us(1);
	}
	retry = 0;
	while(!GPIO_INPUT_GET(GPIO_ID_PIN(5))&&retry < 120)
	{
		retry++;
		os_delay_us(1);
	}
	os_delay_us(45);
	if(GPIO_INPUT_GET(GPIO_ID_PIN(5)))
		return 1;
	else
		return 0;
	
}

/******************************************************************************
 * FunctionName : DHT11_ReadByte
 * Description  : 读取一字节
 * Parameters   : void
 * Returns      : 一个byte值
*******************************************************************************/
static u8 DHT11_ReadByte(void)
{
	u8 i = 0;
	u8 data = 0;
	for(i = 0;i < 8;i++)
	{
		data <<= 1;
		data |= DHT11_ReadBit();
		
	}
	return data;
}

/******************************************************************************
 * FunctionName : DHT11_Read_Data
 * Description  : 读取温度和湿度
 * Parameters   : temp,humi
 * Returns      : void
*******************************************************************************/
void DHT11_Read_Data(u8 *temp,u8 *humi)
{
	u8 buf[5];
	u8 i;

	DHT11_Rst();
	/* 检测DHT11的应答 */
	if(DHT11_Check() == 0)
	{
		for(i = 0;i < 5;i++)
		{
			buf[i] = DHT11_ReadByte();
		}
		if(buf[4] == (buf[0]+buf[1]+buf[2]+buf[3]))
		{
			*temp = buf[0];
			*humi = buf[2];
		}
	}
	
}


/******************************************************************************
 * FunctionName : ATaskDht11
 * Description  : ATaskDht11 测试DHT11采集的温湿度
 * Parameters   : void pvParameters
 * Returns      : none
*******************************************************************************/
void ATaskDht11( void *pvParameters)
{
	u8 humi,temp;
	for(;;)
	{
    	taskENTER_CRITICAL();/* 进入临界段 */
		DHT11_Read_Data(&temp,&humi);
        taskEXIT_CRITICAL();/* 退出临界段 */
		printf("temp = %d,humi = %d\n",temp,humi);
		vTaskDelay(500);
	}
	vTaskDelete(NULL);
}

/******************************************************************************
 * FunctionName : Dht11_init
 * Description  : Dht11_init DHT11初始化
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void Dht11_init(void)
{
	xTaskCreate(ATaskDht11, "ATaskDht11", 256, NULL, 4, NULL);
}


