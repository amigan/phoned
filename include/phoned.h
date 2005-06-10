/*
 * Global stuff for apps in the phoned distribution.
 * (C)2005, Dan Ponte
 * BSDL w/ advert.
 */
/* $Amigan: phoned/include/phoned.h,v 1.4 2005/06/10 00:21:23 dcp1990 Exp $ */
#define VERSION "0.1"
#define LOGFILE "/var/log/phoned.log"
#define SOCKETFILE "/tmp/phoned.sock"
#define CONFIGFILE "phoned.conf"
#define _unused       __attribute__((__unused__))
struct conf {
	char* cfile;
	char* logfile;
	int loglevels;
	char* modemdev;
};
#define LL_DEBUG	0x1
#define	LL_GARBAGE	0x2
#define	LL_INFO		0x4
#define LL_WARN		0x10
#define	LL_ERROR	0x20
#define	LL_CRITICAL	0x40
#define	LL_FATAL	0x80
#define LL_ALL		LL_DEBUG | LL_GARBAGE | LL_INFO | LL_WARN | LL_ERROR \
			| LL_CRITICAL | LL_FATAL /* 247 */
enum ltype {
	debug,
	garbage,
	info,
	warn,
	error,
	critical,
	fatal
};
typedef struct c_t {
	char* name;
	char* number;
	short hour;
	short minute;
	short month;
	short day;
} cid_t;
/* cond_t flags:
 * 1 | 1 | 1
 * ^   ^   ^-------stop processing here? (0x1)
 * |   |---- check name? (0x2)
 * |-- check number? (0x4)
 */
#define CTFLG_STOPPROC	0x1
#define CTFLG_CHECKNAME	0x2
#define CTFLG_CHECKNUMB	0x4
/* action format:
 * 1 | 1 | 1 | 1 | 1 | 1
 * ^   ^   ^   ^   ^   ^-- ignore (0x1)
 * |   |   |   |   |-- hang up (0x2)
 * |   |   |   |-- remote notify (0x4)
 * |   |   |-- answer (not implemented) (0x10)
 * |   |-- play message (not implemented) (0x20)
 * |--- record message (not implemented) (0x40)
 */
#define CTACT_IGN	0x1
#define CTACT_HUP	0x2
#define CTACT_RNOT	0x4
#define CTACT_ANS	0x10
#define CTACT_PLAY	0x20
#define CTACT_REC	0x40
typedef struct cnd_t {
	char* name;
	char* number;
	int action;
	struct af actflags;
	int flags;
	struct cnd_t* last;
	struct cnd_t* next;
} cond_t;
#ifdef HAVE_INET_INCS
typedef struct adll_t {
	in_addr_t addr;
	struct adll_t* next;
} addrsll_t;
addrsll_t* allocaddr(void);
addrsll_t* getlast(addrsll_t* hd);
void freeaddrl(addrsll_t* hd);
#endif
/* function prottypes */
void initialize(void);
void open_log(void);
void read_config(void);
void shutd(void);
void network(void);
int lprintf(enum ltype logtype, const char* fmt, ...);
void handsig(int sig);
void install_handlers(void);
int parse(FILE** fp);
cid_t* parse_cid(char* cidstring);
int free_cid(cid_t* ctf);
void stmod(const char* str);
int init_modem(char* dev);
int close_modem(char* dev);
int cid_notify(cid_t* c);
void flush_lists(void);
void addtoaddrs(const char* par);
void modem_hread(char* cbuf);
void cid_log(cid_t* c);
