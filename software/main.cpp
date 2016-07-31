#include <fmx.h>
#pragma hdrstop
#include "main.h"

/* ����������� ���������� ��� ��������� � �������� HID-���������� */
#include "hidlibrary.h"

#pragma package(smart_init)
#pragma resource "*.fmx"
#pragma resource ("*.Windows.fmx", _PLAT_MSWINDOWS)
#pragma resource ("*.Surface.fmx", _PLAT_MSWINDOWS)

//-------------------------------- VARIABLES --------------------------------

/* ���������� ���������� */
const char vendorName[]  = "ITProm";
const char productName[] = "Smart Control Device";

/** INFO:
 *	  �������������� VID � PID �������� � ����������������� ������,
 *	  � ����� hidlibrary.h, ���������� 'const char idstring[]'.
 */

/* ������ ������������ ��� �������� ����� 1 ���� */
#pragma pack (push, 1)

/* �������� ��������� ��� �������� ������ */
struct config_t
{
	// ��������� ����������� �������, 2 �����
	unsigned short int light;
    // ������� ���������, 1 ����
	unsigned char volume_level;
};

/* �������� ���������� ��������� */
struct config_t	DeviceConfig;

/* ����������� ���������� ��������� ������������ ��� �������� */
#pragma pack (pop)

/* �������� ���������� ������ */
HIDLibrary <config_t> hid;

//-------------------------------- SYSTEM -----------------------------------

TForm1 *Form1;

__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}

//-------------------------------- USER -------------------------------------

int deviceConnect(void)
{
    /* ����� ������ � ����������� ������������ HID-���������� */

	// ���� ������������ ����������
	int res = 0;

	/* ���������� ������-�������������� ���������� */
	string exampleDeviceName = vendorName;
	exampleDeviceName += " ";
	exampleDeviceName += productName;

    /* ��������� ���������� HID-��������� � ������� */
	int n = hid.EnumerateHIDDevices();

	/* ����� ���������� � ������� */
	for (int i = 0; i < n; i++)
	{
        /* ����������� � ���������� */
		hid.Connect(i);

		/** ��������� Vendor Name � Product Name ����������
		 *  ����������� ������� GetConnectedDeviceName(), ������������
		 *  ������ ���� 'Vendor_Name Product_Name'.
		 *  ���� ���������� �������� ��������� � ����������� �
		 *  ���������� 'exampleDeviceName', �� ������� ���������� �������:
		 *  ���������� 'res' ������ �������, ����� � 'res' ����� ����.
		 */
		if (hid.GetConnectedDeviceName() == exampleDeviceName)
		{
			res = 1;
			break;
		}
	}
	return res;
}

void deviceSend(struct config_t Data)
{
	/* ����� �������� ������ HID-���������� */

	// ���������� �������� ������ ����������
	if (deviceConnect())
	{
		hid.SendData(&Data);
		Form1->Label2->Text = "����������";
	}
	else
		Form1->Label2->Text = "���������";
}

void __fastcall TForm1::setLight(TObject *Sender)
{
	/* ����� ������� � �������� �������� ��������� ����������� ���������� ������� */

	if (Sender->ClassName() == "TCheckBox") {
		// �������� (2 �����) ��������� ����������� ������� (0x0000 .. 0xFFFF)
		unsigned short int light = 0x0000;

        // ����� ��������� ������������ � ����� ������� ��������
		for (int i = 1; i < GroupBox1->ControlsCount; i++) {
			if (GroupBox1->Controls->Items[i]->ClassName() == "TCheckBox")
				if (((TCheckBox*)GroupBox1->Controls->Items[i])->IsChecked)
					light ^= (1 << i-1);
		}

        // �������� ������ � ���������
		DeviceConfig.light = light;

		// �������� ������ ����������
		deviceSend(DeviceConfig);

        // DEBUG: ����� ���������� ����������
		Memo1->Lines->Strings[0] = ("Light: 0x" + IntToHex(light, 4));
	}
}

void __fastcall TForm1::setVolume(TObject *Sender)
{
	/* ����� ������� � �������� �������� ������ ��������� */

	// ��������� �������� ��������� (0 .. 127)
	float level = TrackBar1->Value;

    // �������� (1 ����) ������ ��������� (0x00 .. 0x7F)
	unsigned char volume_level = 0x00;

	// ������ �������� ������ ��� �������� ����������
	// DEBUG: ����������� �������������� ��������
	if (CheckBox16->IsChecked) {
		volume_level = (unsigned char) (TrackBar1->Max - level);
	}
	else
		volume_level = (unsigned char) level;

	// ������ � ����������� ������ ��������� � ���������
	Label1->Text = "�������: " + FloatToStrF(level*100/TrackBar1->Max, ffNumber, 3, 0) + "%";

	// �������� ������ � ���������
	DeviceConfig.volume_level = volume_level;

	// �������� ������ ����������
	deviceSend(DeviceConfig);

	// DEBUG: ����� ���������� ����������
	Memo1->Lines->Strings[1] = ("Level: 0x" + IntToHex(volume_level, 2));
}

void __fastcall TForm1::appExit(TObject *Sender)
{
    /* �����, ���������� ��� ������� ������ ������ */

	Application->Terminate();
}

//-------------------------------- DEBUG ------------------------------------

void __fastcall TForm1::testButton(TObject *Sender)
{
    /* DEBUG: ����� �������� ������ */

	AnsiString tmpstr;
	for (int i = 1; i < GroupBox1->ControlsCount; i++) {
		if (GroupBox1->Controls->Items[i]->ClassName() == "TCheckBox")
			tmpstr += ((TCheckBox*)GroupBox1->Controls->Items[i])->Text + ", " +
					  ((TCheckBox*)GroupBox1->Controls->Items[i])->Name + ", i=" + StrToInt(i) + "\n";
	}
	ShowMessage(tmpstr);
}

