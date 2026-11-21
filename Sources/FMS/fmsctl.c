#define _XOPEN_SOURCE 700
#define _GNU_SOURCE
#include <ftw.h>
#include <grp.h>
#include <limits.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static sqlite3 *index_database;
static sqlite3_stmt *insert_statement;
static unsigned long long indexed_files;
static int indexing_failed;

static const char *database_path(void) {
    const char *override = getenv("MINTOSH_FMS_DB");
    return override && override[0] == '/' ? override : "/var/lib/mintosh/fms.db";
}

static int index_entry(const char *path, const struct stat *metadata,
                       int type, struct FTW *walk) {
    (void)walk;
    if (type == FTW_DNR || type == FTW_NS) {
        indexing_failed = 1;
        return 1;
    }
    if (type != FTW_F || !S_ISREG(metadata->st_mode)) return 0;
    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;
    sqlite3_bind_text(insert_statement, 1, path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_statement, 2, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(insert_statement, 3, (sqlite3_int64)metadata->st_size);
    sqlite3_bind_int64(insert_statement, 4, (sqlite3_int64)metadata->st_mtime);
    sqlite3_bind_int64(insert_statement, 5, (sqlite3_int64)metadata->st_mode);
    sqlite3_bind_int64(insert_statement, 6, (sqlite3_int64)metadata->st_uid);
    sqlite3_bind_int64(insert_statement, 7, (sqlite3_int64)metadata->st_gid);
    int result = sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);
    sqlite3_clear_bindings(insert_statement);
    if (result != SQLITE_DONE) {
        indexing_failed = 1;
        return 1;
    }
    ++indexed_files;
    return 0;
}

