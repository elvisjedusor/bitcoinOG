// Berkeley DB C++ compatibility layer for modern systems
// Provides C++ wrapper classes using the Berkeley DB C API
// This replaces the deprecated db_cxx.h header

#ifndef DB_CXX_COMPAT_H
#define DB_CXX_COMPAT_H

// On Windows with MSVC in GUI mode, prevent Berkeley DB from redefining ssize_t
// wxWidgets already defines it in wx/types.h
// We temporarily rename ssize_t to a unique name during db.h inclusion
#if defined(_MSC_VER) && defined(__WXMSW__)
#define ssize_t bdb_ssize_t
#endif

#include <db.h>

#if defined(_MSC_VER) && defined(__WXMSW__)
#undef ssize_t
#endif
#include <cstdlib>
#include <cstring>
#include <stdexcept>

class DbException : public std::exception
{
private:
    int err;
    char msg[256];
public:
    DbException(int error) : err(error) {
        snprintf(msg, sizeof(msg), "DbException: %s", db_strerror(error));
    }
    virtual const char* what() const noexcept { return msg; }
    int get_errno() const { return err; }
};

class Dbt
{
private:
    DBT dbt;
public:
    Dbt() { memset(&dbt, 0, sizeof(dbt)); }
    Dbt(void* data, size_t size) {
        memset(&dbt, 0, sizeof(dbt));
        dbt.data = data;
        dbt.size = size;
    }
    ~Dbt() {}

    void set_data(void* data) { dbt.data = data; }
    void* get_data() const { return dbt.data; }
    void set_size(u_int32_t size) { dbt.size = size; }
    u_int32_t get_size() const { return dbt.size; }
    void set_flags(u_int32_t flags) { dbt.flags = flags; }
    u_int32_t get_flags() const { return dbt.flags; }
    void set_ulen(u_int32_t ulen) { dbt.ulen = ulen; }

    DBT* get_DBT() { return &dbt; }
    const DBT* get_const_DBT() const { return &dbt; }
    operator DBT*() { return &dbt; }
};

class DbTxn
{
private:
    DB_TXN* txn;
public:
    DbTxn(DB_TXN* t) : txn(t) {}
    ~DbTxn() {}

    int commit(u_int32_t flags) {
        int ret = txn->commit(txn, flags);
        return ret;
    }

    int abort() {
        int ret = txn->abort(txn);
        return ret;
    }

    DB_TXN* get_DB_TXN() { return txn; }
    operator DB_TXN*() { return txn; }
};

class Dbc
{
private:
    DBC* cursor;
public:
    Dbc(DBC* c) : cursor(c) {}
    ~Dbc() {}

    int get(Dbt* key, Dbt* data, u_int32_t flags) {
        return cursor->get(cursor, key->get_DBT(), data->get_DBT(), flags);
    }

    int close() {
        if (cursor) {
            int ret = cursor->close(cursor);
            cursor = NULL;
            return ret;
        }
        return 0;
    }

    DBC* get_DBC() { return cursor; }
};

class DbEnv;
class Db;

class DbEnv
{
private:
    DB_ENV* dbenv;
    bool owns_env;
public:
    DbEnv(u_int32_t flags) : dbenv(NULL), owns_env(true) {
        int ret = db_env_create(&dbenv, flags);
        if (ret != 0)
            throw DbException(ret);
    }

    ~DbEnv() {
        if (dbenv && owns_env) {
            dbenv->close(dbenv, 0);
        }
    }

    int open(const char* db_home, u_int32_t flags, int mode) {
        return dbenv->open(dbenv, db_home, flags, mode);
    }

    int close(u_int32_t flags) {
        if (dbenv) {
            int ret = dbenv->close(dbenv, flags);
            dbenv = NULL;
            return ret;
        }
        return 0;
    }

    int set_cachesize(u_int32_t gbytes, u_int32_t bytes, int ncache) {
        return dbenv->set_cachesize(dbenv, gbytes, bytes, ncache);
    }

    int set_lg_dir(const char* dir) {
        return dbenv->set_lg_dir(dbenv, dir);
    }

    int set_lg_max(u_int32_t lg_max) {
        return dbenv->set_lg_max(dbenv, lg_max);
    }

