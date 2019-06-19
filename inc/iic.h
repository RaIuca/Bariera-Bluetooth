#ifndef IIC_H
#define IIC_H

#define TW_START		            0x08
#define TW_REP_START		        0x10
#define TW_MT_SLA_ACK		        0x18
#define TW_MT_SLA_NACK		        0x20
#define TW_MT_DATA_ACK		        0x28
#define TW_MT_DATA_NACK		        0x30
#define TW_MT_ARB_LOST		        0x38
#define TW_MR_ARB_LOST		        0x38
#define TW_MR_SLA_ACK		        0x40
#define TW_MR_SLA_NACK		        0x48
#define TW_MR_DATA_ACK		        0x50
#define TW_MR_DATA_NACK		        0x58
#define TW_ST_SLA_ACK		        0xA8
#define TW_ST_ARB_LOST_SLA_ACK	    0xB0
#define TW_ST_DATA_ACK		        0xB8
#define TW_ST_DATA_NACK		        0xC0
#define TW_ST_LAST_DATA		        0xC8
#define TW_SR_SLA_ACK		        0x60
#define TW_SR_ARB_LOST_SLA_ACK	    0x68
#define TW_SR_GCALL_ACK		        0x70
#define TW_SR_ARB_LOST_GCALL_ACK    0x78
#define TW_SR_DATA_ACK		        0x80
#define TW_SR_DATA_NACK		        0x88
#define TW_SR_GCALL_DATA_ACK	    0x90
#define TW_SR_GCALL_DATA_NACK	    0x98
#define TW_SR_STOP		            0xA0
#define TW_NO_INFO		            0xF8
#define TW_BUS_ERROR		        0x00
#define TW_STATUS_MASK		        (_BV(TWS7)|_BV(TWS6)|_BV(TWS5)|_BV(TWS4)|_BV(TWS3))
#define TW_STATUS		            (TWSR & TW_STATUS_MASK)
#define TW_READ		                1
#define TW_WRITE	                0

typedef int iic_status;

enum e_iic_freq
{
    iic_freq_100k = 100000UL,
    iic_freq_200k = 200000UL,
    iic_freq_400k = 400000UL,
    iic_freq_500k = 500000UL,
    iic_freq_1M   = 1000000UL
};

enum e_iic_status
{
    iic_ok = 1,
    iic_eroare_start_condition,
    iic_eroare_confirmare_slave,
    iic_eroare_scriere_date_slave,
    iic_eroare_citire_date_slave,
    
};

void        iic_init(void);
iic_status  iic_scrie_date_registru(unsigned char , unsigned short , unsigned char *, int );
iic_status  iic_citeste_date_registru(unsigned char , unsigned short , unsigned char *, int );
iic_status  iic_scrie_registru(unsigned char , unsigned char , unsigned char* );
iic_status  iic_citeste_registru(unsigned char , unsigned char , unsigned char* );
iic_status  iic_sterge_registru(unsigned char , unsigned short , unsigned char, int );

#endif