#!/usr/local/bin/wish8.4
# Copyright (c) 2005, Dan Ponte
#
# maindlg.tcl - main dialogue
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 4. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $Amigan: phoned/xfone/maindlg.tcl,v 1.3 2005/06/27 00:08:47 dcp1990 Exp $
# vars
proc logindlg {} {
	toplevel .login
	label .login.lo -text "Login:"
	entry .login.loge
	grid .login.lo -row 0 -column 0
	grid .login.loge -row 0 -column 1
	label .login.pa -text "Pass:"
	entry .login.pase -show "*"
	grid .login.pa  -row 1 -column 0
	grid .login.pase -row 1 -column 1
	button .login.log -text "Login" -command {login [.login.loge get] [.login.pase get] ; grab release .login; destroy .login}
	button .login.cancel -text "Cancel" -command {grab release .login; destroy .login}
	grid .login.log -row 2 -column 0 -sticky ew
	grid .login.cancel -row 2 -column 1 -sticky ew
	grab set .login
	focus .login
	wm resizable .login 0 0
	wm title .login "Login"
}
set prj .mbar.project
set phdm .mbar.phoned
#frame
frame .m -borderwidth 2 -width 20c -height 10c
grid .m -sticky nsew -row 0 -column 0
grid columnconfigure .m 0 -weight 1 -uniform 1
grid columnconfigure .m 1 -weight 1 -uniform 1
grid rowconfigure .m 0 -weight 1 -uniform 1
grid rowconfigure .m 1 -weight 1 -uniform 1
grid rowconfigure .m 2 -weight 1 -uniform 1
#menu
menu .mbar -tearoff no
menu $prj -tearoff no
.mbar add cascade -label "Project" -menu $prj -underline 1
$prj add command -label "Quit" -command "exit"
menu $phdm -tearoff no
.mbar add cascade -label "PhoneD" -menu $phdm -underline 1
$phdm add command -label "Login" -command logindlg
menu .mbar.help -tearoff no
.mbar add cascade -label "Help" -menu .mbar.help -underline 1
.mbar.help add command -label "About Xfone"
. configure -menu .mbar


# main widgets
set w .m.calls
button .m.calls -text "Calls"
$w configure -font "[font actual [$w cget -font]] -size 20"
set w .m.msgs
button .m.msgs -text "Messages"
$w configure -font "[font actual [$w cget -font]] -size 20"
set w .m.admin
button .m.admin -text "Administration" 
$w configure -font "[font actual [$w cget -font]] -size 20"
set w .m.dialer
button .m.dialer -text Dialer 
$w configure -font "[font actual [$w cget -font]] -size 20"
set w .m.utility
button .m.utility -text "Utilities" 
$w configure -font "[font actual [$w cget -font]] -size 20"
set w .m.logoi
button .m.logoi -text Login
$w configure -font "[font actual [$w cget -font]] -size 20"
grid propagate .m false
grid .m.calls .m.dialer -row 0 -sticky nesw
grid .m.msgs .m.utility -row 1 -sticky nesw
grid .m.admin .m.logoi -row 2 -sticky nesw
wm resizable . 0 0
wm title . "Xfone"
