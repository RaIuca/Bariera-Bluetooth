#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

#define TIMER_CONTOR_1MS 63535

// Variabile statice folosite doar in acest fisier
static int          contor_ms;
static int          limita_contor;
static timer_status status;

// Frecventa clock = 16 000 000 Hz
// Cu prescaler 8 rezulta frecventa timer = 2 000 000 Hz
// Perioada numarare timer = 0.5 us
// Pentru 1 ms avem nevoie de 2 000 de perioade de numarare
// Pentru a numara 2 000 de perioade trebuie sa punem contorul 
//  la 65 535 - 2 000 = 63 525

void timer_scrie_contor(unsigned int contor)
{
    unsigned char   sreg_salvat;
    // Salveaza flag-ul global de intrerupere
    sreg_salvat = SREG;
    // Opreste intreruperile
    cli();
    // Scrie contorul
    TCNT1 = contor;
    // Rescrie flag-ul global de intrerupere
    SREG = sreg_salvat;
}

void timer_init(void)
{
    // Opreste intreruperile
    cli();
    // Reseteaza contorul
    timer_scrie_contor(TIMER_CONTOR_1MS);
    // Initializeaza contor secunde
    contor_ms = 0;
    // Setare mod normal timer (numarator)
    TCCR1A = 0x00;
    // Prescaler de 8
    TCCR1B = (1<<CS11);
    // Porneste intreruperea pentru overflow
    TIMSK1 |= (1 << TOIE1);
    // Initializeaza statusul
    status = timer_idle;
    // Porneste intreruperile globale
    sei();
}

void timer_start(int timp_ms)
{
    // Initializeaza timerul
    timer_init();
    // Reseteaza contorul de milisecunde
    contor_ms = 0;
    // Seteaza limita
    limita_contor = timp_ms;
    // Setaza statusul
    status = timer_numara;
}

void timer_stop(void)
{
    // Opreste intreruperile timerului
    TIMSK1 = 0;
    // Setaza statusul
    status = timer_idle;
}

timer_status timer_citeste_status(void)
{
    // Returneaza statusul curent al timerului
    return status;
}

// Rutina intrerupere overflow pentru timer
ISR (TIMER0_OVF_vect)
{
    // Verifica daca s-a trecut de limita de tim
    if(contor_ms >= limita_contor)
    {
        // Daca s-a trecut de limita seteaza statusul
        status = timer_timeout;
        // Opreste timerul
        timer_stop();
    }
    else
    {
        // Daca nu s-a trecut de limita continua numaratoarea
        contor_ms++;
    }
    // Sterge flag-ul de intrerupere
    TIFR1 &= ~(1 >> TOV1);
}