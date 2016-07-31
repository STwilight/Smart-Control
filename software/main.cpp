#include <fmx.h>
#pragma hdrstop
#include "main.h"

/* Подключение библиотеки HIDAPI для обращения к функциям HID-устройства */
#include "hidapi.h"

/* Подключение дополнительных библиотек */
#include <stdio.h>
#include <stdlib.h>

#pragma package(smart_init)
#pragma resource "*.fmx"
#pragma resource ("*.Windows.fmx", _PLAT_MSWINDOWS)
#pragma resource ("*.Surface.fmx", _PLAT_MSWINDOWS)

//-------------------------------- VARIABLES --------------------------------

/* Идентификаторы VID и PID */
#define VID 0x03EB
#define PID 0x204F

// Строка описания продукта
String productString = L"Smart Control Device";

// Указатель на структуру, описывающую HID-устройство
hid_device *handle_device;

// Размер выравнивания для структур равен 1 байт
#pragma pack (push, 1)

/* Описание структуры для передачи данных */
struct config_report
{
	// ID номер отчета, 1 байт
	unsigned char report_ID;
	// Состояние управляющих выводов, 2 байта
	unsigned short int light;
	// Уровень громкости, 1 байт
	unsigned char volume_level;
};

/** INFO:
 *    HIDAPI функции требуют наличия байта ReportID,
 *    даже если устройство его не использует.
 */

// Создание экземпляра структуры для передачи данных
struct config_report DeviceConfig;

// Возвращение предыдущей настройки выравнивания для структур
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
	/* Метод поиска и подключения необходимого HID-устройства */

	// Структура для поиска HID-устройств
	struct hid_device_info *devs, *cur_dev;

	// Установка отметки "Searching"
	Button3->ImageIndex = 0;
	// Вывод статуса
	Label2->Text = "Поиск устройства...";

	// Запуск поиска HID-устройства
	devs = hid_enumerate(VID, PID);

	// Помещение найденных устройств с указанными VID и PID в динамический массив
	cur_dev = devs;

	/* Поиск устройства по строке описания продукта 'Product Name' */
	while (cur_dev) {
		if ((cur_dev->product_string) == productString)
			break;
		cur_dev = cur_dev->next;
	}

	/* Если устройство найдено и успешно открыто... */
	if (cur_dev && (handle_device = hid_open_path(cur_dev->path)))
	{
		// Остановка таймера, который вызывает данный метод
		Timer1->Enabled = false;
		// Установка отметки "Connected"
		Button3->ImageIndex = 1;
		// Вывод статуса
		Label2->Text = "Устройство подключено";
	}
	else
	{
		// Установка отметки "Disconnected"
		Button3->ImageIndex = 0;
		//Button3->ImageIndex = 2;
		// Вывод статуса
		Label2->Text = "Поиск устройства...";
		//Label2->Text = "Устройство не найдено";
	}

	// Удаление списка
	hid_free_enumeration(devs);
}

void deviceSend(struct config_report Data)
{
	/* Метод отправки данных HID-устройству */

	if (hid_write(handle_device, (const unsigned char*)&Data, sizeof(Data))==-1)
	{
		// Установка отметки "Error"
		Form1->Button3->ImageIndex = 3;
		// Вывод статуса
		Form1->Label2->Text = "Ошибка отправки данных!";
	}
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

    // Остановка таймера, который вызывает метод подключения устройства
	if (Timer1->Enabled)
		Timer1->Enabled = false;
	else
		hid_close(handle_device);
	Application->Terminate();
}

void __fastcall TForm1::FormShow(TObject *Sender)
{
	/* Метод, вызываемый при отображении формы */

	Timer1->Enabled = true;
}

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    /* Метод, вызываемый при зыкрытии формы */

	if (Timer1->Enabled)
		Timer1->Enabled = false;
	else
		hid_close(handle_device);
}

//-------------------------------- DEBUG ------------------------------------

void __fastcall TForm1::testButton(TObject *Sender)
{
	/* DEBUG: Метод тестовой кнопки */

}

//-------------------------------- SAMPLES ----------------------------------

	/** Перечисление всех CheckBox в сообщении.
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
