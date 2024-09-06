#ifndef DATABASE_H
#define DATABASE_H

#define HEADER_MAGIC 0x4c4c4144
#define PERMISSION 0644 

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 
#include "dblib.h"
#include "employee.h"

typedef enum {
    DB_SUCCESS = 0,
    DB_BADFD,
    DB_MALLOC,
    DB_READFAIL,
    DB_INVALIDDATA,
    DB_CORRUPTED,
    DB_EXIST,
} database_status;

struct database_info_t {
    unsigned int magic;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
};

struct database_t {
    int fd;
    struct database_info_t *dbhdr;
};

database_status create_database(char *filepath, struct database_t **databaseOut);
database_status validate_database(struct database_t *);
database_status read_database(struct database_t *, struct employee_t **employeesOut);
database_status open_database(char *filepath, struct database_t **databaseOut);
database_status close_database(struct database_t *);
database_status save_database(struct database_t *, struct employee_t *employees);

void print_database(struct database_t *);

#endif