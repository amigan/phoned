/*
 * Global stuff for apps in the phoned distribution.
 * (C)2005, Dan Ponte
 * BSDL w/ advert.
 */
/* $Amigan: phoned/include/phoned.h,v 1.19 2005/06/19 02:47:43 dcp1990 Exp $ */
#include <pcre.h> /* fugly, I know... */
#define VERSION "0.1"
#define LOGFILE "/var/log/phoned.log"
#define SOCKETFILE "/tmp/phoned.sock"
#define CONFIGFILE "phoned.conf"
#define DBFILE	"phoned.sqlite"
#define _unused       __attribute__((__unused__))
struct conf {
	char* cfile;
	char* logfile;
	int loglevels;
	short modem_tm;
	char* modemdev;
	char *dbfile;
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
#define SCTACT_IGN	"\1"
#define SCTACT_HUP	"\2"
#define SCTACT_RNOT	"\3"
#define SCTACT_ANS	"\4"
#define SCTACT_PLAY	"\5"
#define SCTACT_REC	"\6"

struct af {
	char* not_email;
	char* play_file;
	char* record_file;
};
typedef struct rx_t {
	pcre* prex;
	const char* error;
	int erroroffset;
	int pcresize;
} rex_t;
typedef struct cnd_t {
	char* filtername;
	char* name;
	char* number;
	rex_t namerx;
	rex_t numbrx;
	int action;
	struct af actflags;
	int flags;
	struct cnd_t* last;
	struct cnd_t* next;
} cond_t;
typedef enum modes_t {
	ok,
	ring,
	incoming,
	err
} mod_res_t;
typedef struct mod_t {
	char*		modem_name;
	int		features;
	int		deffuncs;
	int		(*init)(void);
	int		(*destroy)(void);
	mod_res_t	(*evalrc)(char *result);
	void		(*pickup)(void);
	void		(*hangup)(void);
} modem_t;	
typedef enum stat {
	init = 0,
	loginstage,
	pass
} states_t;
typedef struct si_t {
	states_t st;
	short freeit;
	FILE* fpo;
	int fd;
} state_info_t;

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
short read_config(void);
void shutd(int whatdone);
void *network(void *b);
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
cond_t* add_condition(char* filtname, char* nameregex, char* numregex, int action, int flags);
void free_condition(cond_t* h, short traverse);
cond_t* copy_condition(cond_t* con);
void check_condition(cid_t* cid);
void *modem_io(void *k);
void give_me_modem(char* str);
char *parse_command(const char *cmd, short *cont, state_info_t *s);
void begin_dialogue(FILE* fp, int fd);
short db_init(char* dbfilename);
short db_destroy(void);
short db_add_call(cid_t* c, time_t t);
void cid_handle(cid_t *c);
void awaken_sel(void);
void modem_wake(void);
/* old stuff...
void modem_pickup(void);
void modem_hangup(void);
*/
/* function defs for modem... */
#define modem_pickup	mo->pickup
#define modem_hangup	mo->hangup
#define modem_evalrc	mo->evalrc
#ifndef MODEM_C
extern modem_t *mo;
#endif
