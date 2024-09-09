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
    uint32_t magic;
    uint16_t version;
    uint16_t count;
    size_t filesize;
};

struct database_t {
    int fd;
    struct database_info_t *info;
};

database_status open_database(char *filepath, struct database_t **databaseOut, struct employee_t **employeesOut);
database_status save_database(struct database_t *, struct employee_t *employees);
database_status close_database(struct database_t *);
database_status add_employee(char *addstr, struct database_t *database, struct employee_t **employees);

void print_database(struct database_t *);

#endif