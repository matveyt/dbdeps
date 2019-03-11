/*
 * Proj: dbdeps
 * Auth: matveyt
 * Desc: Simple wrapper for Symas LMDB (https://symas.com/lmdb)
 * Note: None
 */


#include <errno.h>
#include <stdlib.h>
#include "lmdb.h"
#include "wrapdb.h"


/* internal structure */
struct WRAP_DB {
    MDB_env* env;
    MDB_txn* txn;
    MDB_cursor* cursor;
    MDB_dbi dbi;
    unsigned flags;
};


char* wrap_db_strerror(int err)
{
    return mdb_strerror(err);
}


int wrap_db_open(WRAP_DB** db, const char* path, int rdonly, size_t dbsize)
{
    WRAP_DB* w = malloc(sizeof(WRAP_DB));
    if (!w)
        return ENOMEM;

    w->env = NULL;
    w->txn = NULL;
    w->cursor = NULL;
    w->flags = rdonly ? MDB_RDONLY : 0;

    int err = mdb_env_create(&w->env);
    if (!err && !rdonly && dbsize >= 16384) /* at least 16K */
        err = mdb_env_set_mapsize(w->env, dbsize);
    if (!err)
        err = mdb_env_open(w->env, path, w->flags, 0664);
    if (!err)
        err = mdb_txn_begin(w->env, NULL, w->flags, &w->txn);
    if (!err)
        err = mdb_dbi_open(w->txn, NULL, 0, &w->dbi);

    if (err) {
        if (w->txn)
            mdb_txn_abort(w->txn);
        if (w->env)
            mdb_env_close(w->env);
    } else {
        *db = w;
    }

    return err;
}


int wrap_db_close(WRAP_DB* db)
{
    int err = MDB_SUCCESS;

    if (db->cursor)
        mdb_cursor_close(db->cursor);

    if (db->flags & MDB_RDONLY) 
        mdb_txn_abort(db->txn);
    else
        err = mdb_txn_commit(db->txn);

    mdb_env_close(db->env);
    free(db);
    return err;
}


int wrap_db_get(WRAP_DB* db, void** key, size_t* ksize, void** data, size_t* dsize)
{
    MDB_val vkey, vdata;

    int err = db->cursor ? MDB_SUCCESS :
        mdb_cursor_open(db->txn, db->dbi, &db->cursor);
    if (!err)
        err = mdb_cursor_get(db->cursor, &vkey, &vdata, MDB_NEXT);
    if (!err) {
        *key = vkey.mv_data;
        *ksize = vkey.mv_size;
        *data = vdata.mv_data;
        *dsize = vdata.mv_size;
    }

    return err;
}


int wrap_db_put(WRAP_DB* db, void* key, size_t ksize, void* data, size_t dsize)
{
    return mdb_put(db->txn, db->dbi, &(MDB_val) { ksize, key },
        &(MDB_val) { dsize, data }, 0);
}
