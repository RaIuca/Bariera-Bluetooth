#ifndef USART_H
#define USART_H

#define USART_DIMENSIUNE_BUFFER 128
#define USART_PIN_TX            PIND1 

typedef int usart_status;

enum e_usart_baud
{
    usart_baud_2400 = 2400,
    usart_baud_4800 = 4800,
    usart_baud_9600 = 9600,
    usart_baud_19200 = 19200,
    usart_baud_38400 = 38400,
    usart_baud_57600 = 57600,
    usart_baud_115200 = 115200,
    usart_baud_230400 = 230400
};

enum e_usart_status
{
    usart_ok = 1,
    usart_eroare
};

void            usart_init(void);
void            usart_tx(char);
char            usart_rx(void);
usart_status    usart_tx_string_new_line(char *);
usart_status    usart_tx_string(char *);
usart_status    usart_rx_string(char *);

#endif