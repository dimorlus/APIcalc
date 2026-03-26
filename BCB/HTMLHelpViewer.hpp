// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'HTMLHelpViewer.pas' rev: 6.00

#ifndef HTMLHelpViewerHPP
#define HTMLHelpViewerHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Forms.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <Types.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Htmlhelpviewer
{
//-- type declarations -------------------------------------------------------
//-- var, const, procedure ---------------------------------------------------
extern "C" unsigned __stdcall HtmlHelp(unsigned hwndCaller, char * pszFile, unsigned uCommand, int dwData);
#pragma option push -w-inl
inline unsigned __stdcall HtmlHelp(unsigned hwndCaller, char * pszFile, unsigned uCommand, int dwData)
{
	return HtmlHelpA(hwndCaller, pszFile, uCommand, dwData);
}
#pragma option pop



}	/* namespace Htmlhelpviewer */
using namespace Htmlhelpviewer;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// HTMLHelpViewer
