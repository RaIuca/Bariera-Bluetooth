#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "app.h"
#include "iic.h"
#include "rtc.h"
#include "usart.h"
#include "tipuri.h"
#include <avr/io.h>
#include <util/delay.h>

// Variabila externa definita in fisierul usart.c
extern char usart_buffer[USART_DIMENSIUNE_BUFFER];

static unsigned char string_timp[APP_LUNGIME_TIMP * 2];

unsigned short app_conversie_date(char *date, int lungime)
{
	int index = 0;
	unsigned short temp = 0;

	// Extrage un numar dat de cifre
	while(index <= lungime)
	{
		// Conversie din ascii in numar
		temp += date[index + 1] - '0';
		// Aranjeaza numarul
		temp *= 10;
		// Trece la urmatoarea cifra
		index++;
	}
	// Returneaza datele
	return temp;
}

unsigned char* app_conversie_timp(timp t)
{
	// Salveaza din structura in string
	string_timp[0] = t.secunde;
	string_timp[1] = t.minute;
	string_timp[2] = t.ora;
	string_timp[3] = t.zi;
	string_timp[4] = t.luna;
	string_timp[5] = t.an;

	return string_timp;
}

timp app_conversie_string(unsigned char *string)
{
	timp t;

	// Salveaza din string in structura
	t.secunde	= string[0]; 	
	t.minute	= string[1];
	t.ora		= string[2];
	t.zi		= string[3];
	t.luna		= string[4];
	t.an		= string[5];

	return t;
}

timp app_conversie_string_2(char *string)
{
	timp t;

	// Salveaza din string in structura
	t.secunde	= ((string[0]  - '0') * 10) + (string[1]  - '0');
	t.minute	= ((string[2]  - '0') * 10) + (string[3]  - '0');
	t.ora		= ((string[4]  - '0') * 10) + (string[5]  - '0');
	t.zi		= ((string[6]  - '0') * 10) + (string[7]  - '0');
	t.luna		= ((string[8]  - '0') * 10) + (string[9]  - '0');
	t.an		= ((string[10] - '0') * 10) + (string[11] - '0');

	return t;
}

void app_eroare(void)
{
	// Steaza pinul legat la led ca output
	DDRB |= (1 << APP_LED_ROSU);
	while(1)
	{
		// Porneste/opreste led-ul pentru a semnala eroarea
		PORTB ^= (1 << APP_LED_ROSU);
		_delay_ms(500);
	}
}

void app_bluetooth_init(void)
{
	// Trimite comenzile de configurare pentru modulul Bluetooth
	// Deconecteaza modulul
  	usart_tx_string("AT");
	// Seteaza modulul ca slave
  	usart_tx_string("AT+ROLE0");
	// Seteaza adresa MAC 00:15:87:12:E0:68
  	usart_tx_string("AT+LADDR00:15:87:12:E0:68");
	// Nu cere PIN
  	usart_tx_string("AT+TYPE0");
	// Seteaza UUID
  	usart_tx_string("AT+UUID0xFFE0");
	// Seteaza CHAR
  	usart_tx_string("AT+CHAR0xFFE1");
	// Porneste notificarile la conectare
  	usart_tx_string("AT+NOTI1");
	// Seteaza numele de broadcast
  	usart_tx_string("AT+NAMEbariera");
}

void app_servo_set(int pozitie)
{
	int index;
	if(pozitie == SERVO_POZITIE_90_GRADE)
	{
		// Schimbam pozitia barierei proresiv
		for(index = OCR1A; index >= pozitie; index--)
		{
			// Schimba perioada timerului
			OCR1A = index;
			// Asteapta
			_delay_ms(10);
		}
	}
	else if(pozitie == SERVO_POZITIE_0_GRADE)
	{
		// Schimbam pozitia barierei proresiv
		for(index = OCR1A; index <= pozitie; index++)
		{
			// Schimba perioada timerului
			OCR1A = index;
			// Asteapta
			_delay_ms(10);
		}
	}
}

