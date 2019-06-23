#include "iic.h"
#include "rtc.h"
#include "usart.h"

static char string_conv[3];

char rtc_bcd_transf_decimal(char bcdByte)
{
    // Conversie din BCD in decimal
    return (((bcdByte & 0xF0) >> 4) * 10) + (bcdByte & 0x0F);
}
 
char rtc_decimal_transf_bcd(char decimalByte)
{
    // Conversie din decimal in BCD
    return (((decimalByte / 10) << 4) | (decimalByte % 10));
}

char* rtc_hex_transf_string(char hex)
{
    // Face conversie din numar in string de doua caractere + sfarsit de linie
    string_conv[0] = ((hex / 10) + '0');
    string_conv[1] = ((hex % 10) + '0');
    string_conv[2] = '\0';

    return string_conv;
}

char rtc_string_transf_hex(char* string)
{
    char temp;

    // Face conversie din string in numar 
    temp = string[0] - '0';
    temp *= 10;
    temp += string[1] - '0';

    return temp;
}

void rtc_stop()
{
    unsigned char date_citite = 0;

    // Citeste registul de control
    if(iic_citeste_registru(RTC_SLAVE_ADDRESS,RTC_CONTROL1,&date_citite) != iic_ok)
    {
        // Se semnaleaza erorile daca apar
        usart_tx_string_new_line("RTC: Eroare citire");
    }
    // Seteaza bitul de stop
    date_citite |= RTC_STOP;
    // Scrie datele
    if(iic_scrie_registru(RTC_SLAVE_ADDRESS,RTC_CONTROL1,&date_citite) != iic_ok)
    {
        // Se semnaleaza erorile daca apar
        usart_tx_string_new_line("RTC: Eroare scriere");
    }
}

void rtc_start()
{
    unsigned char date_citite = 0;

    // Citeste registul de control
    if(iic_citeste_registru(RTC_SLAVE_ADDRESS,RTC_CONTROL1,&date_citite) != iic_ok)
    {
        // Se semnaleaza erorile daca apar
        usart_tx_string_new_line("RTC: Eroare citire");
    }
    // Seteaza bitul de stop
    date_citite &= ~RTC_STOP;
    // Scrie datele
    if(iic_scrie_registru(RTC_SLAVE_ADDRESS,RTC_CONTROL1,&date_citite) != iic_ok)
    {
        // Se semnaleaza erorile daca apar
        usart_tx_string_new_line("RTC: Eroare scriere");
    }
}

void rtc_init()
{
    unsigned char reg_1 = 0x00;
    unsigned char reg_2 = 0x00;
    unsigned char reg_3 = 0x01;
    // Porneste RTC
    // Seteaza mod 24H
    // Opreste intreruperile
    // Opreste watchdog-ul
    // Mod direct battery switch
    if((iic_scrie_registru(RTC_SLAVE_ADDRESS,RTC_CONTROL1,&reg_1) != iic_ok) || 
       (iic_scrie_registru(RTC_SLAVE_ADDRESS,RTC_CONTROL2,&reg_2) != iic_ok) ||
       (iic_scrie_registru(RTC_SLAVE_ADDRESS,RTC_CONTROL3,&reg_3) != iic_ok))
    {
        // Se semnaleaza erorile daca apar
        usart_tx_string_new_line("RTC: Eroare initializare RTC");
    }
    // Porneste rtc-ul
    rtc_start();
}

char rtc_citeste(unsigned char parametru)
{
    unsigned char date_citite = 0, conversie_date;

    // Citeste datele cerute (minute, ora, zi, luna sau an)
    if(iic_citeste_registru(RTC_SLAVE_ADDRESS,parametru,&date_citite) != iic_ok)
    {
        // Se semnaleaza erorile daca apar
        usart_tx_string_new_line("RTC: Eroare citire");
    }
    // Face conversia datelor
    conversie_date = rtc_bcd_transf_decimal(date_citite);
    // Returneaza datele
    return conversie_date;
}

