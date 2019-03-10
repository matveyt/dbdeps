/*
 * Proj: dbdeps
 * Auth: MatveyT
 * Desc: Imports compiler generated .d files into the internal database
 * Note: To be preprocessed with re2c
 *
 * $Id$
 */


#include <direct.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "wrapdb.h"


/* constant data */
static const char PROGRAM[] = "updbdeps";
static const char AUTHOR[] = "Matvey Tarasov";
static const char VERSION[] = "0.2";
static const unsigned YEAR = 2019;
static const char DEFAULT_DIR[] = ".deps";


/* own non-compliant getline()/getdelim() implementation */
extern ssize_t my_getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
#define my_getline(buf, bufsiz, fp) my_getdelim(buf, bufsiz, '\n', fp)


/* print usage summary */
static void print_usage(int verbose)
{
    printf("Usage: %s [OPTION]... FILE...\n", PROGRAM);
    if (verbose)
        printf("Update dependency database.\n\n"
            "-d, --dir=DIR\tstore database files in DIR\n"
            "-r, --remove\tremove input files\n"
            "-z, --size=NUM\tset maximum db size in 4K pages (`m' for megabytes)\n"
            "-h, --help\tprint this help\n"
            "-V, --version\tdisplay version number\n\n"
            "Default directory is `%s'.\n", DEFAULT_DIR);
    else
        printf("\nTry `%s --help' for more information.\n", PROGRAM);
}


/* print version number and copyright info */
static void print_version(void)
{
    printf("%s %s\n"
        "Copyright (C) %u %s.\n"
        "This is free software.  You may redistribute copies of it under the terms of\n"
        "the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n"
        "There is NO WARRANTY, to the extent permitted by law.\n\n"
        "Written by %s.\n", PROGRAM, VERSION, YEAR, AUTHOR, AUTHOR);
}


/* process command-line arguments */
static void process_args(int argc, char* argv[], const char** dir, int* keep,
    size_t* dbsize)
{
    while (1) {
        static const struct option long_options[] = {
            { "dir", required_argument, NULL, 'd' },
            { "remove", no_argument, NULL, 'r' },
            { "size", required_argument, NULL, 'z' },
            { "help", no_argument, NULL, 'h' },
            { "version", no_argument, NULL, 'V' },
            { NULL, 0, NULL, 0 }
        };

        /* get next option */
        int c = getopt_long(argc, argv, "d:rz:hV", long_options, NULL);
        if (c == -1)
            break;

        /* process option */
        switch (c) {
        ssize_t newsize;
        char* suffix;

        case 'd':   /* output */
            *dir = optarg;
            break;
        case 'r':   /* remove */
            *keep = 0;
            break;
        case 'z':   /* size */
            newsize = strtol(optarg, &suffix, 10);
            if (newsize > 0) {
                *dbsize = newsize * 4096;
                if (*suffix == 'm' || *suffix == 'M')
                    *dbsize *= 256;
            }
            break;
        case 'h':   /* help */
            print_usage(1);
            exit(1);
        case 'V':   /* version */
            print_version();
            exit(1);
        case 0:     /* flag option */
        case '?':   /* unknown option */
            break;
        default:    /* should never get here */
            abort();
        }
    }
}


/* check if db operation successful */
static void check_db_result(int err)
{
    if (err) {
        fprintf(stderr, "%s: db error(%d) -- %s\n", PROGRAM, err,
            wrap_db_strerror(err));
        exit(1);
    }
}


/* read in dependency file and store all data in the database */
static void process_file(WRAP_DB* db, FILE* fp)
{
    char* buf = NULL;
    size_t bufsiz = 0;
    ssize_t len;

    while ((len = my_getline(&buf, &bufsiz, fp)) > 0) {
        /* ignore short lines */
        if (len < 3)
            continue;

        /* re2c variables */
        const char* YYCURSOR = buf;
        const char* YYMARKER;
        const char* prereq;
        /*!stags:re2c format = 'const char* @@;'; */

        /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:flags:tags = 1;

        print = [\x20-\x7E];
        (print+) ":" @prereq ( print+)? "\n" {
            /* throw trailing ':' and '\n' */
            ptrdiff_t pos = prereq - buf;
            if (wrap_db_put(db, buf, pos - 1, (void*)prereq, len - pos - 1))
                break;
            continue;
        }

        * { continue; }
        */
    }
    free(buf);
}


int main(int argc, char* argv[])
{
    WRAP_DB* db;
    const char* dir = DEFAULT_DIR;
    int keep = 1;
    size_t dbsize = 0; /* use default */

    /* parse command line */
    process_args(argc, argv, &dir, &keep, &dbsize);
    if (optind >= argc) {
        fprintf(stderr, "%s: no input files\n", PROGRAM);
        print_usage(0);
        exit(1);
    }

    /* open database */
    _mkdir(dir);
    check_db_result(wrap_db_open(&db, dir, 0, dbsize));

    /* process file arguments */
    for (; optind < argc; ++optind) {
        FILE* fp = fopen(argv[optind], "r");
        if (fp) {
            process_file(db, fp);
            fclose(fp);
            /* remove input file from disk */
            if (!keep)
                remove(argv[optind]);
        }
    }

    /* close database */
    check_db_result(wrap_db_close(db));
}
