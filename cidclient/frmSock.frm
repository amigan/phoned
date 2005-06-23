VERSION 5.00
Object = "{248DD890-BB45-11CF-9ABC-0080C7E7B78D}#1.0#0"; "MSWINSCK.OCX"
Begin VB.Form frmSock 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "CIDClient"
   ClientHeight    =   2625
   ClientLeft      =   150
   ClientTop       =   840
   ClientWidth     =   4530
   ControlBox      =   0   'False
   Icon            =   "frmSock.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2625
   ScaleWidth      =   4530
   StartUpPosition =   3  'Windows Default
   Visible         =   0   'False
   Begin MSWinsockLib.Winsock wsController 
      Left            =   480
      Top             =   0
      _ExtentX        =   741
      _ExtentY        =   741
      _Version        =   393216
      Protocol        =   1
      RemotePort      =   1450
   End
   Begin MSWinsockLib.Winsock UDPListener 
      Left            =   0
      Top             =   0
      _ExtentX        =   741
      _ExtentY        =   741
      _Version        =   393216
      Protocol        =   1
   End
   Begin VB.Menu mnuST 
      Caption         =   "mnuST"
      Begin VB.Menu mnuSetServ 
         Caption         =   "&Set Server"
      End
      Begin VB.Menu mnuExit 
         Caption         =   "&Exit"
      End
      Begin VB.Menu mnuChgPrt 
         Caption         =   "&About"
      End
   End
End
Attribute VB_Name = "frmSock"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim lastdata As String
Private Declare Function sndPlaySound Lib "winmm.dll" _
Alias "sndPlaySoundA" (ByVal lpszSoundName As String, _
ByVal uFlags As Long) As Long

Public nome, number, dte, tme, code As String

Private Sub mnuChgPrt_Click()
MsgBox "CIDClient by Dan Ponte" & Chr(13) & "Displays CallerID on UDP Packets generated by the" _
 & Chr(13) & "modem server for Unix. BSD Style."
End Sub

Private Sub mnuExit_Click()
frmSock.UDPListener.Close
Form_Unload (0)
End
End Sub

Public Sub mnuSetServ_Click()
    Dim thehost As String
    thehost = InputBox("Server IP/host?", "CIDClient", wsController.RemoteHost)
    If thehost = "" Then Exit Sub
    wsController.RemoteHost = thehost
    SaveSetting "CIDClient", "Main", "host", thehost
End Sub

'Format:
'DATE:TIME:CODE:NAME:NUMBER
Private Sub UDPListener_Error(ByVal number As Integer, Description As String, ByVal Scode As Long, ByVal Source As String, ByVal HelpFile As String, ByVal HelpContext As Long, CancelDisplay As Boolean)
MsgBox number & ":" & Description
End Sub
Private Sub UDPListener_DataArrival(ByVal bytesTotal As Long)
On Error Resume Next
    UDPListener.GetData lastdata
    If Left(lastdata, 4) = "RING" Then
        SoundFile = App.Path & "\RING.WAV"
        Result = sndPlaySound(SoundFile, 1)
    Else
        dte = Split(lastdata, ":")(0)
        tme = Split(lastdata, ":")(1)
        code = Split(lastdata, ":")(2)
        nome = Split(lastdata, ":")(3)
        number = Split(lastdata, ":")(4)
        popupWindow dte, tme, code, number, nome
    End If
End Sub
Public Sub popupWindow(ByVal pdt As String, ByVal ptm As String, ByVal cde As String, ByVal num As String, ByVal nom As String)
    frmPopup.lblName = nom
    frmPopup.lblNumb = Format(num, "(###) ###-####")
    frmPopup.lblTime = Format(ptm, "##:##")
    frmPopup.lblDate = Format(pdt, "##/##")
    frmPopup.Show
    frmPopup.Timer1 = True
    frmPopup.ZOrder 0
End Sub
'THIS MAKES THE MENU POPUP WHEN THE FORM IS HIDDEN IN THE SYSTRAY'
Private Sub Form_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
Dim Sys As Long
Sys = x / Screen.TwipsPerPixelX
Select Case Sys
Case WM_RBUTTONDOWN:
Me.PopupMenu mnuST
End Select
End Sub

'THIS MAKES THE FOR DISSAPEAR/MINIMIZE TO THE SYSTRAY'
Public Sub Form_Resize()
If WindowState = vbMinimized Then
Me.Hide
Me.Refresh
With nid
.cbSize = Len(nid)
.hwnd = Me.hwnd
.uId = vbNull
.uFlags = NIF_ICON Or NIF_TIP Or NIF_MESSAGE
.uCallBackMessage = WM_MOUSEMOVE
.hIcon = Me.Icon
.szTip = Me.Caption & vbNullChar
End With
Shell_NotifyIcon NIM_ADD, nid
Else
Shell_NotifyIcon NIM_DELETE, nid
End If
End Sub

'THIS WILL KILL THE SYSTRAY ICON IF THE FORM IS UNLOADED'
Private Sub Form_Unload(Cancel As Integer)
Shell_NotifyIcon NIM_DELETE, nid
End
End Sub
