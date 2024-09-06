#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "dblib.h"
#include "database.h"

typedef enum {
    EMP_SUCCESS = 0,    
    EMP_BADFD,     // user provided bad fd signature.
    EMP_MALLOC,    // unable to malloc - out of space!
} employee_status;

struct employee_t {
    char name[55];
    char address[55];
    unsigned int hours;
};

employee_status parse_employee(char *addstr, struct database_t *database, struct employee_t **employees);
void list_employees(int count, struct employee_t *employees);

#endif