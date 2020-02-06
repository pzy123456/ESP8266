
#include "s3c2440_soc.h"
#include "uart.h"


int main(void)
{
	char c;
	uart0_init();
	puts("hello world");
	while(1)
	{
		c = getchar();
		if('\r' == c)
		{
			putchar('\n');
		}
		else if('\n' == c)
		{
			putchar('\r');
		}
		putchar(c);

	}
}

