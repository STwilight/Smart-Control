
/* �������� ���� ������� */

#include "GenericHID.h"

/* ������� ������� */
int main(void)
{
	// ������������� ���������
	hardwareInit();
	
	// ���������� ���������� ����������
	sei();
	
	// �������� ����
	while(1)
	{
		HIDTask();
		USB_USBTask();
	}
}

/* ������� ��������� ��������� */
void hardwareInit(void)
{
	// ���������� WatchDog-�������, ���� �� ������� ����������� ������������ ��� FUSE-������
	MCUSR &= ~(1<<WDRF);
	wdt_disable();
	
	// ���������� �������� �������
	clock_prescale_set(clock_div_1);
	
	// ������������� USB
	USB_Init();
}

/* ������� ��������� ������� ����������� ���������� � ���� USB */
void deviceConnect(void)
{
	// TODO: ��������� ����������� ���������� �����������
}

/* ������� ��������� ������� ���������� ���������� �� ���� USB */
void deviceDisconnect(void)
{
	// TODO: ��������� ���������� ���������� �����������
}

/* ������� ��������� ������� ��������� ������������ USB */
void deviceConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	// ��������� �������� ����� ��� ������ ������� (HID-������)
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
}

/* ������� ��������� �������� �� ���� USB */
void deviceControlRequest(void)
{
	// ��������� ������������� �������� HID-������
	switch (USB_ControlRequest.bRequest)
	{
		// �������� ������ � ����������
		case HID_REQ_GetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				uint8_t GenericData[IN_REPORT_SIZE];
				createGenericHIDReport(GenericData);
				Endpoint_ClearSETUP();
				// �������� ������ (������) �� ���������� �����
				Endpoint_Write_Control_Stream_LE(&GenericData, sizeof(GenericData));
				Endpoint_ClearOUT();
			}
			break;
		// ��������� ������ �� �����
		case HID_REQ_SetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				uint8_t GenericData[OUT_REPORT_SIZE];
				Endpoint_ClearSETUP();
				/* ��������� ������ (������) �� ����� */
				Endpoint_Read_Control_Stream_LE(&GenericData, sizeof(GenericData));
				Endpoint_ClearIN();
				processGenericHIDReport(GenericData);
			}
			break;
	}
}

/* ������� ��������� ���������� ������, ����������� �� ����� */
void processGenericHIDReport(uint8_t* DataArray)
{
	// ������ 'DataArray' �������� ������ (�����), ���������� �� �����
	// TODO: ���������� ���������� �� ����� ������
}

/* ������� �������� ������ ��� �������� ����� */
void createGenericHIDReport(uint8_t* DataArray)
{
	// ������ 'DataArray' �������� ������ (�����) ��� �������� �����
	// TODO: ���������� ���������� �� ����� ������
}

/* �������, ����������� ���������� USB HID-���������� */
void HIDTask(void)
{
	// ���������� ������ ���� ���������� � ���������������� ��� ���������� ������� HID-����������
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;
	
	// ����� ������� ����� (�� �����)
	Endpoint_SelectEndpoint(GENERIC_OUT_EPADDR);
	// �������� ������� ������ �� �����
	if (Endpoint_IsOUTReceived())
	{
		// �������� ������� ������ � ���������� ������
		if (Endpoint_IsReadWriteAllowed())
		{
			// ����� ��� ������ ����������� ��������� ������
			uint8_t GenericData[OUT_REPORT_SIZE];
			// ������ ������ �� ������
			Endpoint_Read_Stream_LE(&GenericData, sizeof(GenericData), NULL);
			// ��������� ������ ������
			processGenericHIDReport(GenericData);
		}
		// �������� ������ ������
		Endpoint_ClearOUT();
	}

	// ����� �������� ����� (�� ����������)
	Endpoint_SelectEndpoint(GENERIC_IN_EPADDR);
	// ��������, ����� �� ���� �������� ������
	if (Endpoint_IsINReady())
	{
		// �������� ���������� ������ ��� �������� ������
		uint8_t GenericData[IN_REPORT_SIZE];
		// �������� ������
		createGenericHIDReport(GenericData);
		// ������ ������ ������ � �����
		Endpoint_Write_Stream_LE(&GenericData, sizeof(GenericData), NULL);
		// �������� ������ ��������
		Endpoint_ClearIN();
	}
}

