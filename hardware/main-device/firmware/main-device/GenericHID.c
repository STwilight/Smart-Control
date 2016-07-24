
/* Основной файл проекта */

#include "GenericHID.h"

/* Главная функция */
int main(void)
{
	// Инициализация периферии
	hardwareInit();
	
	// Глобальное разрешение прерываний
	sei();
	
	// Основной цикл
	while(1)
	{
		HIDTask();
		USB_USBTask();
	}
}

/* Функция настройки периферии */
void hardwareInit(void)
{
	// Отключение WatchDog-таймера, если он включен посредством конфигурации или FUSE-битами
	MCUSR &= ~(1<<WDRF);
	wdt_disable();
	
	// Отключение делителя частоты
	clock_prescale_set(clock_div_1);
	
	// Инициализация USB
	USB_Init();
}

/* Функция обработки события подключения устройства к шине USB */
void deviceConnect(void)
{
	// TODO: индикация подключения устройства светодиодом
}

/* Функция обработки события отключения устройства от шины USB */
void deviceDisconnect(void)
{
	// TODO: индикация отключения устройства светодиодом
}

/* Функция обработки события изменения конфигурации USB */
void deviceConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	// Установка конечных точек для обмена данными (HID-отчеты)
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
}

/* Функция обработки запросов на шине USB */
void deviceControlRequest(void)
{
	// Обработка специфических запросов HID-класса
	switch (USB_ControlRequest.bRequest)
	{
		// Отправка данных с устройства
		case HID_REQ_GetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				uint8_t GenericData[IN_REPORT_SIZE];
				createGenericHIDReport(GenericData);
				Endpoint_ClearSETUP();
				// Передача данных (отчета) от устройства хосту
				Endpoint_Write_Control_Stream_LE(&GenericData, sizeof(GenericData));
				Endpoint_ClearOUT();
			}
			break;
		// Получение данных от хоста
		case HID_REQ_SetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				uint8_t GenericData[OUT_REPORT_SIZE];
				Endpoint_ClearSETUP();
				/* Получение данных (отчета) от хоста */
				Endpoint_Read_Control_Stream_LE(&GenericData, sizeof(GenericData));
				Endpoint_ClearIN();
				processGenericHIDReport(GenericData);
			}
			break;
	}
}

/* Функция обработки последнего отчета, полученного от хоста */
void processGenericHIDReport(uint8_t* DataArray)
{
	// Массив 'DataArray' содержит данные (отчет), полученные от хоста
	// TODO: обработчик полученных от хоста данных
}

/* Функция создания отчета для отправки хосту */
void createGenericHIDReport(uint8_t* DataArray)
{
	// Массив 'DataArray' содержит данные (отчет) для отправки хосту
	// TODO: обработчик полученных от хоста данных
}

/* Функция, реализующая функционал USB HID-устройства */
void HIDTask(void)
{
	// Устройство должно быть подключено и сконфигурировано для выполнения функций HID-устройства
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;
	
	// Выбор входной точки (от хоста)
	Endpoint_SelectEndpoint(GENERIC_OUT_EPADDR);
	// Проверка наличия пакета от хоста
	if (Endpoint_IsOUTReceived())
	{
		// Проверка наличия данных в полученном пакете
		if (Endpoint_IsReadWriteAllowed())
		{
			// Буфер для чтения содержимого входящего пакета
			uint8_t GenericData[OUT_REPORT_SIZE];
			// Чтение данных из отчета
			Endpoint_Read_Stream_LE(&GenericData, sizeof(GenericData), NULL);
			// Обработка данных отчета
			processGenericHIDReport(GenericData);
		}
		// Закрытие потока приема
		Endpoint_ClearOUT();
	}

	// Выбор выходной точки (от устройства)
	Endpoint_SelectEndpoint(GENERIC_IN_EPADDR);
	// Проверка, готов ли хост получать пакеты
	if (Endpoint_IsINReady())
	{
		// Создание временного буфера для хранения данных
		uint8_t GenericData[IN_REPORT_SIZE];
		// Создание отчета
		createGenericHIDReport(GenericData);
		// Запись данных отчета в буфер
		Endpoint_Write_Stream_LE(&GenericData, sizeof(GenericData), NULL);
		// Закрытие потока передачи
		Endpoint_ClearIN();
	}
}

