
#ifndef _UART_USB_H
#define _UART_USB_H
#include "ql_uart.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

    /*========================================================================
     *  Macro Definition
     *========================================================================*/
void uart_usb_init(void);
void quec_usbcdc_callback(QL_UART_PORT_NUMBER_E port, void *para);    
void USB_LOG(char *str,...);
void UART_Send(uint8_t UARTx, uint8_t* Str, uint16_t len);
void UART_Putc(uint8_t UARTx, uint8_t c);
void UART_Puts(uint8_t UARTx, char* s);
void USARTItoa(uint8_t UARTx,long val, int radix, int len);
void UART_Printf(uint8_t UARTx, char* str, ...);
#define usb_log(fmt, args...) UART_Printf(QL_USB_CDC_PORT,fmt, ##args)
#ifdef __cplusplus
} /*"C" */
#endif

#endif
