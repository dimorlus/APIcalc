//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ViewVars.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TVarsFrm *VarsFrm;
//---------------------------------------------------------------------------
__fastcall TVarsFrm::TVarsFrm(TComponent* Owner)
        : TForm(Owner)
{
 HelpFile = Application->HelpFile;
}                                                      
//---------------------------------------------------------------------------
void __fastcall TVarsFrm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == VK_ESCAPE)
  {
   Key = 0;
   Close();
  }                                        
}
//---------------------------------------------------------------------------


