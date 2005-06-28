/*
 * Copyright (c) 2005, Dan Ponte
 *
 * db.c - database stuff, currently for SQLite
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* $Amigan: phoned/phoned/db.c,v 1.8 2005/06/28 02:02:21 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
/* sqlite 3 */
/* LONGLONG */
#include <sqlite3.h>

#include <phoned.h>
#include <ourmd5.h>

#define USERS_TABLE	"users"
#define CALLS_TABLE	"calls"

pthread_mutex_t dbmx = PTHREAD_MUTEX_INITIALIZER;
sqlite3 *db; /* lock this shiat, bizzatch */

short db_check_for_table(tablename, dbh) /* needs to be locked before use!!!!!!! */
	char *tablename;
	sqlite3 *dbh;
{
	const char *tail;
	const unsigned char *result;
	sqlite3_stmt *cst;
	const char* const sql = "SELECT name FROM sqlite_master WHERE type='table'"; /* check for the table in the master table */
	int rc;
	if((rc = sqlite3_prepare(dbh, sql, strlen(sql), &cst, &tail)) != SQLITE_OK) {
		lprintf(error, "db_check_for_table(%s, 0x%p): error in prepare: %s\n", tablename, dbh, sqlite3_errmsg(dbh));
		return 0;
	}
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		result = sqlite3_column_text(cst, 0);
		if(strcmp((const char *)result, tablename) == 0) {
			sqlite3_finalize(cst);
			return 1;
		}
	}
	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		lprintf(error, "db_check_for_table(%s, 0x%p): error after loop: %s\n", tablename, dbh, sqlite3_errmsg(dbh));
		return 0;
	}
	sqlite3_finalize(cst);
	return 0;
}
short db_create_calls_table(dbh)
	sqlite3 *dbh;
{
	int rc;
	char *emsg;
	const char* const sql = 
		"CREATE TABLE " CALLS_TABLE " (id INTEGER PRIMARY KEY, dtime INTEGER, number TEXT, name TEXT, cidmon INTEGER, cidday INTEGER, cidhour INTEGER, cidmin INTEGER)";
	rc = sqlite3_exec(dbh, sql, NULL, NULL, &emsg);
	if(rc != SQLITE_OK) {
		lprintf(error, "db_create_calls_table(0x%p): error: %s\n", dbh, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}

short db_create_users_table(dbh)
	sqlite3 *dbh;
{
	int rc;
	char *emsg;
	const char* const sql = 
		"CREATE TABLE " USERS_TABLE " (id INTEGER PRIMARY KEY, login TEXT, passmd5 TEXT, privlvl INTEGER)";
	rc = sqlite3_exec(dbh, sql, NULL, NULL, &emsg);
	if(rc != SQLITE_OK) {
		lprintf(error, "db_create_users_table(0x%p): error: %s\n", dbh, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}

short db_init(dbfilename)
	char *dbfilename;
{
	int rc;
	pthread_mutex_lock(&dbmx);
	if((rc = sqlite3_open(dbfilename, &db))) {
		lprintf(error, "sqlite engine: Error opening database %s: %s\n", dbfilename, sqlite3_errmsg(db));
		sqlite3_close(db);
		pthread_mutex_unlock(&dbmx);
		return 0;
	}
	if(!db_check_for_table(USERS_TABLE, db)) {
		lprintf(warn, "WARNING: users table didn't exist, you will likely be denied access!\n");
		if(!db_create_users_table(db)) {
			pthread_mutex_unlock(&dbmx);
			return 0;
		}
	}
	if(!db_check_for_table(CALLS_TABLE, db)) {
		lprintf(info, "Calls table didn't exist, creating...\n");
		if(!db_create_calls_table(db)) {
			pthread_mutex_unlock(&dbmx);
			return 0;
		}
	}
	pthread_mutex_unlock(&dbmx);
	lprintf(info, "Database opened.\n");
	return 1;
}

short db_destroy(void)
{
	pthread_mutex_lock(&dbmx);
	sqlite3_close(db);
	pthread_mutex_unlock(&dbmx);
	return 1;
}

/* stuff that does stuff */

short db_check_crend(loginna, pass)
	char *loginna;
	char *pass; /* not md5'd yet */
{
	int rc;
	char *sql;
	md5_state_t state;
	md5_byte_t digest[16];
	const char *tail;
	char pmd5[20];
	int di;
	const unsigned char *lres, *pres;
	sqlite3_stmt *cst;
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)pass, strlen(pass));
	md5_finish(&state, digest);
	for(di = 0; di < 16; ++di)
		sprintf(pmd5 + di * 2, "%02x", digest[di]);
	sql = sqlite3_mprintf("SELECT login,passmd5 FROM " USERS_TABLE " WHERE login=='%q' AND passmd5=='%q'", loginna, pmd5);
	pthread_mutex_lock(&dbmx);
	rc = sqlite3_prepare(db, sql, strlen(sql), &cst, &tail);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		lprintf(error, "db_check_crend: error prepare: %s\n", sqlite3_errmsg(db));
		pthread_mutex_unlock(&dbmx);
		return 0;
	}
	di = 0;
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		lres = sqlite3_column_text(cst, 0);
		pres = sqlite3_column_text(cst, 1);
		if(strcmp(lres, loginna) == 0 && strcasecmp(pres, pmd5) == 0) {
			di = 1;
			break;
		}
	}
	if(rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) {
		lprintf(error, "db_check_crend: error step: %s\n", sqlite3_errmsg(db));
		pthread_mutex_unlock(&dbmx);
		return 0;
	}
	sqlite3_finalize(cst);
	pthread_mutex_unlock(&dbmx);
	if(di) return 1;
	return 0;
}
short db_add_call(c, t)
	cid_t *c;
	time_t t;
{
	int rc;
	char *emsg;
	char *sql;
	pthread_mutex_lock(&dbmx);
	sql = sqlite3_mprintf("INSERT INTO " CALLS_TABLE " VALUES(null, %d, '%q', '%q', %d, %d, %d, %d)",
			t, c->name, c->number, c->month, c->day, c->hour, c->minute);
	if((rc = sqlite3_exec(db, sql, NULL, 0x0, &emsg)) != SQLITE_OK) {
		lprintf(error, "Error adding call to db: %s\n", emsg);
		sqlite3_free(emsg);
		sqlite3_free(sql);
		return 0;
	}
	sqlite3_free(sql);
	pthread_mutex_unlock(&dbmx);
	return 1;
}
