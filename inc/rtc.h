#ifndef RTC_H
#define RTC_H

#define RTC_SLAVE_ADDRESS   0x68

#define RTC_CONTROL1        0x00
#define RTC_CONTROL2        0x01
#define RTC_CONTROL3        0x02
#define RTC_SECUNDE         0x03
#define RTC_MINUTE          0x04
#define RTC_ORA             0x06
#define RTC_ZI              0x06
#define RTC_LUNA            0x08
#define RTC_AN              0x09
#define RTC_STOP            0x20

struct t_timp
{
    char secunde;
    char minute;
    char ora;
    char zi;
    char luna;
    char an;
};

typedef struct t_timp timp;

void rtc_init();
void rtc_seteaza_timp(timp );
timp rtc_citeste_timp();
timp rtc_diferenta_timp(timp , timp );
void rtc_afiseaza_timp(timp );
#endif