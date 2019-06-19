#include <avr/io.h>
#include "iic.h"
#include "usart.h"
#include "timer.h"
#include "tipuri.h"

#define IIC_FREQ        iic_freq_400k
#define IIC_PRESCALER   1
#define IIC_FREQ_REG    ((((F_CPU / IIC_FREQ) / IIC_PRESCALER) - 16 ) / 2)

void iic_init(void)
{
    // Scrie valoarea calculata in registru
	TWBR = (uint8_t)IIC_FREQ_REG;
}

void iic_stop(void)
{
    // Trimite conditie de stop
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

void iic_eroare(int eroare)
{
    usart_tx_string_new_line("Eroare transmisie iic :");
    switch(eroare)
    {
        case(iic_eroare_start_condition):
        usart_tx_string_new_line("IIC: Start condition");
        break;
        case(iic_eroare_confirmare_slave):
        usart_tx_string_new_line("IIC:Confirmare slave");
        break;
        case(iic_eroare_scriere_date_slave):
        usart_tx_string_new_line("IIC: scriedre date slave");
        break;
        case(iic_eroare_citire_date_slave):
        usart_tx_string_new_line("IIC: citire date slave");
        break;
    }
    iic_stop();
}

void iic_asteapta(void)
{
    // Seteaza timerul sa astepte 100 ms pentru a nu ramane blocata functia
    timer_start(100);
	// Asteapta sa termine transmisia sau sa expire timpul de asteptare
	while (!(TWCR & (1<<TWINT)) && timer_citeste_status() != timer_timeout)
    {
        // Asteapta
    }
}

int iic_adresare_slave(unsigned char adresa_slave)
{
    unsigned char status;

	// Trimite start condition pentru iic
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    //iic_asteapta();
    while (!(TWCR & (1<<TWINT)));
    // Daca statusul e diferit de start condition
    if ((TWSR & 0xF8) != TW_START && (TWSR & 0xF8) != TW_REP_START)
    {
        // Semnaleaza eroarea
        iic_eroare(iic_eroare_start_condition);
        return iic_eroare_start_condition;
    }
    // Incarca adresa slave-ului
    TWDR = adresa_slave;
    // Reseteaza flagurile
    TWCR = (1<<TWINT) |(1<<TWEN);
	// Asteapta sa termine transmisia
    //iic_asteapta();
    while (!(TWCR & (1<<TWINT)));
    // Citeste statusul
    status = TW_STATUS & 0xF8;
    // Daca statusul este diferit de cel de transmisie master->slave address
    if (status != TW_MT_SLA_ACK && status != TW_MR_SLA_ACK)
    {
        // Semnaleaza eroarea
        iic_eroare(iic_eroare_confirmare_slave);
        return iic_eroare_confirmare_slave;
    }
    // Semnaleaza rulare finalizata cu succces
    return iic_ok;
}

int iic_scrie_byte(unsigned char byte)
{
    // Scrie datele de transmis in registru
    TWDR = byte;
    // Incepe transmisia
    TWCR = (1<<TWINT) | (1<<TWEN);
    // Asteapta sa se termine transmisia
    //iic_asteapta();
    while (!(TWCR & (1<<TWINT)));
    // Daca statusul e diferit de transmisie master->slave data
    if ((TWSR & 0xF8)!= TW_MT_DATA_ACK)
    {
        // Semnaleaza eroarea
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Semnaleaza rulare finalizata cu succces
    return iic_ok;
}

int iic_citeste_byte(unsigned char *byte)
{
    // Trimite conditie de incepere a citirii
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA); 
	// Asteapta sa se termine citirea
    //iic_asteapta();
    while (!(TWCR & (1<<TWINT)));
    // Daca statusul e diferit de transmisie master->slave data
    if ((TWSR & 0xF8)!= TW_MR_DATA_ACK)
    {
        //Semnaleaza eroarea
        iic_eroare(iic_eroare_citire_date_slave);
        return iic_eroare_citire_date_slave;
    }
	//Scrie datele in variabila
	*byte = TWDR;

    // Semnaleaza rulare finalizata cu succces
    return iic_ok;
}

int iic_citeste_ultimul_byte(unsigned char *byte)
{
    // Trimite conditie de incepere a citirii
    TWCR = (1<<TWINT) | (1<<TWEN);
	// Asteapta sa se termine citirea
    //iic_asteapta();
    while (!(TWCR & (1<<TWINT)));
    // Daca statusul e diferit de transmisie master->slave data
    if ((TWSR & 0xF8)!= TW_MR_DATA_NACK)
    {
        // Semnaleaza eroarea
        iic_eroare(iic_eroare_citire_date_slave);
        return iic_eroare_citire_date_slave;
    }
	//Scrie datele in variabila
	*byte = TWDR;

    // Semnaleaza rulare finalizata cu succces
    return iic_ok;
}

iic_status iic_scrie_date_registru(unsigned char adresa_slave, unsigned short registru, unsigned char *date, int lungime)
{
    volatile unsigned short aH;					//Address MSB byte
	volatile unsigned short aL;					//Address LSB byte
    int iic_status, index;

    aH = registru >> 8;
    aL = registru;
    // Se shifteaza adresa cu o pozitie si se pune bitul de operatie (scriere sau citire)
    iic_status = iic_adresare_slave(adresa_slave << 1 | TW_WRITE);
    // Verifica status
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_confirmare_slave);
        return iic_eroare_confirmare_slave;
    }
    // Scrie adresa registrului pe 2 byte
    // Scrie primul byte
    iic_status = iic_scrie_byte(aH);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Scrie urmatorul byte
    iic_status = iic_scrie_byte(aL);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Trimite in functie de lungime
    for(index = 0; index < lungime; index++)
    {
        // Scrie 1 byte de date
        iic_status = iic_scrie_byte(date[index]);
        // Verifica statusul
        if(iic_status != iic_ok)
        {
            // Semnaleaza eroare
            iic_eroare(iic_eroare_scriere_date_slave);
            return iic_eroare_scriere_date_slave;
        }
    }
    // Inchide transmisia
    iic_stop();

    // Semnaleaza rulare finalizata cu succces
    return iic_ok;
}

