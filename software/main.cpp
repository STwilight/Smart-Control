#include <fmx.h>
#pragma hdrstop
#include "main.h"

/* Подключение библиотеки для обращения к функциям HID-устройства */
#include "hidlibrary.h"

#pragma package(smart_init)
#pragma resource "*.fmx"
#pragma resource ("*.Windows.fmx", _PLAT_MSWINDOWS)
#pragma resource ("*.Surface.fmx", _PLAT_MSWINDOWS)

//-------------------------------- VARIABLES --------------------------------

/* Объявление переменных */
const char vendorName[]  = "ITProm";
const char productName[] = "Smart Control Device";

/** INFO:
 *	  Идентификаторы VID и PID задаются в идентификационной строке,
 *	  в файле hidlibrary.h, переменная 'const char idstring[]'.
 */

/* Размер выравнивания для структур равен 1 байт */
#pragma pack (push, 1)

/* Описание структуры для передачи данных */
struct config_t
{
	// состояние управляющих выводов, 2 байта
	unsigned short int light;
    // уровень громкости, 1 байт
	unsigned char volume_level;
};

/* Создание экземпляра структуры */
struct config_t	DeviceConfig;

/* Возвращение предыдущей настройки выравнивания для структур */
#pragma pack (pop)

/* Создание экземпляра класса */
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
    /* Метод поиска и подключения необходимого HID-устройства */

	// Флаг соответствия устройства
	int res = 0;

	/* Построение строки-идентификатора устройства */
	string exampleDeviceName = vendorName;
	exampleDeviceName += " ";
	exampleDeviceName += productName;

    /* Получение количества HID-устройств в системе */
	int n = hid.EnumerateHIDDevices();

	/* Поиск устройства в системе */
	for (int i = 0; i < n; i++)
	{
        /* Подключение к устройству */
		hid.Connect(i);

		/** Получение Vendor Name и Product Name устройства
		 *  посредством функции GetConnectedDeviceName(), возвращающей
		 *  строку вида 'Vendor_Name Product_Name'.
		 *  Если полученное значение совпадает с сохраненным в
		 *  переменной 'exampleDeviceName', то искомое устройство найдено:
		 *  возвращаем 'res' равный единице, иначе – 'res' равен нулю.
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
	/* Метод отправки данных HID-устройству */

	// Выполнение отправки данных устройству
	if (deviceConnect())
	{
		hid.SendData(&Data);
		Form1->Label2->Text = "Подключено";
	}
	else
		Form1->Label2->Text = "Отключено";
}

void __fastcall TForm1::setLight(TObject *Sender)
{
	/* Метод расчета и передачи значения состояния управляющих освещением выводов */

	if (Sender->ClassName() == "TCheckBox") {
		// Значение (2 байта) состояния управляющих выходов (0x0000 .. 0xFFFF)
		unsigned short int light = 0x0000;

        // Опрос состояния выключателей с целью расчета значения
		for (int i = 1; i < GroupBox1->ControlsCount; i++) {
			if (GroupBox1->Controls->Items[i]->ClassName() == "TCheckBox")
				if (((TCheckBox*)GroupBox1->Controls->Items[i])->IsChecked)
					light ^= (1 << i-1);
		}

        // Передача данных в структуру
		DeviceConfig.light = light;

		// Отправка данных устройству
		deviceSend(DeviceConfig);

        // DEBUG: Вывод отладочной информации
		Memo1->Lines->Strings[0] = ("Light: 0x" + IntToHex(light, 4));
	}
}

void __fastcall TForm1::setVolume(TObject *Sender)
{
	/* Метод расчета и передачи значения уровня громкости */

	// Положение ползунка громкости (0 .. 127)
	float level = TrackBar1->Value;

    // Значение (1 байт) уровня громкости (0x00 .. 0x7F)
	unsigned char volume_level = 0x00;

	// Расчет значения уровня для передачи устройству
	// DEBUG: возможность инвертирования значения
	if (CheckBox16->IsChecked) {
		volume_level = (unsigned char) (TrackBar1->Max - level);
	}
	else
		volume_level = (unsigned char) level;

	// Расчет и отображение уровня громкости в процентах
	Label1->Text = "Уровень: " + FloatToStrF(level*100/TrackBar1->Max, ffNumber, 3, 0) + "%";

	// Передача данных в структуру
	DeviceConfig.volume_level = volume_level;

	// Отправка данных устройству
	deviceSend(DeviceConfig);

	// DEBUG: Вывод отладочной информации
	Memo1->Lines->Strings[1] = ("Level: 0x" + IntToHex(volume_level, 2));
}

void __fastcall TForm1::appExit(TObject *Sender)
{
    /* Метод, вызываемый при нажатии кнопки выхода */

	Application->Terminate();
}

//-------------------------------- DEBUG ------------------------------------

void __fastcall TForm1::testButton(TObject *Sender)
{
    /* DEBUG: Метод тестовой кнопки */

	AnsiString tmpstr;
	for (int i = 1; i < GroupBox1->ControlsCount; i++) {
		if (GroupBox1->Controls->Items[i]->ClassName() == "TCheckBox")
			tmpstr += ((TCheckBox*)GroupBox1->Controls->Items[i])->Text + ", " +
					  ((TCheckBox*)GroupBox1->Controls->Items[i])->Name + ", i=" + StrToInt(i) + "\n";
	}
	ShowMessage(tmpstr);
}

