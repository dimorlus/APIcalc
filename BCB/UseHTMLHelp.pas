{***************************************************************}
{                                                               }
{       HTML Help Unit for Delphi 3, 4 and 5                    }
{                                                               }
{       Copyright (c) 2001  Jan Goyvaerts                       }
{                                                               }
{       Design & implementation, by Jan Goyvaerts, 2001         }
{                                                               }
{***************************************************************}

{
  Not suitable for Delphi 6 or later, since Delphi 6 introduces
  a new way of calling help.  The new system is designed to handle
  different kinds of help systems to make Delphi suitable for
  cross-platform development.

  You may use this unit free of charge in all your Delphi applications.
  Distribution of this unit or units derived from it is prohibited.
  If other people would like a copy, they are welcome to download it from:
  http://www.helpscribble.com/delphi-bcb.html

  Add this unit to the uses clause of any unit in your application,
  and any help requests will automatically be translated into the
  proper HTML Help calls.

  You will need to specify the .chm file that contains your HTML Help
  in the help file setting in Delphi's project options.
  Otherwise, nothing will happen when the user presses F1.

  The unit does its work by assigning its own event handler to the
  Application.OnHelp event.  This means you must take care not to
  assign your own handler to Application.OnHelp, as this will
  defeat the purpose of this unit.

  Also, this simple unit will ignore any form's HelpFile property.
  This property is new in Delphi 4
  
  If you're looking for a tool to easily create HLP and/or CHM files,
  take a look at HelpScribble at http://www.helpscribble.com/
  HelpScribble includes a special HelpContext property editor for Delphi
  that makes it very easy to link your help file to your application.
}

unit UseHTMLHelp;

interface

uses
  Windows, Messages, SysUtils, Forms;
{
// Commands to pass to HtmlHelp()
const
  HH_DISPLAY_TOPIC        = $0000;
  HH_HELP_FINDER          = $0000;  // WinHelp equivalent
  HH_DISPLAY_TOC          = $0001;
  HH_DISPLAY_INDEX        = $0002;
  HH_DISPLAY_SEARCH       = $0003;
  HH_SET_WIN_TYPE         = $0004;
  HH_GET_WIN_TYPE         = $0005;
  HH_GET_WIN_HANDLE       = $0006;
  HH_ENUM_INFO_TYPE       = $0007;  // Get Info type name, call repeatedly to enumerate, -1 at end
  HH_SET_INFO_TYPE        = $0008;  // Add Info type to filter.
  HH_SYNC                 = $0009;
  HH_RESERVED1            = $000A;
  HH_RESERVED2            = $000B;
  HH_RESERVED3            = $000C;
  HH_KEYWORD_LOOKUP       = $000D;
  HH_DISPLAY_TEXT_POPUP   = $000E;  // display string resource id or text in a popup window
  HH_HELP_CONTEXT         = $000F;  // display mapped numeric value in dwData
  HH_TP_HELP_CONTEXTMENU  = $0010;  // text popup help, same as WinHelp HELP_CONTEXTMENU
  HH_TP_HELP_WM_HELP      = $0011;  // text popup help, same as WinHelp HELP_WM_HELP
  HH_CLOSE_ALL            = $0012;  // close all windows opened directly or indirectly by the caller
  HH_ALINK_LOOKUP         = $0013;  // ALink version of HH_KEYWORD_LOOKUP
  HH_GET_LAST_ERROR       = $0014;  // not currently implemented // See HHERROR.h
  HH_ENUM_CATEGORY        = $0015;	// Get category name, call repeatedly to enumerate, -1 at end
  HH_ENUM_CATEGORY_IT     = $0016;  // Get category info type members, call repeatedly to enumerate, -1 at end
  HH_RESET_IT_FILTER      = $0017;  // Clear the info type filter of all info types.
  HH_SET_INCLUSIVE_FILTER = $0018;  // set inclusive filtering method for untyped topics to be included in display
  HH_SET_EXCLUSIVE_FILTER = $0019;  // set exclusive filtering method for untyped topics to be excluded from display
  HH_INITIALIZE           = $001C;  // Initializes the help system.
  HH_UNINITIALIZE         = $001D;  // Uninitializes the help system.
  HH_PRETRANSLATEMESSAGE  = $00FD;  // Pumps messages. (NULL, NULL, MSG*).
  HH_SET_GLOBAL_PROPERTY  = $00FC;  // Set a global property. (NULL, NULL, HH_GPROP)
}
//function HtmlHelp(hwndCaller: THandle; pszFile: PChar; uCommand: cardinal; dwData: longint): THandle; stdcall;