void rtc_scrie(int parametru, char date)
{
    unsigned char date_scris;

    // Face conversie din decimal in 
    date_scris = rtc_decimal_transf_bcd(date);
    // Scrie datele
    if(iic_scrie_registru(RTC_SLAVE_ADDRESS,parametru,&date_scris) != iic_ok)
    {
        // Se semnaleaza erorile daca apar
        usart_tx_string_new_line("RTC: Eroare citire");
    }
}

void rtc_seteaza_timp(timp timp_set)
{
    // Opreste ceasul
    rtc_stop();
    // Scrie datele convertite in bcd
    rtc_scrie(RTC_SECUNDE,timp_set.secunde);
    rtc_scrie(RTC_MINUTE,timp_set.minute);
    rtc_scrie(RTC_ORA,timp_set.ora);
    rtc_scrie(RTC_ZI,timp_set.zi);
    rtc_scrie(RTC_LUNA,timp_set.luna);
    rtc_scrie(RTC_AN,timp_set.an);
    // Porneste ceasul
    rtc_start();
}

timp rtc_citeste_timp()
{
    timp timp_citit;

    // Citeste datele si le salveaza in structura
    timp_citit.secunde = rtc_citeste(RTC_SECUNDE);
    timp_citit.minute  = rtc_citeste(RTC_MINUTE);
    timp_citit.ora     = rtc_citeste(RTC_ORA);
    timp_citit.zi      = rtc_citeste(RTC_ZI);
    timp_citit.luna    = rtc_citeste(RTC_LUNA);
    timp_citit.an      = rtc_citeste(RTC_AN);

    // Returnaeza datele
    return timp_citit;
}

timp rtc_diferenta_timp(timp t1, timp t2)
{
    timp diferenta;

    // Face progresiv diferenta de timp
    diferenta.an = t2.an - t1.an;
    if(t2.luna >= t1.luna)
    {
        diferenta.luna = t2.luna - t1.luna;
    }
    else
    {
        diferenta.an--;
        diferenta.luna = 12 - t1.luna + t2.luna;
    }

    if(t2.zi >= t1.zi)
    {
        diferenta.zi = t2.zi - t1.zi;
    }
    else
    {
        diferenta.luna--;
        diferenta.zi = 30 - t2.zi + t1.zi;
    }

    if(t2.ora >= t1.ora)
    {
        diferenta.ora = t2.ora - t1.ora;
    }
    else
    {
        diferenta.zi--;
        diferenta.ora = 24 - t2.ora + t1.ora;
    }
    
    if(t2.minute >= t1.minute)
    {
        diferenta.minute = t2.minute - t1.minute;
    }
    else
    {
        diferenta.ora--;
        diferenta.minute = 60 - t2.minute + t1.minute;
    }
    
    if(t2.secunde >= t1.secunde)
    {
        diferenta.secunde = t2.secunde + t1.secunde;
    }
    else
    {
        diferenta.minute--;
        diferenta.secunde =  60 - t2.secunde + t1.secunde;
    }
    
    // Returneaza diferenta de timp
    return diferenta;
}

void rtc_afiseaza_timp(timp t)
{
    // Trimite datele despartite de caracterul ':'
    usart_tx_string("An :");
    usart_tx_string(rtc_hex_transf_string(t.an));
    usart_tx('|');
    usart_tx_string("Luna :");
    usart_tx_string(rtc_hex_transf_string(t.luna));
    usart_tx('|');
    usart_tx_string("Zi :");
    usart_tx_string(rtc_hex_transf_string(t.zi));
    usart_tx('|');
    usart_tx_string("Ora :");
    usart_tx_string(rtc_hex_transf_string(t.ora));
    usart_tx('|');
    usart_tx_string("Minute :");
    usart_tx_string(rtc_hex_transf_string(t.minute));
    usart_tx('|');
    usart_tx_string("Secunde :");
    usart_tx_string(rtc_hex_transf_string(t.secunde));
    // Trimite linie noua
    usart_tx('\n');
    // Trimite carriage return
    usart_tx('\r');
}