iic_status iic_citeste_date_registru(unsigned char adresa_slave, unsigned short registru, unsigned char *date, int lungime)
{
    volatile unsigned short aH;					//Address MSB byte
	volatile unsigned short aL;					//Address LSB byte
    int iic_status, index;

    aH = registru >> 8;
    aL = registru;

    // Se shifteaza adresa cu o pozitie si se pune bitul de operatie (scriere sau citire)
    iic_status = iic_adresare_slave(adresa_slave << 1 | TW_WRITE);
    // Verifica status
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_confirmare_slave);
        return iic_eroare_confirmare_slave;
    }
    // Scrie adresa registrului pe 2 byte
    // Scrie primul byte
    iic_status = iic_scrie_byte(aH);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Scrie urmatorul byte
    iic_status = iic_scrie_byte(aL);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Se shifteaza adresa cu o pozitie si se pune bitul de operatie (scriere sau citire)
    iic_status = iic_adresare_slave(adresa_slave << 1 | TW_READ);
    // Verifica status
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_confirmare_slave);
        return iic_eroare_confirmare_slave;
    }
    // Citeste in functie de lungime (ultimul byte trebuie citit diferit)
    for(index = 0; index < lungime - 1; index++)
    {
        // Citeste 1 byte de date
        iic_status = iic_citeste_byte(date + index);
        // Verifica statusul
        if(iic_status != iic_ok)
        {
            // Semnaleaza eroare
            iic_eroare(iic_eroare_citire_date_slave);
            return iic_eroare_citire_date_slave;
        }
    }
    // Citeste ultimul byte de date
    iic_status = iic_citeste_ultimul_byte(date + lungime - 1);
    // Inchide transmisia
    iic_stop();

    // Semnaleaza rulare finalizata cu succces
    return iic_ok;
}

iic_status iic_scrie_registru(unsigned char adresa_slave, unsigned char registru, unsigned char *date)
{
    int iic_status;
    // Se shifteaza adresa cu o pozitie si se pune bitul de operatie (scriere sau citire)
    iic_status = iic_adresare_slave(adresa_slave << 1 | TW_WRITE);
    // Verifica status
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_confirmare_slave);
        return iic_eroare_confirmare_slave;
    }
    // Scrie adresa registrului
    iic_status = iic_scrie_byte(registru);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Scrie 1 byte de date
    iic_status = iic_scrie_byte(*date);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Inchide transmisia
    iic_stop();

    // Semnaleaza rulare finalizata cu succces
    return iic_ok;
}

iic_status iic_citeste_registru(unsigned char adresa_slave, unsigned char registru, unsigned char *date)
{
    int iic_status;

    // Se shifteaza adresa cu o pozitie si se pune bitul de operatie (scriere sau citire)
    iic_status = iic_adresare_slave(adresa_slave << 1 | TW_WRITE);
    // Verifica status
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_confirmare_slave);
        return iic_eroare_confirmare_slave;
    }
    // Scrie adresa registrului 
    iic_status = iic_scrie_byte(registru);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Se shifteaza adresa cu o pozitie si se pune bitul de operatie (scriere sau citire)
    iic_status = iic_adresare_slave(adresa_slave << 1 | TW_READ);
    // Verifica status
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_confirmare_slave);
        return iic_eroare_confirmare_slave;
    }
    // Citeste ultimul byte de date
    iic_status = iic_citeste_ultimul_byte(date);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_citire_date_slave);
        return iic_eroare_citire_date_slave;
    }
    // Inchide transmisia
    iic_stop();

    // Semnaleaza rulare finalizata cu succces
    return iic_ok;
}


iic_status iic_sterge_registru(unsigned char adresa_slave, unsigned short registru, unsigned char caracter_stergere, int lungime)
{
    volatile unsigned short aH;					//Address MSB byte
	volatile unsigned short aL;					//Address LSB byte
    int iic_status, index;

    aH = registru >> 8;
    aL = registru;
    // Se shifteaza adresa cu o pozitie si se pune bitul de operatie (scriere sau citire)
    iic_status = iic_adresare_slave(adresa_slave << 1 | TW_WRITE);
    // Verifica status
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_confirmare_slave);
        return iic_eroare_confirmare_slave;
    }
    // Scrie adresa registrului pe 2 byte
    // Scrie primul byte
    iic_status = iic_scrie_byte(aH);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Scrie urmatorul byte
    iic_status = iic_scrie_byte(aL);
    // Verifica statusul
    if(iic_status != iic_ok)
    {
        // Semnaleaza eroare
        iic_eroare(iic_eroare_scriere_date_slave);
        return iic_eroare_scriere_date_slave;
    }
    // Trimite in functie de lungime
    for(index = 0; index < lungime; index++)
    {
        // Scrie caracterul de stergere
        iic_status = iic_scrie_byte(caracter_stergere);
        // Verifica statusul
        if(iic_status != iic_ok)
        {
            // Semnaleaza eroare
            iic_eroare(iic_eroare_scriere_date_slave);
            return iic_eroare_scriere_date_slave;
        }
    }
    // Inchide transmisia
    iic_stop();

    // Semnaleaza rulare finalizata cu succces
    return iic_ok;
}
