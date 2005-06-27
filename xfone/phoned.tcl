#!/usr/local/bin/wish8.4
# Copyright (c) 2005, Dan Ponte
#
# phoned.tcl - Tcl interface to phoned's socket using my udom package
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
# $Amigan: phoned/xfone/phoned.tcl,v 1.7 2005/06/27 21:08:04 dcp1990 Exp $
set loggedin false
set callscb addtocallslist
proc openSock {sfile} {
	set os [udom -file $sfile]
	fconfigure $os -buffering line -blocking false
	fileevent $os readable [list handleme $os]
	return $os
}
proc handleme {fh} {
	parseres [gets $fh]
}
proc parseres {res} {
# 501 = success, 514 = failure
#	tk_messageBox -message $res -type ok -title Result
	global loggedin
	global callscb
	if {[regexp -- "^(\[0-9\]{3}) (\[A-Z\]+): (.*)$" $res a code msg data]} {
		switch $code {
			501 {
				tk_messageBox -message "Logged in!" -type ok -title "Login"
				set loggedin true
			}
			514 {
				set res [tk_messageBox -message "Login failure." -type retrycancel]
				switch $res { retry {logindlg} cancel {return} }
			}
			700 {
				$callscb $data
			}
			default {
				tk_messageBox -message [list Result was $res] -type ok -title "Result"
			}
		}
	}
}

proc login {user pass} {
	global sh
	puts $sh [list login $user $pass]
}

set sh [openSock $sockfile]
