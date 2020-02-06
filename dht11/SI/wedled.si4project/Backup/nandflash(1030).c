
#include  "my_printf.h"
#include "s3c2440_soc.h"

#define TACLS (0<<12)
#define TWRPH0 (1<<8)
#define TWRPH1 (0<<4)

/* HCLK��100M,T = 10ns */
void nandflash_init(void)
{
    NFCONF = TACLS|TWRPH0|(TWRPH1);
    /*  NAND flash controller enable */
    /* ECC init,CS disable */
    NFCONT = (1<<0)|(1<<4)|(1<<1);
    
}

void nand_select(void)
{
    NFCONT &= ~(1<<1);
}

void nand_diselect(void)
{
    NFCONT |= (1<<1);
}

void nand_cmd(unsigned char cmd)
{
    volatile unsigned int i;
    NFCMD = cmd;
    for(i = 0;i < 10; i++);
}

void nand_addr(unsigned char addr)
{
    volatile unsigned int i;
    NFADDR = addr;
    for(i = 0;i < 10;i++);
}
unsigned char nand_data(void)
{
    return NFDATA;
}
/******************************************************************************
 * FunctionName : wait_ready
 * Description  : �ȴ�����
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void wait_ready(void)
{
    while(!(NFSTAT & 1));
}


void do_erase_nandflash()
{
    
}


/******************************************************************************
 * FunctionName : do_read_nandflash
 * Description  : ��nandҳ����
 * Parameters   : addr,buf,len(���ĸ���ַ����������೤����)
 * Returns      : none
*******************************************************************************/

void do_read_nandflash(unsigend int addr,unsigned char *buf ,unsigned int len)
{
    unsigned int i;
    unsigned int page = addr/2048;
    unsigned int col  = addr & 2047;
    nand_cmd(0);
    /* ����(ҳ)���е�ַ */
    nand_addr( page & 0xff);/* �Ͱ�λ */
    nand_addr( (page >> 8) & 0xff);/* �߰�λ */
    nand_addr( col & 0xff);/* �Ͱ�λ */
    nand_addr( (col >> 8) & 0xff);/* �а�λ */
    nand_addr( (col >> 16) & 0xff);/* �߰�λ */

    /* �ȴ�æ���� */
    wait_ready();
    /* ѭ����ȡҳ���� */
    for(;(i < 2048)&&(i <= len); i++)
    {
        buf[i++] = nand_data();
    }
    
    if(i > len)
        break;
        
  
}

void do_scan_nandflash(void)
{
    char buf[6];
    nand_select();
    nand_cmd(0x90);
    nand_addr(0x00);

    buf[0] = nand_data();
    buf[1] = nand_data();
    buf[2] = nand_data();
    buf[3] = nand_data();
    buf[4] = nand_data();
    buf[5] = '\0';

    printf("Master code = %c",buf[0]);
    printf("Device code = %c",buf[1]);
    nand_diselect();
}

void nandflash_test(void)
{
    char c;

    while(1)
    {
        
        /*��ӡ�˵���������ѡ��*/
        printf("[s],scan nandflash\r\n ");
        printf("[e],erase nandflash\r\n ");
        printf("[w],write nandflash\r\n");
        printf("[r],read nandflash\r\n");
        printf("[q],quit nandflash menu\r\n");
        
        printf("enter select:");
        
        c = getchar();
        
        printf("%c\r\n", c);
        
        switch (c)
        {
            case 's':
            case 'S':
                do_scan_nandflash();
                break;
            
            case 'e':
            case 'E':
                do_erase_nandflash();
                break;
            case 'w':
            case 'W':
                do_write_nandflash();
                break;
            
            case 'r':
            case 'R':
                do_read_nandflash();
                break;
            
            case 'q':
            case 'Q':
                do_quit_nandflash();
                break;
            default:
                break;
        }
    }
}


