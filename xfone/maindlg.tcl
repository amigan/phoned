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
# $Amigan: phoned/xfone/maindlg.tcl,v 1.2 2005/06/26 16:51:00 dcp1990 Exp $
# vars
proc logindlg {} {
	toplevel .login
	frame .login.m -width 5c -height 2c
	grid .m -row 0 -column 0
	label .login.m.lo -text "Login:"
	entry .login.m.loge
	grid .login.m.lo .login.m.loge -row 0
	label .login.m.pa -text "Pass:"
	entry .login.m.pase -show "*"
	grid .login.m.pa .login.m.pase -row 1
	button .login.m.log -text "Login" -command {loginProc [.login.m.loge get] [.login.m.pase get]}
	button .login.m.cancel -text "Cancel" -command {destroy .login}
	grid .login.m.log -row 0 -column 0 -sticky ew
	grid .login.m.cancel -row 0 -column 1 -sticky ew
}
set prj .mbar.project
set phdm .mbar.phoned
#frame
frame .m -borderwidth 2 -width 20c -height 10c
grid .m -sticky wn -row 0 -column 0
#menu
menu .mbar -tearoff no
menu $prj -tearoff no
.mbar add cascade -label "Project" -menu $prj -underline 1
$prj add command -label "Quit" -command "exit"
menu $phdm -tearoff no
.mbar add cascade -label "PhoneD" -menu $phdm -underline 1
$phdm add command -label "Login"
menu .mbar.help -tearoff no
.mbar add cascade -label "Help" -menu .mbar.help -underline 1
.mbar.help add command -label "About Xfone"
. configure -menu .mbar