    int set_lk_max_locks(u_int32_t max) {
        return dbenv->set_lk_max_locks(dbenv, max);
    }

    int set_lk_max_objects(u_int32_t max) {
        return dbenv->set_lk_max_objects(dbenv, max);
    }

    int set_errfile(FILE* errfile) {
        dbenv->set_errfile(dbenv, errfile);
        return 0;
    }

    int txn_begin(DbTxn* parent, DbTxn** txnp, u_int32_t flags) {
        DB_TXN* parent_txn = parent ? parent->get_DB_TXN() : NULL;
        DB_TXN* new_txn = NULL;
        int ret = dbenv->txn_begin(dbenv, parent_txn, &new_txn, flags);
        if (ret == 0 && new_txn) {
            *txnp = new DbTxn(new_txn);
        }
        return ret;
    }

    int txn_checkpoint(u_int32_t kbyte, u_int32_t min, u_int32_t flags) {
        return dbenv->txn_checkpoint(dbenv, kbyte, min, flags);
    }

    int log_archive(char*** listp, u_int32_t flags) {
        return dbenv->log_archive(dbenv, listp, flags);
    }

    int dbremove(DbTxn* txn, const char* file, const char* database, u_int32_t flags) {
        DB_TXN* t = txn ? txn->get_DB_TXN() : NULL;
        return dbenv->dbremove(dbenv, t, file, database, flags);
    }

    int set_flags(u_int32_t flags, int onoff) {
        return dbenv->set_flags(dbenv, flags, onoff);
    }

    int lsn_reset(const char* file, u_int32_t flags) {
        return dbenv->lsn_reset(dbenv, file, flags);
    }

    DB_ENV* get_DB_ENV() { return dbenv; }
    operator DB_ENV*() { return dbenv; }
};

class Db
{
private:
    DB* db;
    DbEnv* env;
public:
    Db(DbEnv* dbenv, u_int32_t flags) : db(NULL), env(dbenv) {
        int ret = db_create(&db, dbenv ? dbenv->get_DB_ENV() : NULL, flags);
        if (ret != 0)
            throw DbException(ret);
    }

    ~Db() {
        if (db) {
            db->close(db, 0);
        }
    }

    int open(DbTxn* txnid, const char* file, const char* database,
             DBTYPE type, u_int32_t flags, int mode) {
        DB_TXN* txn = txnid ? txnid->get_DB_TXN() : NULL;
        return db->open(db, txn, file, database, type, flags, mode);
    }

    int close(u_int32_t flags) {
        if (db) {
            int ret = db->close(db, flags);
            db = NULL;
            return ret;
        }
        return 0;
    }

    int get(DbTxn* txnid, Dbt* key, Dbt* data, u_int32_t flags) {
        DB_TXN* txn = txnid ? txnid->get_DB_TXN() : NULL;
        return db->get(db, txn, key->get_DBT(), data->get_DBT(), flags);
    }

    int put(DbTxn* txnid, Dbt* key, Dbt* data, u_int32_t flags) {
        DB_TXN* txn = txnid ? txnid->get_DB_TXN() : NULL;
        return db->put(db, txn, key->get_DBT(), data->get_DBT(), flags);
    }

    int del(DbTxn* txnid, Dbt* key, u_int32_t flags) {
        DB_TXN* txn = txnid ? txnid->get_DB_TXN() : NULL;
        return db->del(db, txn, key->get_DBT(), flags);
    }

    int exists(DbTxn* txnid, Dbt* key, u_int32_t flags) {
        DB_TXN* txn = txnid ? txnid->get_DB_TXN() : NULL;
        return db->exists(db, txn, key->get_DBT(), flags);
    }

    int cursor(DbTxn* txnid, Dbc** cursorp, u_int32_t flags) {
        DB_TXN* txn = txnid ? txnid->get_DB_TXN() : NULL;
        DBC* dbc = NULL;
        int ret = db->cursor(db, txn, &dbc, flags);
        if (ret == 0 && dbc) {
            *cursorp = new Dbc(dbc);
        }
        return ret;
    }

    int set_flags(u_int32_t flags) {
        return db->set_flags(db, flags);
    }

    DB* get_DB() { return db; }
    operator DB*() { return db; }
};

#endif // DB_CXX_COMPAT_H
