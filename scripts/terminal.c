#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <limits.h> /*For MAX_CANON -- Linux: use <limits.h> instead */
#include <tcl.h>

#define CMD_ARGS (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])

int TerminalEchoOff CMD_ARGS {
  struct termios terminal;
  int fid = fileno (stdout);

  if (objc != 1) {
    Tcl_WrongNumArgs (interp, 0, NULL, "terminal:echoOff");
    return TCL_ERROR;
  }

  tcgetattr (fid, &terminal);
  terminal.c_lflag &= (~ECHO);
  tcsetattr (fid, TCSANOW, &terminal);

  return TCL_OK;
}

int TerminalEchoOn CMD_ARGS {
  struct termios terminal;
  int fid = fileno (stdout);

  if (objc != 1) {
    Tcl_WrongNumArgs (interp, 0, NULL, "terminal:echoOn");
    return TCL_ERROR;
  }

  tcgetattr (fid, &terminal);
  terminal.c_lflag |= (ECHO);
  tcsetattr (fid, TCSANOW, &terminal);

  return TCL_OK;
}

int TerminalCanonicalOff CMD_ARGS {
  struct termios terminal;
  int fid = fileno (stdin);

  if (objc != 1) {
    Tcl_WrongNumArgs (interp, 0, NULL, "terminal:canonicalOff");
    return TCL_ERROR;
  }

  tcgetattr (fid, &terminal);
  terminal.c_lflag &= (~ICANON);
  terminal.c_cc[VTIME] = 0;
  terminal.c_cc[VMIN] = 1;
  tcsetattr (fid, TCSANOW, &terminal);

  return TCL_OK;
}

int TerminalCanonicalOn CMD_ARGS {
  struct termios terminal;
  int fid = fileno (stdin);

  if (objc != 1) {
    Tcl_WrongNumArgs (interp, 0, NULL, "terminal:canonicalOn");
    return TCL_ERROR;
  }

  tcgetattr (fid, &terminal);
  terminal.c_lflag |= (ICANON);
  terminal.c_cc[VTIME] = 0;
  terminal.c_cc[VMIN] = MAX_CANON;
  tcsetattr (fid, TCSANOW, &terminal);

  return TCL_OK;
}

int Terminal_Init (Tcl_Interp *interp) {
  #define OBJ_CMD(name,func) Tcl_CreateObjCommand(interp, name, func, (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL)

  OBJ_CMD ("terminal:echoOff", TerminalEchoOff);
  OBJ_CMD ("terminal:echoOn", TerminalEchoOn);
  OBJ_CMD ("terminal:canonicalOff", TerminalCanonicalOff);
  OBJ_CMD ("terminal:canonicalOn", TerminalCanonicalOn);

  #undef OBJ_CMD
  return TCL_OK;
}