implementation
// Commands to pass to HtmlHelp()
const
  HH_DISPLAY_TOPIC        = $0000;
  HH_HELP_FINDER          = $0000;  // WinHelp equivalent
  HH_DISPLAY_TOC          = $0001;
  HH_DISPLAY_INDEX        = $0002;
  HH_DISPLAY_SEARCH       = $0003;
  HH_SET_WIN_TYPE         = $0004;
  HH_GET_WIN_TYPE         = $0005;
  HH_GET_WIN_HANDLE       = $0006;
  HH_ENUM_INFO_TYPE       = $0007;  // Get Info type name, call repeatedly to enumerate, -1 at end
  HH_SET_INFO_TYPE        = $0008;  // Add Info type to filter.
  HH_SYNC                 = $0009;
  HH_RESERVED1            = $000A;
  HH_RESERVED2            = $000B;
  HH_RESERVED3            = $000C;
  HH_KEYWORD_LOOKUP       = $000D;
  HH_DISPLAY_TEXT_POPUP   = $000E;  // display string resource id or text in a popup window
  HH_HELP_CONTEXT         = $000F;  // display mapped numeric value in dwData
  HH_TP_HELP_CONTEXTMENU  = $0010;  // text popup help, same as WinHelp HELP_CONTEXTMENU
  HH_TP_HELP_WM_HELP      = $0011;  // text popup help, same as WinHelp HELP_WM_HELP
  HH_CLOSE_ALL            = $0012;  // close all windows opened directly or indirectly by the caller
  HH_ALINK_LOOKUP         = $0013;  // ALink version of HH_KEYWORD_LOOKUP
  HH_GET_LAST_ERROR       = $0014;  // not currently implemented // See HHERROR.h
  HH_ENUM_CATEGORY        = $0015;	// Get category name, call repeatedly to enumerate, -1 at end
  HH_ENUM_CATEGORY_IT     = $0016;  // Get category info type members, call repeatedly to enumerate, -1 at end
  HH_RESET_IT_FILTER      = $0017;  // Clear the info type filter of all info types.
  HH_SET_INCLUSIVE_FILTER = $0018;  // set inclusive filtering method for untyped topics to be included in display
  HH_SET_EXCLUSIVE_FILTER = $0019;  // set exclusive filtering method for untyped topics to be excluded from display
  HH_INITIALIZE           = $001C;  // Initializes the help system.
  HH_UNINITIALIZE         = $001D;  // Uninitializes the help system.
  HH_PRETRANSLATEMESSAGE  = $00FD;  // Pumps messages. (NULL, NULL, MSG*).
  HH_SET_GLOBAL_PROPERTY  = $00FC;  // Set a global property. (NULL, NULL, HH_GPROP)

function HtmlHelp(hwndCaller: THandle; pszFile: PChar; uCommand: cardinal; dwData: longint): THandle; stdcall;
         external 'hhctrl.ocx' name 'HtmlHelpA';

type
  TUseHTMLHelp = class(TObject)
    function ApplicationHelp(Command: Word; Data: Longint; var CallHelp: Boolean): Boolean;
  end;

function TUseHTMLHelp.ApplicationHelp(Command: Word; Data: Longint; var CallHelp: Boolean): Boolean;
begin
  // Make sure VCL doesn't activate WinHelp
  CallHelp := False;
  Result := True;
  // Activate HTML Help
  if Command in [HELP_CONTEXT, HELP_CONTEXTPOPUP] then
    // Ordinary context jump
    HtmlHelp(0, PChar(Application.HelpFile), HH_HELP_CONTEXT, Data)
  else if Command = HELP_KEY then
    // Keyword lookup
    HtmlHelp(0, PChar(Application.HelpFile), HH_DISPLAY_INDEX, Data)
  else if Command = HELP_QUIT then
    // Application quits -> close all its help files
    HtmlHelp(0, nil, HH_CLOSE_ALL, 0)
  else
    // Any other command -> display table of contents
    HtmlHelp(0, PChar(Application.HelpFile), HH_HELP_FINDER, 0)
end;

var
  HTMLHelpUser: TUseHTMLHelp;

initialization
  HTMLHelpUser := TUseHTMLHelp.Create;
  Application.OnHelp := HTMLHelpUser.ApplicationHelp;
finalization
  Application.OnHelp := nil;
  HTMLHelpUser.Free;
end.
