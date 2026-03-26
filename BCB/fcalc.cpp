//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("fcalc.cpp", Form1);
USEFORM("ViewVars.cpp", VarsFrm);
USEFORM("frcalc.cpp", CalcForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Scientific string calculator";
                 Application->HelpFile = "fcalc.chm";
                 Application->CreateForm(__classid(TCalcForm), &CalcForm);
                 Application->CreateForm(__classid(TVarsFrm), &VarsFrm);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
        return 0;
}
//---------------------------------------------------------------------------
