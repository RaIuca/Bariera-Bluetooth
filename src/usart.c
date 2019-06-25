#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "app.h"
#include "usart.h"
#include "timer.h"
#include "tipuri.h"

#define USART_CARACTER_INCEPUT  '$'
#define USART_CARACTER_SFARSIT  '#'
#define USART_BAUD              usart_baud_9600
#define BAUD_REGISTER           ( ( F_CPU / ( USART_BAUD * 8UL ) ) - 1 )

// Variabila statica folosita doar in acest fisier
static unsigned char    pozitie_buffer;
char                    usart_buffer[USART_DIMENSIUNE_BUFFER];

void usart_init(void)
{
    // Seteaza pinul de TX ca output
    //DDRD |= (1 << USART_PIN_TX);
    // Pune valoarea pe 16 biti calculata pentru baud rate
    // in 2 registri pe 8 biti
    UBRR0H = (BAUD_REGISTER >> 8);
    UBRR0L =  BAUD_REGISTER;
    UCSR0A |= ( 1 << U2X0 );
    // Porneste transmisie si receptie pe USART + intrerupere
    UCSR0B |= (1 << RXEN0 ) | (1 << TXEN0 ) | ( 1 << RXCIE0 );
    // Seteaza modul de transmisie :
    //      - asincron
    //      - fara biti de paritate
    //      - 8 biti de date
    //      - 1 bit de stop
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
}

void usart_tx(char data)
{
    // Asteapta sa se seteze flag-ul pentru empty buffer
	while ( !( UCSR0A & ( 1 << UDRE0 ) ) );
    {
        // Asteapta
    }
    // Pune data in bufferul de transmisie si trimite
    UDR0 = data;
}

char usart_rx(void)
{
    // Seteaza timerul sa astepte 100 ms pentru a nu ramane blocata functia
    timer_start(100);
    // Asteapta sa se seteze flag-ul pentru receptie de date
    while (!(UCSR0A & (1<<RXC0)) && timer_citeste_status() != timer_timeout)
    {
        // Asteapta
    }
    return (char)UDR0;
}

usart_status usart_tx_string_new_line(char *string)
{
    int pozitie = 0;
    // Verifica daca string-ul exista
    if(string != 0)
    {
        // Cat timp nu s-a ajuns la sfarsit de string
        while(string[pozitie] != SFARSIT_DE_STRING && (string[pozitie] != USART_CARACTER_SFARSIT))
        {
            // Trimite caracterul
            usart_tx(string[pozitie]);
            // Trece la urmatorul caracter
            pozitie++;
        }
        // Trimite linie noua
        usart_tx('\n');
        // Trimite carriage return
        usart_tx('\r');
        // Asteapta sa proceseze mesajul
        _delay_ms(100);
        // S-a finalizat transmisia si returneaza un cod 
        return usart_ok;
    }
    else
    {
        // Returneaza un cod de eroare
        return usart_eroare;
    }
}

usart_status usart_tx_string(char *string)
{
    int pozitie = 0;
    // Verifica daca string-ul exista
    if(string != 0)
    {
        // Cat timp nu s-a ajuns la sfarsit de string
        while(string[pozitie] != SFARSIT_DE_STRING && (string[pozitie] != USART_CARACTER_SFARSIT))
        {
            // Trimite caracterul
            usart_tx(string[pozitie]);
            // Trece la urmatorul caracter
            pozitie++;
        }
        // Asteapta sa proceseze mesajul
        _delay_ms(100);
        // S-a finalizat transmisia si returneaza un cod 
        return usart_ok;
    }
    else
    {
        // Returneaza un cod de eroare
        return usart_eroare;
    }
}

usart_status usart_rx_string(char *string)
{
    int     pozitie = 0;
    char    caracter_citit;

    do
    {
        // Citeste un caracter
        caracter_citit = usart_rx();
        // Daca primul caracter citit e sfarsit de linie
        if(caracter_citit == SFARSIT_DE_STRING &&  pozitie == 0)
        {
            // Returneaza un cod de eroare
            return usart_eroare;
        }
        // Scrie caracterul in string
        string[pozitie] = caracter_citit;
        // Incrementeaza pozitia de scriere
        pozitie++;
    // Cat timpi nu s-a ajuns la sfarsitul de string
    } while (caracter_citit != SFARSIT_DE_STRING);
    
    // S-a finalizat citirea si returneaza statusul
    return usart_ok;
}

void usart_golire_buffer()
{
    unsigned char index;

    for(index = 0; index < USART_DIMENSIUNE_BUFFER; index++)
    {
        usart_buffer[index] = 0x00;
    }
}

// Rutina intrerupere receptie usart
ISR (USART_RX_vect)
{
    unsigned char byte_receptionat;

    // Salveaza in variabila byte-ul receptionat
    byte_receptionat = UDR0;
    // Verifica daca e inceput de linie
    if(byte_receptionat == USART_CARACTER_INCEPUT)
    {
        // Initializeaza pozitia la care se va scrie in buffer
        pozitie_buffer = 0;
    }
    else
    {
        // Scrie caracterul in buffer
        usart_buffer[pozitie_buffer++] = byte_receptionat;
        // Verifica daca e sfarsut de linie
        if(byte_receptionat == USART_CARACTER_SFARSIT)
        {
            // Proceseaza datele
            app_procesare_date();
            usart_golire_buffer();
        }
    }
}
