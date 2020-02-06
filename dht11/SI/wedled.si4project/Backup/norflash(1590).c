#include "string_utils.h"
#include  "my_printf.h"


#define Baseaddr 0 /* jz2440��0��ַ��Ӧnorflash */ 

/*
    ���ĸ���ַд��ĳ��ֵ
    0x55д��0x98
    (base+offset)<<1
       
*/
void do_word_norflash(unsigned int base,unsigned int offset,unsigned int value)
{
    /*д��ĳ����ַ�������ֽ�������short*/
    volatile unsigned short *p = (volatile unsigned short *)\
                    ((base+offset)<<1);
    *p = value;
}

/*
    ������뿴����ַ��Ҫ�ڷ�װһ������
*/
void nor_cmd(unsigned int offset,unisgned int val)
{
    do_word_norflash(Baseaddr, offset, val);
}

unsigned short do_read_norflash(unsigned int base,unsigned int offset)
{
    volatile unsigned short *p = (volatile unsigend short *)\
                    ((base+offset));
    return *p;
}

void nor_data(unsigned int offset)
{
    do_read_norflash(Baseaddr, offset);
}

void do_quit_norflash_test(void)
{

}

/*
    ������ϵĵ�ַ��
    �ʹ��ڴ�ӡ������������
    ���Ƿ����
*/
void do_read_norflash_test(void)
{
    unsigned int addr;
    int i,j;
    char str[16];
    volatile unsigned char *p;
    addr = get_uint();
    p = (volatile unsigned char *)addr;
    for(i = 0;i < 4; i++)
    {
        for(j = 0;j < 16; j++)
        {
            c = *p++;
            printf("%02x ",c);
        }

        for(j = 0;j < 16; j++)
        {
            /* �ж��ַ��ǲ��ǿɼ��ַ� */
            if( str[j] < 0x20 || str[j] > 0x7e )
            {
                putchar('.');
            }
            else
            {
                putchar(str[j]);
            }
          
        }
        printf("\r\n");
    }
}


void wait_ready(unsigned int addr)
{
    int i;
    unsigned int pre;
    unsigned int val;
    pre = nor_data(addr<<1);
    val = nor_data(addr<<1);
    while((pre & (1<<6)) != (val & (1<<6))
    {
        pre = val;
        val = nor_data(addr<<1);
    }
}

void do_write_norflash_test(void)
{
    int i,j;
    unsigned int val;
    unsigned int addr;
    char str[16];
    addr = get_uint();

    gets(str);

    /*
     *  str[i] str[j]<<8���16λ����
    */
    i = 0;
    j = 1;
    while(str[i]&&str[j])
    { 
        /* 16bit��д�� */
        val = str[i] + (str[j]<<8);

        /* ����*/
        
        nor_cmd(0x555, 0xAA);
        nor_cmd(0x2AA, 0x55);
        nor_cmd(0x555, 0xA0);
        nor_cmd(addr>>1, 0x30);

       /* �ȴ���д���,������Q6�ޱ仯 */
       wait_ready(addr);
       i += 2;
       j += 2;
       addr += 2;
    }

    /* ����ѭ����������� */
    /* str[i]!=0,str[j]=0
    *  str[i]=0,str[j]=0
    *  str[i]=0,str[j]!=0
    */
    val = str[i];
    /* ��д */
    nor_cmd(0x555, 0xAA);
    nor_cmd(0x2AA, 0x55);
    nor_cmd(0x555, 0xA0);
    nor_cmd(addr>>1, 0x30);
     wait_ready(addr);
}

void do_erase_norflash_test(void)
{
    
    unsigned int addr;
    addr = get_uint();

    nor_cmd(0x555, 0xAA);
    nor_cmd(0x2AA, 0x55);
    nor_cmd(0x555, 0x80);

    nor_cmd(0x555, 0xAA);
    nor_cmd(0x2AA, 0x55);
    nor_cmd(addr>>1, 0x30);
    wait_ready(addr);
}

/*
    ����Norflash��CFIģʽ
*/
void do_scan_norflash_test(void)
{
    char str[5];
    int size;
    int i,j,region;
    short block;
    int cnt = 0;
    short block_size;
    int region_info_base;
    int blockaddr;
    int ventor,device;

    /* ��ӡ�豸ID������ID  */
    nor_cmd(0x555, 0xAA);
    nor_cmd(0x2AA,0x55);
    nor_cmd(0x555,0x90);
    ventor = nor_data(0);
    device = nor_data(1);
    printf(" venter = 0x%x,device = 0x%x\r\n ",ventor,device);
    nor_cmd( 0, 0xf0 );/* ��λ */
    
    nor_cmd(0x55,0x98);
    /*����CFIģʽ����10��ַ���Ƿ��QRI*/
    str[0] = nor_data(0x10);
    str[1] = nor_data(0x11);
    str[2] = nor_data(0x12);
    str[3] = "\0";

    printf("str is %s\n",str);

    /*����*/
    size = 1<<(nor_data(0x27));
    printf("nor size = 0x%x,%dM\r\n",size,size/(1024*1024));

    /*
        ��ӡ����������ʼ��ַ

        Erase block region information:
        ǰ�����ֽ�+1 ����ʾ��region�ж��ٸ�block
        �������ֽ�*256
    */

    /* ��������ʼ��ַ */
    region = nor_data(0x2C);
    region_info_base = 0x2D;
    for(i = 0;i < region; i++)
    {
        block = 1+nor_data(region_info_base) + nor_data(region_info_base+1)<<8;

        block_size = 256*(nor_data(region_info_base+2) + nor_data(region_info_base+4)<<8);

        region_info_base += 4;
        for(j = 0;j < block; j++)
        {
            cnt++;
            printHex(blockaddr);
            putchar(' ');
            blockaddr += block_size;
            if(cnt > 5)
            {
                putchar('\r');
                putchar('\n');
            }
        }
    }

    /* �˳�CFIģʽ */
    nor_cmd(0, 0xf0);
}


void norflash_test(void)
{
    char c;
    /*��ӡ�˵���������ѡ��*/
    printf("[s],scan norflash\r\n ");
    printf("[e],erase norflash\r\n ");
    printf("[w],write norflash\r\n");
    printf("[r],read norflash\r\n");
    printf("[q],quit norflash menu\r\n");

    printf("enter select:");

    c = getchar();

    putchar(c);

    switch (c)
    {
        case 's':
        case 'S':
            do_scan_norflash_test();
            break;
        
        case 'e':
        case 'E':
            do_erase_norflash_test();
            break;
        case 'w':
        case 'W':
            do_write_norflash_test();
            break;
        
        case 'r':
        case 'R':
            do_read_norflash_test();
            break;
        
        case 'q':
        case 'Q':
            do_quit_norflash_test();
            break;
    }
}