static int index_root(const char *root) {
    char canonical[PATH_MAX];
    if (!realpath(root, canonical)) { perror("fmsctl: resolve root"); return 1; }
    struct stat metadata;
    if (stat(canonical, &metadata) < 0 || !S_ISDIR(metadata.st_mode)) {
        fputs("fmsctl: index root must be an existing directory\n", stderr);
        return 64;
    }
    const char *destination = database_path();
    char temporary[PATH_MAX];
    if (snprintf(temporary, sizeof temporary, "%s.new", destination) >= (int)sizeof temporary) {
        fputs("fmsctl: database path is too long\n", stderr);
        return 64;
    }
    unlink(temporary);
    if (sqlite3_open_v2(temporary, &index_database,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
        fprintf(stderr, "fmsctl: create index: %s\n",
                index_database ? sqlite3_errmsg(index_database) : "out of memory");
        if (index_database) sqlite3_close(index_database);
        index_database = NULL;
        return 1;
    }
    (void)sqlite3_exec(index_database, "PRAGMA journal_mode=DELETE", NULL, NULL, NULL);
    const char *schema =
        "CREATE TABLE files("
        "path TEXT PRIMARY KEY,name TEXT NOT NULL,size INTEGER NOT NULL,"
        "mtime INTEGER NOT NULL,mode INTEGER NOT NULL,uid INTEGER NOT NULL,gid INTEGER NOT NULL);"
        "CREATE INDEX files_name_idx ON files(name);";
    if (sqlite3_exec(index_database, schema, NULL, NULL, NULL) != SQLITE_OK ||
        sqlite3_exec(index_database, "BEGIN IMMEDIATE", NULL, NULL, NULL) != SQLITE_OK ||
        sqlite3_prepare_v2(index_database,
            "INSERT INTO files(path,name,size,mtime,mode,uid,gid) VALUES(?1,?2,?3,?4,?5,?6,?7)",
            -1, &insert_statement, NULL) != SQLITE_OK) {
        fprintf(stderr, "fmsctl: initialize index: %s\n", sqlite3_errmsg(index_database));
        sqlite3_close(index_database);
        index_database = NULL;
        unlink(temporary);
        return 1;
    }

    indexed_files = 0;
    indexing_failed = 0;
    int walk_result = nftw(canonical, index_entry, 32, FTW_PHYS | FTW_MOUNT);
    sqlite3_finalize(insert_statement);
    insert_statement = NULL;
    if (walk_result != 0 || indexing_failed ||
        sqlite3_exec(index_database, "COMMIT", NULL, NULL, NULL) != SQLITE_OK) {
        (void)sqlite3_exec(index_database, "ROLLBACK", NULL, NULL, NULL);
        fprintf(stderr, "fmsctl: indexing failed: %s\n", sqlite3_errmsg(index_database));
        sqlite3_close(index_database);
        index_database = NULL;
        unlink(temporary);
        return 1;
    }
    if (sqlite3_close(index_database) != SQLITE_OK) {
        index_database = NULL;
        unlink(temporary);
        fputs("fmsctl: could not close index database\n", stderr);
        return 1;
    }
    index_database = NULL;
    if (chmod(temporary, 0640) < 0) {
        perror("fmsctl: protect index");
        unlink(temporary);
        return 1;
    }
    if (geteuid() == 0) {
        struct group *group = getgrnam("mintosh-services");
        if (chown(temporary, 0, group ? group->gr_gid : 0) < 0) {
            perror("fmsctl: set index ownership");
            unlink(temporary);
            return 1;
        }
    }
    if (rename(temporary, destination) < 0) {
        perror("fmsctl: publish index");
        unlink(temporary);
        return 1;
    }
    printf("Indexed %llu files from %s\n", indexed_files, canonical);
    return 0;
}

static void json_string(const char *value) {
    putchar('"');
    for (const unsigned char *cursor = (const unsigned char *)value; *cursor; ++cursor) {
        if (*cursor == '"' || *cursor == '\\') { putchar('\\'); putchar(*cursor); }
        else if (*cursor < 0x20) printf("\\u%04x", *cursor);
        else putchar(*cursor);
    }
    putchar('"');
}

static int search_files(const char *query, int json) {
    if (!query || !query[0] || strlen(query) > 256) {
        fputs("fmsctl: search query must contain 1 to 256 bytes\n", stderr);
        return 64;
    }
    sqlite3 *database = NULL;
    if (sqlite3_open_v2(database_path(), &database, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        fprintf(stderr, "fmsctl: open index: %s\n",
                database ? sqlite3_errmsg(database) : "out of memory");
        if (database) sqlite3_close(database);
        return 1;
    }
    sqlite3_stmt *statement = NULL;
    const char *sql =
        "SELECT path,size,mtime,mode FROM files "
        "WHERE instr(lower(name),lower(?1))>0 OR instr(lower(path),lower(?1))>0 "
        "ORDER BY path LIMIT 200";
    if (sqlite3_prepare_v2(database, sql, -1, &statement, NULL) != SQLITE_OK) {
        fprintf(stderr, "fmsctl: prepare search: %s\n", sqlite3_errmsg(database));
        sqlite3_close(database);
        return 1;
    }
    sqlite3_bind_text(statement, 1, query, -1, SQLITE_TRANSIENT);
    if (json) putchar('[');
    int first = 1, count = 0, result;
    while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
        const char *path = (const char *)sqlite3_column_text(statement, 0);
        sqlite3_int64 size = sqlite3_column_int64(statement, 1);
        sqlite3_int64 modified = sqlite3_column_int64(statement, 2);
        sqlite3_int64 mode = sqlite3_column_int64(statement, 3);
        if (json) {
            if (!first) putchar(',');
            fputs("{\"path\":", stdout);
            json_string(path ? path : "");
            printf(",\"size\":%lld,\"modified\":%lld,\"mode\":%lld}",
                   (long long)size, (long long)modified, (long long)mode);
        } else {
            printf("%s\t%lld bytes\tmtime=%lld\n", path ? path : "",
                   (long long)size, (long long)modified);
        }
        first = 0;
        ++count;
    }
    if (json) puts("]");
    if (result != SQLITE_DONE) {
        fprintf(stderr, "fmsctl: search failed: %s\n", sqlite3_errmsg(database));
        sqlite3_finalize(statement);
        sqlite3_close(database);
        return 1;
    }
    sqlite3_finalize(statement);
    sqlite3_close(database);
    if (!json && count == 0) puts("No matching files.");
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Mintosh File Metadata Search 0.1.0");
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "index") == 0) return index_root(argv[2]);
    if (argc == 3 && strcmp(argv[1], "search") == 0) return search_files(argv[2], 0);
    if (argc == 4 && strcmp(argv[1], "search") == 0 &&
        strcmp(argv[3], "--json") == 0) return search_files(argv[2], 1);
    fputs("usage: fmsctl index ROOT | search QUERY [--json]\n", stderr);
    return 64;
}