void app_servo_init()
{
	// Seteaza pinul de contorl ca output
	DDRB |= (1 << SERVO_PIN_CONTROL);
	// Reseteaza counterul timerului
	TCNT1 = 0;		
	// Seteaza o valoare top pana unde numara
	ICR1 = 2499;		
	// Seteaza Fast PWM, TOP = ICR1, Clear OC1A on compare match, clk/64 
	TCCR1A = (1<<WGM11)|(1<<COM1A1);
	TCCR1B = (1<<WGM12)|(1<<WGM13)|(1<<CS10)|(1<<CS11);
	OCR1A = SERVO_POZITIE_0_GRADE;
}

int app_test_memorie()
{
	int index;
	unsigned char date_scris[4] = {'t','e','s','t'};
	unsigned char date_citit[4] = {'x','x','x','x'};
	// Scrie in eeprom datele referitoare la timp
	iic_scrie_date_registru(EEPROM_ADRESA_SLAVE, 999, date_scris, 4);
	// Delay intre scriere si citre
	_delay_ms(100);
	// Face o citire initiala pentru a se asigura ca persoana nu a fost pontata de intrare anterior
	iic_citeste_date_registru(EEPROM_ADRESA_SLAVE, 999, date_citit, 4);
	for(index = 0; index < 4; index++)
	{
		if(date_scris[index] != date_citit[index])
		{
			usart_tx_string_new_line("Eroare initializare memorie");
			return APP_EROARE;
		}
	}
	// Sterge datele din memorie
	iic_sterge_registru(EEPROM_ADRESA_SLAVE, 999, APP_DATE_NESCRISE, 4);
			
	return APP_MEMORIE_INITIALIZATA;
}

