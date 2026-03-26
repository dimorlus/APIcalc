object VarsFrm: TVarsFrm
  Left = 298
  Top = 156
  HelpContext = 3000
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Vars'
  ClientHeight = 288
  ClientWidth = 292
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  OnKeyDown = FormKeyDown
  PixelsPerInch = 120
  TextHeight = 16
  object VarsMemo: TMemo
    Left = 0
    Top = 0
    Width = 292
    Height = 288
    HelpContext = 3000
    Align = alClient
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -17
    Font.Name = 'Lucida Console'
    Font.Style = []
    ParentFont = False
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 0
  end
end
