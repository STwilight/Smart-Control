#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>   /* нужно для usbdrv.h */
#include "usbdrv.h"


struct dataexchange_t       // Описание структуры для передачи данных
{
   uchar b1;        // Я решил для примера написать структуру на 3 байта.
   uchar b2;        // На каждый байт подцепим ногу из PORTB. Конечно это
   uchar b3;        // не рационально (всего то 3 бита нужно).
};                  // Но в целях демонстрации в самый раз.
                    // Для наглядности прикрутить по светодиоду и созерцать :)


struct dataexchange_t pdata = {0, 0, 0};


PROGMEM char usbHidReportDescriptor[22] = { // USB report descriptor         // Дескриптор описывает структуру пакета данных для обмена
    0x06, 0x00, 0xff,                       // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                             // USAGE (Vendor Usage 1)
    0xa1, 0x01,                             // COLLECTION (Application)
    0x15, 0x00,                             //    LOGICAL_MINIMUM (0)        // min. значение для данных
    0x26, 0xff, 0x00,                       //    LOGICAL_MAXIMUM (255)      // max. значение для данных, 255 тут не случайно, а чтобы уложиться в 1 байт
    0x75, 0x08,                             //    REPORT_SIZE (8)            // информация передается порциями, это размер одного "репорта" 8 бит
    0x95, sizeof(struct dataexchange_t),    //    REPORT_COUNT               // количество порций (у нашем примере = 3, описанная выше структура передастся за три репорта)
    0x09, 0x00,                             //    USAGE (Undefined)
    0xb2, 0x02, 0x01,                       //    FEATURE (Data,Var,Abs,Buf)
    0xc0                                    // END_COLLECTION
};
/* Здесь мы описали только один report, из-за чего не нужно использовать report-ID (он должен быть первым байтом).
 * С его помощью передадим 3 байта данных (размер одного REPORT_SIZE = 8 бит = 1 байт, их количество REPORT_COUNT = 3).
 */


/* Эти переменные хранят статус текущей передачи */
static uchar    currentAddress;
static uchar    bytesRemaining;


/* usbFunctionRead() вызывается когда хост запрашивает порцию данных от устройства
 * Для дополнительной информации см. документацию в usbdrv.h
 */
uchar   usbFunctionRead(uchar *data, uchar len)
{
    if(len > bytesRemaining)
        len = bytesRemaining;

    uchar *buffer = (uchar*)&pdata;

    if(!currentAddress)        // Ни один кусок данных еще не прочитан.
    {                          // Заполним структуру для передачи
        if ( PINB & _BV(1) )
            pdata.b1 = 1;
        else
            pdata.b1 = 0;


        if ( PINB & _BV(2) )
            pdata.b2 = 1;
        else
            pdata.b2 = 0;


        if ( PINB & _BV(3) )
            pdata.b3 = 1;
        else
            pdata.b3 = 0;
    }

    uchar j;
    for(j=0; j<len; j++)
        data[j] = buffer[j+currentAddress];

    currentAddress += len;
    bytesRemaining -= len;
    return len;
}


/* usbFunctionWrite() вызывается когда хост отправляет порцию данных к устройству
 * Для дополнительной информации см. документацию в usbdrv.h
 */
uchar   usbFunctionWrite(uchar *data, uchar len)
{
    if(bytesRemaining == 0)
        return 1;               /* конец передачи */

    if(len > bytesRemaining)
        len = bytesRemaining;

    uchar *buffer = (uchar*)&pdata;
    
    uchar j;
    for(j=0; j<len; j++)
        buffer[j+currentAddress] = data[j];

    currentAddress += len;
    bytesRemaining -= len;

    if(bytesRemaining == 0)     // Все данные получены
    {                           // Выставим значения на PORTB
        if ( pdata.b1 )
            PORTB |= _BV(1);
        else
            PORTB &= ~_BV(1);


        if ( pdata.b2 )
            PORTB |= _BV(2);
        else
            PORTB &= ~_BV(2);


        if ( pdata.b3 )
            PORTB |= _BV(3);
        else
            PORTB &= ~_BV(3);
    }

    return bytesRemaining == 0; /* 0 означает, что есть еще данные */
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* HID устройство */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            // у нас только одна разновидность репорта, можем игнорировать report-ID
            bytesRemaining = sizeof(struct dataexchange_t);
            currentAddress = 0;
            return USB_NO_MSG;  // используем usbFunctionRead() для отправки данных хосту
        }else if(rq->bRequest == USBRQ_HID_SET_REPORT){
            // у нас только одна разновидность репорта, можем игнорировать report-ID
            bytesRemaining = sizeof(struct dataexchange_t);
            currentAddress = 0;
            return USB_NO_MSG;  // используем usbFunctionWrite() для получения данных от хоста
        }
    }else{
        /* остальные запросы мы просто игнорируем */
    }
    return 0;
}
/* ------------------------------------------------------------------------- */

int main(void)
{
    DDRB = 0b00001110;      // PB1,PB2,PB3 - выход

    usbInit();
    usbDeviceDisconnect();  // принудительно отключаемся от хоста, так делать можно только при выключенных прерываниях!
    
    uchar i = 0;
    while(--i){             // пауза > 250 ms
        _delay_ms(1);
    }
    
    usbDeviceConnect();     // подключаемся

    sei();                  // разрешаем прерывания

    for(;;){                // главный цикл программы
        usbPoll();          // эту функцию надо регулярно вызывать с главного цикла, максимальная задержка между вызовами - 50 ms
    }
    return 0;
}
/* ------------------------------------------------------------------------- */ 
