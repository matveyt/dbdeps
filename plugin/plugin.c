/*
 * Proj: dbdeps
 * Auth: MatveyT
 * Desc: GNU Make plugin to read in dependency database
 * Note: None
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wrapdb.h"


#define PLUGIN dbdeps
#define _CONCAT(a, b) _CONCAT2(a, b)
#define _CONCAT2(a, b) a##b
#define _STRINGIZE(a) _STRINGIZE2(a)
#define _STRINGIZE2(a) #a


/* GNU Make Loaded Object API */

typedef struct {
    const char* filenm;
    unsigned long lineno;
} gmk_floc;

typedef char* (*gmk_func_ptr)(const char*, unsigned, char**);

int plugin_is_GPL_compatible;
static char* (*gmk_alloc)(unsigned);
static void (*gmk_free)(char*);
static void (*gmk_eval)(const char*, const gmk_floc*);
static char* (*gmk_expand)(const char*);
static void (*gmk_add_function)(const char*, gmk_func_ptr,
    unsigned, unsigned, unsigned);


/* growable buffer */
struct buffer {
    char* buf;
    size_t bufsiz;
};


/* grow buffer if necessary */
/* note: the buffer's content is not preserved */
static int grow_buffer(struct buffer* pbuf, size_t newsize)
{
    if (pbuf->bufsiz < newsize) {
        pbuf->bufsiz = ((newsize + 255) / 256) * 256;
        free(pbuf->buf);
        pbuf->buf = malloc(pbuf->bufsiz);
        if (!pbuf->buf)
            return 1;
    }
    return 0;
}


/* The only exported function */
static char* PLUGIN(const char* name, unsigned argc, char** argv)
{
    /* unused */
    (void)name;
    (void)argc;

    /* open database */
    WRAP_DB* db;
    if (wrap_db_open(&db, *argv, 1, 0))
        return NULL;

    /* file/location structure for Make diagnostic messages */
    gmk_floc floc = { _STRINGIZE(PLUGIN), 1 };

    /* eval all key-value pairs in the database */
    void *key, *data;
    size_t ksize, dsize;
    struct buffer buf = { NULL, 0 };
    while (!wrap_db_get(db, &key, &ksize, &data, &dsize)) {
        if (!grow_buffer(&buf, ksize + dsize + 2)) {
            snprintf(buf.buf, buf.bufsiz, "%.*s:%.*s", (int)ksize, (char*)key,
                (int)dsize, (char*)data);
            gmk_eval(buf.buf, &floc);
            ++floc.lineno;
        }
    }
    free(buf.buf);

    /* close database */
    wrap_db_close(db);
    return NULL;
}


/* Plugin entry point */
int _CONCAT(PLUGIN, _gmk_setup)(void)
{
#if _WIN32
    extern void* __stdcall GetProcAddress(void*, const char*);
    gmk_alloc = GetProcAddress(NULL, "gmk_alloc");
    gmk_free = GetProcAddress(NULL, "gmk_free");
    gmk_eval = GetProcAddress(NULL, "gmk_eval");
    gmk_expand = GetProcAddress(NULL, "gmk_expand");
    gmk_add_function = GetProcAddress(NULL, "gmk_add_function");
#else /* TODO: test */
    #include <dlfcn.h>
    void* handle = dlopen(NULL, RTLD_LAZY);
    gmk_alloc = dlsym(handle, "gmk_alloc");
    gmk_free = dlsym(handle, "gmk_free");
    gmk_eval = dlsym(handle, "gmk_eval");
    gmk_expand = dlsym(handle, "gmk_expand");
    gmk_add_function = dlsym(handle, "gmk_add_function");
#endif /* _WIN32 */

    if (!gmk_alloc || !gmk_free || !gmk_eval || !gmk_expand ||
        !gmk_add_function)
        return 0;

    gmk_add_function(_STRINGIZE(PLUGIN), PLUGIN, 1, 1, 0);
    return 1;
}
