//---------------------------------------------------------------------------

#ifndef ViewVarsH
#define ViewVarsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TVarsFrm : public TForm
{
__published:	// IDE-managed Components
        TMemo *VarsMemo;
        void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// User declarations
public:		// User declarations
        __fastcall TVarsFrm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TVarsFrm *VarsFrm;
//---------------------------------------------------------------------------
#endif
