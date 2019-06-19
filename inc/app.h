#ifndef APP_H
#define APP_H

#define APP_LUGNIME_USER            4
#define APP_LUNGIME_TIMP            6
#define APP_COMANDA_PONTARE         (char)'P'
#define APP_COMANDA_DEPONTARE       (char)'D'
#define APP_COMANDA_SETARE_TIMP     (char)'T'
#define APP_COMANDA_CITIRE_TIMP     (char)'C'
#define APP_LUNGIME_DATE_EEPROM     APP_LUNGIME_TIMP
#define APP_DATE_NESCRISE           (unsigned char)0xFF
#define APP_MEMORIE_INITIALIZATA    0x01
#define APP_EROARE                  0x00
#define APP_LED_ROSU                PINB5
#define APP_COMANDA                 usart_buffer[0]

#define EEPROM_ADRESA_SLAVE         0x50

#define SERVO_POZITIE_0_GRADE       420
#define SERVO_POZITIE_90_GRADE      250
#define SERVO_PIN_CONTROL           PB1
void app_procesare_date(void);

#endif