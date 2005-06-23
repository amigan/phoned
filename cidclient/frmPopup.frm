VERSION 5.00
Begin VB.Form frmPopup 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Incoming Call"
   ClientHeight    =   3210
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   4695
   ControlBox      =   0   'False
   Icon            =   "frmPopup.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3210
   ScaleWidth      =   4695
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton telemarketer 
      Caption         =   "Telemarketer-ize"
      Height          =   255
      Left            =   1680
      TabIndex        =   6
      Top             =   2280
      Width           =   1455
   End
   Begin VB.Timer Timer1 
      Interval        =   1000
      Left            =   3960
      Top             =   1440
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "&OK"
      Default         =   -1  'True
      Height          =   495
      Left            =   120
      TabIndex        =   5
      Top             =   2640
      Width           =   4455
   End
   Begin VB.Shape Shape1 
      BorderColor     =   &H80000001&
      FillColor       =   &H80000003&
      FillStyle       =   0  'Solid
      Height          =   375
      Index           =   1
      Left            =   4200
      Shape           =   3  'Circle
      Top             =   120
      Width           =   375
   End
   Begin VB.Shape Shape1 
      BorderColor     =   &H80000001&
      FillColor       =   &H80000003&
      FillStyle       =   0  'Solid
      Height          =   375
      Index           =   0
      Left            =   120
      Shape           =   3  'Circle
      Top             =   120
      Width           =   375
   End
   Begin VB.Label lblTime 
      Caption         =   "14:30"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   15.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   1080
      TabIndex        =   4
      Top             =   1200
      Width           =   855
   End
   Begin VB.Label lblDate 
      Caption         =   "01/01"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   15.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   2880
      TabIndex        =   3
      Top             =   1200
      Width           =   855
   End
   Begin VB.Label lblNumb 
      Caption         =   "(###) ###-####"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   27.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   735
      Left            =   240
      TabIndex        =   2
      Top             =   1680
      Width           =   4455
   End
   Begin VB.Label lblName 
      Caption         =   "John Smith"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   18
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   1200
      TabIndex        =   1
      Top             =   720
      Width           =   3375
   End
   Begin VB.Label Label1 
      Alignment       =   2  'Center
      Caption         =   "Incoming Call"
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   600
      TabIndex        =   0
      Top             =   0
      Width           =   3375
   End
   Begin VB.Image Image1 
      Height          =   855
      Left            =   120
      Picture         =   "frmPopup.frx":5C12
      Stretch         =   -1  'True
      Top             =   600
      Width           =   855
   End
End
Attribute VB_Name = "frmPopup"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim number As Integer

Private Sub cmdOK_Click()
Timer1.Enabled = False
number = 0
frmPopup.Hide
End Sub


Private Sub telemarketer_Click()
    frmSock.wsController.SendData "AHU" & Chr(10)
End Sub

Private Sub Timer1_Timer()
  number = number + 1
  If number = 15 Then
    number = 0
    Timer1.Enabled = False
    frmPopup.Hide
  End If
  frmPopup.Caption = "Incoming Call (" & 15 - number & ")"
End Sub
