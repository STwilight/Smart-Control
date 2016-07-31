#include <fmx.h>
#pragma hdrstop
#include "main.h"

/* ����������� ���������� HIDAPI ��� ��������� � �������� HID-���������� */
#include "hidapi.h"

/* ����������� �������������� ��������� */
#include <stdio.h>
#include <stdlib.h>

#pragma package(smart_init)
#pragma resource "*.fmx"
#pragma resource ("*.Windows.fmx", _PLAT_MSWINDOWS)
#pragma resource ("*.Surface.fmx", _PLAT_MSWINDOWS)

//-------------------------------- VARIABLES --------------------------------

/* �������������� VID � PID */
#define VID 0x03EB
#define PID 0x204F

// ������ �������� ��������
String productString = L"Smart Control Device";

// ��������� �� ���������, ����������� HID-����������
hid_device *handle_device;

// ������ ������������ ��� �������� ����� 1 ����
#pragma pack (push, 1)

/* �������� ��������� ��� �������� ������ */
struct config_report
{
	// ID ����� ������, 1 ����
	unsigned char report_ID;
	// ��������� ����������� �������, 2 �����
	unsigned short int light;
	// ������� ���������, 1 ����
	unsigned char volume_level;
};

/** INFO:
 *    HIDAPI ������� ������� ������� ����� ReportID,
 *    ���� ���� ���������� ��� �� ����������.
 */

// �������� ���������� ��������� ��� �������� ������
struct config_report DeviceConfig;

// ����������� ���������� ��������� ������������ ��� ��������
#pragma pack (pop)

//-------------------------------- SYSTEM -----------------------------------

TForm1 *Form1;

__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}

//-------------------------------- USER -------------------------------------

void __fastcall TForm1::deviceConnect(TObject *Sender)
{
	/* ����� ������ � ����������� ������������ HID-���������� */

	// ��������� ��� ������ HID-���������
	struct hid_device_info *devs, *cur_dev;

	// ��������� ������� "Searching"
	Button3->ImageIndex = 0;
	// ����� �������
	Label2->Text = "����� ����������...";

	// ������ ������ HID-����������
	devs = hid_enumerate(VID, PID);

	// ��������� ��������� ��������� � ���������� VID � PID � ������������ ������
	cur_dev = devs;

	/* ����� ���������� �� ������ �������� �������� 'Product Name' */
	while (cur_dev) {
		if ((cur_dev->product_string) == productString)
			break;
		cur_dev = cur_dev->next;
	}

	/* ���� ���������� ������� � ������� �������... */
	if (cur_dev && (handle_device = hid_open_path(cur_dev->path)))
	{
		// ��������� �������, ������� �������� ������ �����
		Timer1->Enabled = false;
		// ��������� ������� "Connected"
		Button3->ImageIndex = 1;
		// ����� �������
		Label2->Text = "���������� ����������";
	}
	else
	{
		// ��������� ������� "Disconnected"
		Button3->ImageIndex = 0;
		//Button3->ImageIndex = 2;
		// ����� �������
		Label2->Text = "����� ����������...";
		//Label2->Text = "���������� �� �������";
	}

	// �������� ������
	hid_free_enumeration(devs);
}

void deviceSend(struct config_report Data)
{
	/* ����� �������� ������ HID-���������� */

	if (hid_write(handle_device, (const unsigned char*)&Data, sizeof(Data))==-1)
	{
		// ��������� ������� "Error"
		Form1->Button3->ImageIndex = 3;
		// ����� �������
		Form1->Label2->Text = "������ �������� ������!";
	}
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

    // ��������� �������, ������� �������� ����� ����������� ����������
	if (Timer1->Enabled)
		Timer1->Enabled = false;
	else
		hid_close(handle_device);
	Application->Terminate();
}

void __fastcall TForm1::FormShow(TObject *Sender)
{
	/* �����, ���������� ��� ����������� ����� */

	Timer1->Enabled = true;
}

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    /* �����, ���������� ��� �������� ����� */

	if (Timer1->Enabled)
		Timer1->Enabled = false;
	else
		hid_close(handle_device);
}

//-------------------------------- DEBUG ------------------------------------

void __fastcall TForm1::testButton(TObject *Sender)
{
	/* DEBUG: ����� �������� ������ */

}

//-------------------------------- SAMPLES ----------------------------------

	/** ������������ ���� CheckBox � ���������.
	 *
	 *	AnsiString tmpstr;
	 *	for (int i = 1; i < GroupBox1->ControlsCount; i++) {
	 *		if (GroupBox1->Controls->Items[i]->ClassName() == "TCheckBox")
	 *			tmpstr += ((TCheckBox*)GroupBox1->Controls->Items[i])->Text + ", " +
	 *					  ((TCheckBox*)GroupBox1->Controls->Items[i])->Name + ", i=" + StrToInt(i) + "\n";
	 *	}
	 *	ShowMessage(tmpstr);
	 *
	 */
