#ifndef DATABASE_H
#define DATABASE_H

#define HEADER_MAGIC 0x4c4c4144
#define PERMISSION 0644 
#define DB_T struct database_t
#define INFO_T struct database_info_t
#define INFO_SIZE sizeof(INFO_T)
#define DB_SIZE sizeof(DB_T)

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
    DB_INVALIDINPUT,
    DB_CONNECTED,
    DB_CORRUPTED,
    DB_EXIST,
    DB_NOTFOUND,
} database_status;

struct database_info_t {
    uint32_t magic;
    uint16_t version;
    uint16_t count;
    size_t filesize;
};

DB_T {
    int fd;
    INFO_T *info;
};

bool valid_connection(DB_T *db);
database_status open_database(char *filepath, DB_T **databaseOut, EMP_T **employeesOut);
database_status save_database(DB_T *, EMP_T *employees);

/// @brief Close and free database connection (This will close file descriptor and free database_info_t obj)
/// @param  target database to close
/// @return database status : SUCCESS if pass
database_status close_database(DB_T *);

/// @brief Parse and append new employee from string.
/// @param addstr comma splice data containing "name,address,hours" entry
/// @param database target database pointer - to update employee table count
/// @param employees target employees collection - to append newly parsed employees
/// @return database status: SUCCESS if new entry added | potential failure code: DB_MALLOC
database_status add_employee(char *addstr, DB_T *database, EMP_T **employees);

/// @brief Remove target employee that matches name filter
/// @param filter the name of the employee to remove entry from
/// @param database target database pointer
/// @param employees target employees collection to remove from
/// @return database status - SUCCESS if pass
database_status remove_employee(char *filter, DB_T *database, EMP_T **employees);

/// @brief Update employee
/// @param newstr New updated information to replace current entry 
/// @param database Target database to affect to
/// @param employees Collection of employees to make changes to
/// @return Status : DB_SUCCESS returns | potential errors: DB_MALLOC, DB_NOTFOUND
database_status update_employee(char *newstr, DB_T *database, EMP_T **employees);

#endif