void app_procesare_date(void)
{
	timp			timp_curent, timp_intrare, diferenta_timp;
	int				index;
	unsigned short	eeprom_adresa_scriere;
	unsigned char*	string_timp_curent;
	unsigned char 	date_verificare[APP_LUNGIME_DATE_EEPROM];
	
	// Verifica ce comanda s-a primit
	switch(APP_COMANDA)
	{
		case(APP_COMANDA_PONTARE):
			// Face conversie din caractere in numar
			eeprom_adresa_scriere = app_conversie_date(usart_buffer, APP_LUGNIME_USER);
			// Aranjeaza adresa astfel incat sa nu se suprapuna datele si sa poata fi gasita usor
			eeprom_adresa_scriere *= APP_LUNGIME_DATE_EEPROM;
			// Face o citire initiala pentru a se asigura ca persoana nu a fost pontata de intrare anterior
			iic_citeste_date_registru(EEPROM_ADRESA_SLAVE, eeprom_adresa_scriere, date_verificare, APP_LUNGIME_DATE_EEPROM);
			// Verifica datele
			index = 0;
			while(index < APP_LUNGIME_DATE_EEPROM)
			{
				// Daca s-au gasit date scrise inseamna persoana s-a mai pontat de intrare anterior
				if(date_verificare[index] != APP_DATE_NESCRISE)
				{
					// Semnaleaza eroarea
        			// Trimite linie noua
        			usart_tx('\n');
        			// Trimite carriage return
        			usart_tx('\r');
					usart_tx_string("Userul [");
					usart_tx(usart_buffer[1]);
					usart_tx(usart_buffer[2]);
					usart_tx(usart_buffer[3]);
					usart_tx(usart_buffer[4]);
					usart_tx_string_new_line("] este deja pontat");
					// Iese din functie
					return;
				}
				index++;
			}
			// Citeste timpul curent
			timp_curent = rtc_citeste_timp();
			// Converteste timpul citit pentru a putea fi scris in eeprom
			string_timp_curent = app_conversie_timp(timp_curent);
			// Trimite linie noua
			usart_tx('\n');
			// Trimite carriage return
			usart_tx('\r');
			usart_tx_string("Userul [");
			usart_tx(usart_buffer[1]);
			usart_tx(usart_buffer[2]);
			usart_tx(usart_buffer[3]);
			usart_tx(usart_buffer[4]);
			usart_tx_string_new_line("] este pontat la data : ");
			// Iese din functie
			rtc_afiseaza_timp(timp_curent);
			// Scrie in eeprom datele referitoare la timp
			iic_scrie_date_registru(EEPROM_ADRESA_SLAVE, eeprom_adresa_scriere, string_timp_curent, APP_LUNGIME_DATE_EEPROM);
			// Ridica bariera
			app_servo_set(SERVO_POZITIE_90_GRADE);
			// Asteapta sa treaca
			_delay_ms(2000);
			// Inchide bariera
			app_servo_set(SERVO_POZITIE_0_GRADE);
		break;
		case(APP_COMANDA_DEPONTARE):
			// Face conversie din caractere in numar
			eeprom_adresa_scriere = app_conversie_date(usart_buffer, APP_LUGNIME_USER);
			// Aranjeaza adresa astfel incat sa nu se suprapuna datele si sa poata fi gasita usor
			eeprom_adresa_scriere *= APP_LUNGIME_DATE_EEPROM;
			// Face o citire initiala pentru a se asigura ca persoana nu a fost pontata de iesire anterior
			iic_citeste_date_registru(EEPROM_ADRESA_SLAVE, eeprom_adresa_scriere, date_verificare, APP_LUNGIME_DATE_EEPROM);
			// Verifica datele
			index = 0;
			while(index < APP_LUNGIME_DATE_EEPROM)
			{
				// Daca datele au fost sterse anterior
				if(date_verificare[index] == APP_DATE_NESCRISE)
				{
					// Semnaleaza eroarea
        			// Trimite linie noua
        			usart_tx('\n');
        			// Trimite carriage return
        			usart_tx('\r');
					usart_tx_string("Userul [");
					usart_tx(usart_buffer[1]);
					usart_tx(usart_buffer[2]);
					usart_tx(usart_buffer[3]);
					usart_tx(usart_buffer[4]);
					usart_tx_string_new_line("] este deja depontat");
					// Iese din functie
					return;
				}
				index++;
			}
			// Citeste timpul curent
			timp_curent = rtc_citeste_timp();
			// Afisare timp intrare			
			usart_tx_string("IN : ");
			rtc_afiseaza_timp(timp_intrare);
			// Afisare timp iesire			
			usart_tx_string("OUT : ");
			rtc_afiseaza_timp(timp_curent);
			// Salveaza timpul de intrare
			timp_intrare = app_conversie_string(date_verificare);
			// Calculeaza diferenta
			diferenta_timp = rtc_diferenta_timp(timp_intrare, timp_curent);
			// Trimite linie noua
			usart_tx('\n');
			// Trimite carriage return
			usart_tx('\r');
			usart_tx_string("Userul [");
			usart_tx(usart_buffer[1]);
			usart_tx(usart_buffer[2]);
			usart_tx(usart_buffer[3]);
			usart_tx(usart_buffer[4]);
			usart_tx_string_new_line("] are un pontaj de : ");
			// Afiseaza timpul stat
			rtc_afiseaza_timp(diferenta_timp);
			// Sterge datele din memorie
			iic_sterge_registru(EEPROM_ADRESA_SLAVE, eeprom_adresa_scriere, APP_DATE_NESCRISE, APP_LUNGIME_TIMP);
			// Ridica bariera
			app_servo_set(SERVO_POZITIE_90_GRADE);
			// Asteapta sa treaca
			_delay_ms(2000);
			// Inchide bariera
			app_servo_set(SERVO_POZITIE_0_GRADE);
		break;
		case(APP_COMANDA_SETARE_TIMP):
			// Trimite linie noua
			usart_tx('\n');
			// Trimite carriage return
			usart_tx('\r');
			usart_tx_string("Timpul a fost setat la : ");
			// Afiseaza timpul setat
			rtc_afiseaza_timp(app_conversie_string_2(usart_buffer + 1));
			// Seteaza timpul curent
			rtc_seteaza_timp(app_conversie_string_2(usart_buffer + 1));
		break;
	}
}

void app_initializare(void)
{
	// Initializeaza usart la 9600 baud
	usart_init();
	// Initializeaza iic la 400KHz
	iic_init();
	// Initializare rtc
	rtc_init();
	// Initializare modul bluetooth
	app_bluetooth_init();
	// Initializare servo motor
	app_servo_init();
	// Verifica daca s-a initializat memoria externa
	if(app_test_memorie() == APP_MEMORIE_INITIALIZATA)
	{
		// Trimite mesaj de confirmare pentru initializare
		usart_tx_string_new_line("Initializare finalizata");
	}
	else
	{		
		app_eroare();
	}
	// Porneste intreruperi
	sei();
}

int main()
{
	// Se initializeaza microcontrollerul
	app_initializare();
	// Mentine microcontrollerul in bucla infinita
	BUCLA_INFINITA
	return (0);
}
