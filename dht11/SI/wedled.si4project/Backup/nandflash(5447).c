
#include  "my_printf.h"
#include "s3c2440_soc.h"

#define TACLS (0<<12)
#define TWRPH0 (1<<8)
#define TWRPH1 (0<<4)

/* HCLK是100M,T = 10ns */
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
 * Description  : 等待就绪
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void wait_ready(void)
{
    while(!(NFSTAT & 1));
}
/******************************************************************************
 * FunctionName : nand_w_data
 * Description  : 写入某字节数据
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

void nand_w_data(unsigned char data)
{
    NFDATA = data;
}
/******************************************************************************
 * FunctionName : nand_write
 * Description  : 擦除扇区,从哪个地址开始写入，写到哪里，并且擦除的长度
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
            /* 把buff里的数据写入到nandflash里去 */
            nand_w_data(*buf[i++]);
        }
        /* 循环跳出条件是写完一页数据了 */
        nand_cmd(0x10);
        wait_ready();

        /* 判断是否写完整页 */
        if(i == len)
            break;
        else
        {
            /* 继续下一页写入 */
            page++;
            col = 0;
        }
    }
    
    nand_diselect();
}

/******************************************************************************
 * FunctionName : nand_erase
 * Description  : 擦除扇区,从哪个地址开始擦除，并且擦除的长度
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

int nand_erase(unsigned int addr,unsigned int len)
{
    unsigned int i;
    unsigned int page = addr / 2048;
    unsigned int col  = addr % 2048;

     /* 地址是128k的 */
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

        /* 一页一页的减 */
        len -= 2048;

        if(len == 0)
            break;

        /* 地址递增一页 */
        addr += 128*1024;
    } 

    
    nand_diselect();
    return 0;
        
}
/******************************************************************************
 * FunctionName : nand_bad
 * Description  : 读一个字节的地址,判断是不是坏块
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
    /* 发出(页)，列地址 */
    nand_page(page);
    nand_col(col);
    
    nand_cmd(0x30);
    /* 等待忙结束 */
    wait_ready();
    /* 读一个字节的值 */
    val = nand_data();

    nand_diselect();
    
    if(0xff != val)
        return 0;
    else
        return 1;
   
}

/******************************************************************************
 * FunctionName : do_read_nandflash
 * Description  : 读nand页数据
 * Parameters   : addr,buf,len(从哪个地址读到哪里读多长数据)
 * Returns      : none
*******************************************************************************/

void do_read_nandflash(unsigend int addr,unsigned char *buf ,unsigned int len)
{
    unsigned int i;
    unsigned int page = addr / 2048;
    unsigned int col  = addr % 2048;

      nand_select();
    /* 如果没达到长度 */
    while(i < len)
    {
        /* 判断地址是不是为一个块并且*/
        if( !(addr % 2048*64) && ( nand_bad(addr) ) )
        {
            /* 如果是，地址跳过此块 */
            addr += 2048*64;
            continue;
        }
  
        nand_cmd(0);
        /* 发出(页)，列地址 */
        nand_page(page);
        nand_col(col);
        
        nand_cmd(0x30);
        /* 等待忙结束 */
        wait_ready();
        
        /* 循环读取页数据 */
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
        
        /*打印菜单，供我们选择*/
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


