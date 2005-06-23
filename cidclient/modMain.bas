Attribute VB_Name = "modMain"
Sub main()
Dim thehost As String
frmSock.UDPListener.Close
frmSock.UDPListener.LocalPort = 3890
frmSock.UDPListener.Bind 3890
thehost = GetSetting("CIDClient", "Main", "host", "buhwhy?")
If thehost = "buhwhy?" Then
ths:
thehost = InputBox("Server IP/host?", "CIDClient")
    If thehost = "" Then MsgBox "Needs Host": GoTo ths
    frmSock.wsController.RemoteHost = thehost
    SaveSetting "CIDClient", "Main", "host", thehost
    Else
    frmSock.wsController.RemoteHost = thehost
    End If

'frmSock.Hide
frmSock.WindowState = vbMinimized
frmSock.Show
frmSock.Form_Resize
End Sub
