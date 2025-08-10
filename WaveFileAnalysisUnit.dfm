object MainForm: TMainForm
  Left = 0
  Top = 0
  Caption = 'Wave file analysis tool'
  ClientHeight = 903
  ClientWidth = 1467
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -16
  Font.Name = 'Consolas'
  Font.Style = []
  WindowState = wsMaximized
  DesignSize = (
    1467
    903)
  TextHeight = 19
  object Splitter1: TSplitter
    Left = 0
    Top = 705
    Width = 1467
    Height = 8
    Cursor = crVSplit
    Align = alTop
  end
  object OpenButton: TButton
    Left = 1351
    Top = 719
    Width = 116
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = #25171#24320#25991#20214'(&O)'
    TabOrder = 0
    OnClick = OpenButtonClick
  end
  object Memo1: TMemo
    Left = 0
    Top = 713
    Width = 1345
    Height = 190
    Align = alLeft
    Anchors = [akLeft, akTop, akRight, akBottom]
    ScrollBars = ssVertical
    TabOrder = 1
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 1467
    Height = 705
    Align = alTop
    TabOrder = 2
    object Splitter2: TSplitter
      Left = 209
      Top = 1
      Width = 7
      Height = 703
      Align = alRight
      ExplicitLeft = 379
      ExplicitTop = -4
    end
    object TreeView1: TTreeView
      Left = 1
      Top = 1
      Width = 208
      Height = 703
      Align = alClient
      AutoExpand = True
      Indent = 19
      TabOrder = 0
      OnClick = TreeView1Click
    end
    object ListView1: TListView
      Left = 216
      Top = 1
      Width = 1250
      Height = 703
      Align = alRight
      Columns = <
        item
          Caption = 'Address'
          Width = 160
        end
        item
          Caption = '0'
          Width = 30
        end
        item
          Caption = '1'
          Width = 30
        end
        item
          Caption = '2'
          Width = 30
        end
        item
          Caption = '3'
          Width = 30
        end
        item
          Caption = '4'
          Width = 30
        end
        item
          Caption = '5'
          Width = 30
        end
        item
          Caption = '6'
          Width = 30
        end
        item
          Caption = '7'
          Width = 30
        end
        item
          Caption = '8'
          Width = 30
        end
        item
          Caption = '9'
          Width = 30
        end
        item
          Caption = 'A'
          Width = 30
        end
        item
          Caption = 'B'
          Width = 30
        end
        item
          Caption = 'C'
          Width = 30
        end
        item
          Caption = 'D'
          Width = 30
        end
        item
          Caption = 'E'
          Width = 30
        end
        item
          Caption = 'F'
          Width = 30
        end
        item
          Caption = 'ASCII'
          Width = 160
        end>
      RowSelect = True
      TabOrder = 1
      ViewStyle = vsReport
    end
  end
  object PlayButton: TButton
    Left = 1351
    Top = 767
    Width = 116
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = #25773#25918'(&Play)'
    Enabled = False
    TabOrder = 3
    OnClick = PlayButtonClick
  end
  object PauseResumeButton: TButton
    Left = 1351
    Top = 823
    Width = 116
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = #26242#20572'(P&ause)'
    Enabled = False
    TabOrder = 4
    OnClick = PauseResumeButtonClick
  end
  object StopButton: TButton
    Left = 1351
    Top = 878
    Width = 116
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = #20572#27490'(&Stop)'
    Enabled = False
    TabOrder = 5
    OnClick = StopButtonClick
  end
  object OpenFileDialog: TOpenDialog
    Filter = 'All files|*.*'
    Left = 16
    Top = 16
  end
  object EventTimer: TTimer
    Interval = 100
    OnTimer = EventTimerTimer
    Left = 96
    Top = 16
  end
end
