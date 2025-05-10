#include "uart_usb.h"
#include "ql_uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#define test_log(fmt, args...) printf(fmt, ##args)
void quec_usbcdc_callback(QL_UART_PORT_NUMBER_E port, void *para)
{
	int read_len = 0;
	char r_data[1024] = {0};
	read_len = ql_uart_read(port, r_data, sizeof(r_data));
	test_log("uart[%d] callback read data, data:%s, read_len:%d \r\n", port, r_data, read_len);
}
void uart_usb_init()
{
    int ret = -1;
    char r_data[128] = {0};
	int write_bytes = 0;
	ql_uart_config_t dcb;

    ql_uart_open(QL_USB_CDC_PORT, QL_UART_BAUD_115200, QL_FC_NONE);
    ql_uart_register_cb(QL_USB_CDC_PORT, quec_usbcdc_callback);	//use callback to read uart data
      
}
void USB_LOG(char *str,...)
{
	if (str!=NULL)
	{
		ql_uart_write(QL_USB_CDC_PORT, str, sizeof(str)); 
	}
}
void UART_Send(uint8_t UARTx, uint8_t* Str, uint16_t len){

    ql_uart_write(UARTx, Str, len);
}

void UART_Putc(uint8_t UARTx, uint8_t c){
    ql_uart_write(UARTx, &c, 1);
}



void UART_Puts(uint8_t UARTx, char* s)
{
	 while(*s != '\0')
    {		
		UART_Putc(UARTx, (uint16_t) (*s));
		s++;
    }	
}



void USARTItoa(uint8_t UARTx,long val, int radix, int len)
{
    uint8_t c, r, sgn = 0, pad = ' ';    
    uint8_t s[20], i = 0;    
    uint32_t v;

    if (radix < 0)
    {    
        radix = -radix;    
        if (val < 0) 
        {
    
            val = -val;    
            sgn = '-';    
        }    
    }    
    v = val;    
    r = radix;    
    if (len < 0) 
    {    
        len = -len;    
        pad = '0';    
    }    
    if (len > 20) return;    
    do 
    {    
        c = (uint8_t)(v % r);    
        if (c >= 10) c += 7;    
        c += '0';    
        s[i++] = c;    
        v /= r;    
    } while (v);    
    if (sgn) s[i++] = sgn;    
    while (i < len)    
        s[i++] = pad;    
    do    
        UART_Putc(UARTx,s[--i]);    
    while (i);

}


void UART_Printf(uint8_t UARTx, char* str, ...)
{

  	va_list arp;
    int d, r, w, s, l;
    va_start(arp, str);

	while ((d = *str++) != 0) 
        {
            if (d != '%')
            {    
                UART_Putc(UARTx, d); continue;    
            }
            d = *str++; w = r = s = l = 0;    
            if (d == '0') 
            {    
                d = *str++; s = 1;    
            }    
            while ((d >= '0')&&(d <= '9')) 
            {    
                w += w * 10 + (d - '0');    
                d = *str++;    
            }    
            if (s) w = -w;    
            if (d == 'l') 
            {    
                l = 1;    
                d = *str++;    
            }    
            if (!d) break;    
            if (d == 's') 
            {    
                UART_Puts(UARTx,va_arg(arp, char*));    
                continue;    
            }    
            if (d == 'c') 
            {    
                UART_Putc(UARTx,(char)va_arg(arp, int));    
                continue;    
            }    
            if (d == 'u') r = 10;    
            if (d == 'd') r = -10;    
            if (d == 'X' || d == 'x') r = 16; // 'x' added by mthomas in increase compatibility    
            if (d == 'b') r = 2;    
            if (!r) break;    
            if (l) 
            {    
                USARTItoa(UARTx,(long)va_arg(arp, long), r, w);    
            } 
            else 
            {    
                if (r > 0)        
                    USARTItoa(UARTx,(unsigned long)va_arg(arp, int), r, w);        
                else        
                    USARTItoa(UARTx,(long)va_arg(arp, int), r, w);
            }    
        }  
        va_end(arp);
	
}

