//---------------------------------------------------------------------------

#ifndef frcalcH
#define frcalcH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include "..\scalc.h"
//---------------------------------------------------------------------------
class TCalcForm : public TForm
{
__published:	// IDE-managed Components
        TMainMenu *MainMenu;
        TMenuItem *MnCalc;
        TComboBox *CBStr;
        TMemo *MOutput;
        TMenuItem *Cstyle;
        TMenuItem *Casesensitive;
        TMenuItem *N1;
        TMenuItem *Exit1;
        TMenuItem *N2;
        TMenuItem *Variables1;
        TMenuItem *Escmin;
        TMenuItem *Forcedfloat;
        TMenuItem *Format;
        TMenuItem *Scientific;
        TMenuItem *ScientificPop;
        TMenuItem *Normalized;
        TMenuItem *NormalizedPop;
        TMenuItem *Integer;
        TMenuItem *IntegerPop;
        TMenuItem *Unsigned;
        TMenuItem *UnsignedPop;
        TMenuItem *Computing;
        TMenuItem *ComputingPop;
        TMenuItem *Hex;
        TMenuItem *HexPop;
        TMenuItem *Octal;
        TMenuItem *OctalPop;
        TMenuItem *Binary;
        TMenuItem *BinaryPop;
        TMenuItem *Datetime;
        TMenuItem *DatetimePop;
        TMenuItem *Degrees;
        TMenuItem *DegreesPop;
        TMenuItem *Char;
        TMenuItem *CharPop;
        TMenuItem *WChar;
        TMenuItem *WCharPop;
        TMenuItem *Unixtime;
        TMenuItem *UnixtimePop;
        TMenuItem *Fraction;
        TMenuItem *FractionPop;
        TMenuItem *Inch;
        TMenuItem *InchPop;
        TMenuItem *Temperature;
        TMenuItem *TemperaturePop;
        TMenuItem *String;
        TMenuItem *StringPop;
        TMenuItem *All;
        TMenuItem *AllPop;
        TMenuItem *Binarywidth;
        TMenuItem *N8;
        TMenuItem *N16;
        TMenuItem *N24;
        TMenuItem *N32;
        TMenuItem *N48;
        TMenuItem *N64;
        TMenuItem *binwide64;
        TMenuItem *Opacity;
        TMenuItem *ImpMul;
        TPopupMenu *PopupMenu;
        void __fastcall CBStrChange(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall CBStrKeyPress(TObject *Sender, char &Key);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall CstyleClick(TObject *Sender);
        void __fastcall CasesensitiveClick(TObject *Sender);
        void __fastcall Exit1Click(TObject *Sender);
        void __fastcall Variables1Click(TObject *Sender);
        void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall CBStrKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall CBStrSelect(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall EscminClick(TObject *Sender);
        void __fastcall ForcedfloatClick(TObject *Sender);
        void __fastcall ScientificClick(TObject *Sender);
        void __fastcall ComputingClick(TObject *Sender);
        void __fastcall IntegerClick(TObject *Sender);
        void __fastcall UnsignedClick(TObject *Sender);
        void __fastcall HexClick(TObject *Sender);
        void __fastcall OctalClick(TObject *Sender);
        void __fastcall BinaryClick(TObject *Sender);
        void __fastcall DatetimeClick(TObject *Sender);
        void __fastcall DegreesClick(TObject *Sender);
        void __fastcall AllClick(TObject *Sender);
        void __fastcall CharClick(TObject *Sender);
        void __fastcall WCharClick(TObject *Sender);
        void __fastcall StringClick(TObject *Sender);
        void __fastcall N8Click(TObject *Sender);
        void __fastcall N16Click(TObject *Sender);
        void __fastcall N24Click(TObject *Sender);
        void __fastcall N32Click(TObject *Sender);
        void __fastcall N48Click(TObject *Sender);
        void __fastcall N64Click(TObject *Sender);
        void __fastcall binwide64Click(TObject *Sender);
        void __fastcall NormalizedClick(TObject *Sender);
        void __fastcall OpacityClick(TObject *Sender);
        void __fastcall CBStrKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall UnixtimeClick(TObject *Sender);
        void __fastcall FractionClick(TObject *Sender);
        void __fastcall InchClick(TObject *Sender);
        void __fastcall TemperatureClick(TObject *Sender);
        void __fastcall ImpMulClick(TObject *Sender);
private:	// User declarations
        calculator *ccalc;
        char errMsg[256];
        //TStrings* SaveHist;
        long double fVal;
        long double imVal;
        void __fastcall SetOpt(bool forced = false);
        void __fastcall AddHist(void);
        void __fastcall Opt2Mnu(void);

public:		// User declarations
        //TStrings* Vars;
        int Options;
        int opacity;
        int binwide;

        __fastcall TCalcForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TCalcForm *CalcForm;
//---------------------------------------------------------------------------
#endif
