
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

void nand_page(unsigned int page)
{
    volatile unsigned int i;
    NFADDR = ( page & 0xff);
    for(i = 0;i < 10;i++);

    NFADDR = ( (page >> 8) & 0xff);
    for(i = 0;i < 10;i++);

}

void nand_col(unsigned int col)
{
    
    volatile unsigned int i;
    NFADDR = ( col & 0xff);
    for(i = 0;i < 10;i++);
    
    NFADDR = ( (col>>8 & 0xff);
    for(i = 0;i < 10;i++);

    NFADDR = ( (col>>16) & 0xff);
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
/******************************************************************************
 * FunctionName : nand_w_data
 * Description  : д��ĳ�ֽ�����
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

void nand_w_data(unsigned char data)
{
    NFDATA = data;
}
/******************************************************************************
 * FunctionName : nand_write
 * Description  : ��������,���ĸ���ַ��ʼд�룬д��������Ҳ����ĳ���
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

void nand_write(unsigned int addr,unsigned char *buf,unsigned int len)
{
    
    unsigned int page = addr / 2048;
    unsigned int col  = addr % 2048;
    nand_select();
    while()
    {
        nand_cmd(0x80);
        nand_page(page);
        nand_col(col);
        
        for(; (col < 2048) &&(i<len);col++)
        {
            /* ��buff�������д�뵽nandflash��ȥ */
            nand_w_data(*buf[i++]);
        }
        /* ѭ������������д��һҳ������ */
        nand_cmd(0x10);
        wait_ready();

        /* �ж��Ƿ�д����ҳ */
        if(i == len)
            break;
        else
        {
            /* ������һҳд�� */
            page++;
            col = 0;
        }
    }
    
    nand_diselect();
}

/******************************************************************************
 * FunctionName : nand_erase
 * Description  : ��������,���ĸ���ַ��ʼ���������Ҳ����ĳ���
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

int nand_erase(unsigned int addr,unsigned int len)
{
    unsigned int i;
    unsigned int page = addr / 2048;
    unsigned int col  = addr % 2048;

     /* ��ַ��128k�� */
    if(!(addr & 0x1ffff))
    {
        printf("nandaddr error\r\n");
        return -1;
       
    }
    else if(!(len & 0x1ffff))
    {
        printf("nand len error\r\n");
        return -1;
    }
    nand_select();

    while(1)
    {
        nand_cmd(0x60);
        nand_page(page);
        nand_cmd(0xD0);
        
        wait_ready();

        /* һҳһҳ�ļ� */
        len -= 2048;

        if(len == 0)
            break;

        /* ��ַ����һҳ */
        addr += 128*1024;
    } 

    
    nand_diselect();
    return 0;
        
}
/******************************************************************************
 * FunctionName : nand_bad
 * Description  : ��һ���ֽڵĵ�ַ,�ж��ǲ��ǻ���
 * Parameters   : addr
 * Returns      : none
*******************************************************************************/

int nand_bad(unsigned char addr)
{
    unsigned int i;
    unsigned int page = addr / 2048;
    unsigned int col  = addr % 2048;

    unsigned char val;
    nand_select();
    nand_cmd(0);
    /* ����(ҳ)���е�ַ */
    nand_page(page);
    nand_col(col);
    
    nand_cmd(0x30);
    /* �ȴ�æ���� */
    wait_ready();
    /* ��һ���ֽڵ�ֵ */
    val = nand_data();

    nand_diselect();
    
    if(0xff != val)
        return 0;
    else
        return 1;
   
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
    unsigned int page = addr / 2048;
    unsigned int col  = addr % 2048;

      nand_select();
    /* ���û�ﵽ���� */
    while(i < len)
    {
        /* �жϵ�ַ�ǲ���Ϊһ���鲢��*/
        if( !(addr % 2048*64) && ( nand_bad(addr) ) )
        {
            /* ����ǣ���ַ�����˿� */
            addr += 2048*64;
            continue;
        }
  
        nand_cmd(0);
        /* ����(ҳ)���е�ַ */
        nand_page(page);
        nand_col(col);
        
        nand_cmd(0x30);
        /* �ȴ�æ���� */
        wait_ready();
        
        /* ѭ����ȡҳ���� */
        for(;(col < 2048)&&(i < len); col++)
        {
            buf[i++] = nand_data();
        }
        
        if(i == len)
            break;
        
        page++;
        col = 0;

         
    }
   
     nand_diselect(); 
  
}

void nand_chip_id(void)
{
    char buf[6];
    nand_select();
    nand_cmd(0x90);
    nand_addr_byte(0x00);

    buf[0] = nand_data();
    buf[1] = nand_data();
    buf[2] = nand_data();
    buf[3] = nand_data();
    buf[4] = nand_data();
    buf[5] = '\0';

    printf("Master code = %c\r\n",buf[0]);
    printf("Device code = %c\r\n",buf[1]);
    printf("page size is = %d\r\n",1 << (buf[3] & 0x3);
    printf("block size is = %d\r\n",1 << (buf[3] >> 4)& 0x3);
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
                nand_chip_id();
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


