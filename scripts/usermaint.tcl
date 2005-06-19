#!/usr/local/bin/tclsh8.4
if {$argc < 3} {
	puts stderr "usage: %s dbname command arg1 arg2 ...\ncommands:\nadduser uname - asks for pass, no echo"
	exit -1
}
package require md5
load ./terminal.so
load /usr/local/lib/sqlite/libtclsqlite3.so Sqlite3
sqlite db [lindex $argv 0]
set cmd [lindex $argv 1]
if {[string equal $cmd "adduser"]} {
	set uname [lindex $argv 2]
	puts -nonewline "Password: "
	flush stdout
	terminal:echoOff
	set pass [gets stdin]
	terminal:echoOn
	puts ""
	# pkey, login, pass, priv
	set md5pass [string tolower [md5::md5 -hex $pass]]
	db eval {INSERT INTO users VALUES(null, $uname, $md5pass, 32)}
	puts "User added successfully."
}
db close
