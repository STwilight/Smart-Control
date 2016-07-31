//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Edit.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Memo.hpp>
#include <FMX.ScrollBox.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TStatusBar *StatusBar1;
	TButton *Button1;
	TGroupBox *GroupBox1;
	TCheckBox *CheckBox0;
	TCheckBox *CheckBox1;
	TCheckBox *CheckBox2;
	TCheckBox *CheckBox3;
	TCheckBox *CheckBox4;
	TCheckBox *CheckBox5;
	TCheckBox *CheckBox6;
	TCheckBox *CheckBox7;
	TCheckBox *CheckBox8;
	TCheckBox *CheckBox9;
	TCheckBox *CheckBox10;
	TCheckBox *CheckBox11;
	TCheckBox *CheckBox12;
	TCheckBox *CheckBox13;
	TCheckBox *CheckBox14;
	TCheckBox *CheckBox15;
	TGroupBox *GroupBox2;
	TLabel *Label1;
	TTrackBar *TrackBar1;
	TGroupBox *GroupBox3;
	TMemo *Memo1;
	TButton *Button2;
	TCheckBox *CheckBox16;
	TLabel *Label2;
	TButton *Button3;
	void __fastcall appExit(TObject *Sender);
	void __fastcall setVolume(TObject *Sender);
	void __fastcall testButton(TObject *Sender);
	void __fastcall setLight(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
