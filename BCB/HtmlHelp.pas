unit HtmlHelp;

interface

uses
  Windows, Messages, SysUtils, Classes, Forms;

type
  THelpController = class
  private
    FActive: Boolean;
    FShowing: Boolean;
    procedure ShowHelp(const Command, Data: LongWord);
    procedure CloseHelp;
    function ApplicationHelp(Command: Word;
      Data: Longint; var CallHelp: Boolean): Boolean;
  public
    constructor Create;
    destructor Destroy; override;
  end;

  procedure ShowHelpContents;
  procedure ShowHelpContext(const Context: LongWord);

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

var
  Controller: THelpController;

procedure ShowHelpContents;
begin
  Controller.ShowHelp(HH_DISPLAY_TOC, 0);
end;

procedure ShowHelpContext(const Context: LongWord);
begin
  Controller.ShowHelp(HH_HELP_CONTEXT, Context);
end;

{ THelpController }

constructor THelpController.Create;
begin
  Application.OnHelp := ApplicationHelp;
end;

destructor THelpController.Destroy;
begin
  Application.OnHelp := nil;
  CloseHelp;
end;

procedure THelpController.ShowHelp(const Command, Data: LongWord);
var
  HelpFile: string;
begin
  if (not FShowing) then
  begin
    FShowing := True;
    try
      HelpFile := Application.HelpFile;

      if not FileExists(HelpFile) then
        //MessageBox()
      else
      begin
        FActive := True;
        HtmlHelp(0, Pointer(HelpFile), Command, Data);
      end;
    finally
      FShowing := False;
    end;
  end;
end;

procedure THelpController.CloseHelp;
begin
  if FActive then
  begin
    HtmlHelp(0, nil, HH_CLOSE_ALL, 0);
    Sleep(100);
  end;
end;

function THelpController.ApplicationHelp(Command: Word;
  Data: Longint; var CallHelp: Boolean): Boolean;
begin
  Result := True;
  CallHelp := False;
  if (Command = HELP_CONTEXT) then
    ShowHelp(HH_HELP_CONTEXT, Data)
  else
    if (Command = HELP_CONTENTS) then
      ShowHelp(HH_DISPLAY_TOC, 0);
end;

initialization
  Controller := THelpController.Create;

finalization
  Controller.Free;

end.