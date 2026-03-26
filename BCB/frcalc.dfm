object CalcForm: TCalcForm
  Left = 500
  Top = 253
  HelpContext = 3005
  BorderIcons = [biSystemMenu, biMinimize, biMaximize, biHelp]
  BorderStyle = bsSingle
  Caption = 'Calc'
  ClientHeight = 256
  ClientWidth = 667
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  Menu = MainMenu
  OldCreateOrder = False
  PopupMenu = PopupMenu
  ShowHint = True
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  OnShow = FormShow
  DesignSize = (
    667
    256)
  PixelsPerInch = 120
  TextHeight = 16
  object CBStr: TComboBox
    Left = 0
    Top = 0
    Width = 668
    Height = 21
    AutoComplete = False
    Anchors = [akLeft, akTop, akRight]
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'Courier'
    Font.Pitch = fpFixed
    Font.Style = []
    ItemHeight = 13
    ParentFont = False
    Sorted = True
    TabOrder = 0
    OnChange = CBStrChange
    OnKeyDown = CBStrKeyDown
    OnKeyPress = CBStrKeyPress
    OnKeyUp = CBStrKeyUp
    OnSelect = CBStrSelect
  end
  object MOutput: TMemo
    Left = 0
    Top = 21
    Width = 667
    Height = 235
    HelpContext = 3005
    Anchors = [akLeft, akTop, akRight, akBottom]
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'Courier'
    Font.Pitch = fpFixed
    Font.Style = []
    Lines.Strings = (
      ''
      ''
      ''
      ''
      ''
      ''
      ''
      ''
      ''
      '')
    ParentFont = False
    ReadOnly = True
    TabOrder = 1
  end
  object MainMenu: TMainMenu
    Left = 48
    Top = 32
    object MnCalc: TMenuItem
      Caption = '&Calc'
      object Cstyle: TMenuItem
        Caption = '&C-style'
        Hint = 'Expression style'
        OnClick = CstyleClick
      end
      object Casesensitive: TMenuItem
        Caption = 'C&ase sensitive'
        Hint = 'Case sensitive variables'
        OnClick = CasesensitiveClick
      end
      object Forcedfloat: TMenuItem
        Caption = 'Forced &float'
        Hint = 'Disable integer calculation'
        OnClick = ForcedfloatClick
      end
      object ImpMul: TMenuItem
        Caption = 'Implicit multiplication'
        Checked = True
        OnClick = ImpMulClick
      end
      object Format: TMenuItem
        Caption = 'F&ormat...'
        Hint = 'Select output formats'
        object Scientific: TMenuItem
          Caption = '&Scientific'
          OnClick = ScientificClick
        end
        object Normalized: TMenuItem
          Caption = '&Normalized'
          OnClick = NormalizedClick
        end
        object Fraction: TMenuItem
          Caption = 'Fraction'
          OnClick = FractionClick
        end
        object Computing: TMenuItem
          Caption = '&Computing'
          OnClick = ComputingClick
        end
        object Integer: TMenuItem
          Caption = '&Integer'
          OnClick = IntegerClick
        end
        object Unsigned: TMenuItem
          Caption = '&Unsigned'
          OnClick = UnsignedClick
        end
        object Hex: TMenuItem
          Caption = '&Hex'
          OnClick = HexClick
        end
        object Octal: TMenuItem
          Caption = '&Octal'
          OnClick = OctalClick
        end
        object Binary: TMenuItem
          Caption = '&Binary'
          OnClick = BinaryClick
        end
        object Char: TMenuItem
          Caption = 'Cha&r'
          OnClick = CharClick
        end
        object WChar: TMenuItem
          Caption = '&Wide char'
          OnClick = WCharClick
        end
        object Datetime: TMenuItem
          Caption = 'Date &time'
          OnClick = DatetimeClick
        end
        object Unixtime: TMenuItem
          Caption = '&Unix time'
          OnClick = UnixtimeClick
        end
        object Degrees: TMenuItem
          Caption = '&Degrees'
          OnClick = DegreesClick
        end
        object Temperature: TMenuItem
          Caption = '&Temperature'
          OnClick = TemperatureClick
        end
        object String: TMenuItem
          Caption = 'Strin&g'
          OnClick = StringClick
        end
        object Inch: TMenuItem
          Caption = 'Inch'
          OnClick = InchClick
        end
        object All: TMenuItem
          Caption = '&All'
          OnClick = AllClick
        end
      end
      object Binarywidth: TMenuItem
        Caption = 'Binary width'
        Hint = 'Select binary and HEX width'
        object N8: TMenuItem
          Caption = '8'
          OnClick = N8Click
        end
        object N16: TMenuItem
          Caption = '16'
          OnClick = N16Click
        end
        object N24: TMenuItem
          Caption = '24'
          OnClick = N24Click
        end
        object N32: TMenuItem
          Caption = '32'
          OnClick = N32Click
        end
        object N48: TMenuItem
          Caption = '48'
          OnClick = N48Click
        end
        object N64: TMenuItem
          Caption = '64'
          OnClick = N64Click
        end
        object binwide64: TMenuItem
          Caption = 'binwide(64)'
          OnClick = binwide64Click
        end
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object Variables1: TMenuItem
        Caption = '&View variables'
        Hint = 'Show variables list'
        OnClick = Variables1Click
      end
      object N2: TMenuItem
        Caption = '-'
      end
      object Opacity: TMenuItem
        Caption = 'Opacity(100)'
        OnClick = OpacityClick
      end
      object Escmin: TMenuItem
        Caption = 'Esc &minimized'
        Hint = 'ESC minimized or close'
        OnClick = EscminClick
      end
      object Exit1: TMenuItem
        Caption = 'E&xit'
        Hint = 'Close'
        ShortCut = 49240
        OnClick = Exit1Click
      end
    end
  end
  object PopupMenu: TPopupMenu
    Left = 112
    Top = 32
    object ScientificPop: TMenuItem
      Caption = '&Scientific'
      OnClick = ScientificClick
    end
    object NormalizedPop: TMenuItem
      Caption = '&Normalized'
      OnClick = NormalizedClick
    end
    object FractionPop: TMenuItem
      Caption = 'Fraction'
      OnClick = FractionClick
    end
    object ComputingPop: TMenuItem
      Caption = '&Computing'
      OnClick = ComputingClick
    end
    object IntegerPop: TMenuItem
      Caption = '&Integer'
      OnClick = IntegerClick
    end
    object UnsignedPop: TMenuItem
      Caption = '&Unsigned'
      OnClick = UnsignedClick
    end
    object HexPop: TMenuItem
      Caption = '&Hex'
      OnClick = HexClick
    end
    object OctalPop: TMenuItem
      Caption = '&Octal'
      OnClick = OctalClick
    end
    object BinaryPop: TMenuItem
      Caption = '&Binary'
      OnClick = BinaryClick
    end
    object CharPop: TMenuItem
      Caption = 'Cha&r'
      OnClick = CharClick
    end
    object WCharPop: TMenuItem
      Caption = '&Wide char'
      OnClick = WCharClick
    end
    object DatetimePop: TMenuItem
      Caption = 'Date &time'
      OnClick = DatetimeClick
    end
    object UnixtimePop: TMenuItem
      Caption = '&Unix time'
      OnClick = UnixtimeClick
    end
    object DegreesPop: TMenuItem
      Caption = '&Degrees'
      OnClick = DegreesClick
    end
    object TemperaturePop: TMenuItem
      Caption = '&Temperature'
      OnClick = TemperatureClick
    end
    object StringPop: TMenuItem
      Caption = 'Strin&g'
      OnClick = StringClick
    end
    object InchPop: TMenuItem
      Caption = 'Inch'
      OnClick = InchClick
    end
    object AllPop: TMenuItem
      Caption = '&All'
      OnClick = AllClick
    end
  end
end
