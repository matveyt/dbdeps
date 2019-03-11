/*
 * Proj: dbdeps
 * Auth: matveyt
 * Desc: Simple wrapper for Symas LMDB (https://symas.com/lmdb)
 * Note: None
 */


#ifndef _WRAPDB_H

typedef struct WRAP_DB WRAP_DB;

char* wrap_db_strerror(int err);

int wrap_db_open(WRAP_DB** db, const char* path, int rdonly, size_t dbsize);
int wrap_db_close(WRAP_DB* db);

int wrap_db_get(WRAP_DB* db, void** key, size_t* ksize, void** data, size_t* dsize);
int wrap_db_put(WRAP_DB* db, void* key, size_t ksize, void* data, size_t dsize);

#endif /* _WRAPDB_H */
