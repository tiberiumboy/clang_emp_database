#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "dblib.h"

typedef enum {
    EMP_SUCCESS = 0,    
    EMP_BADFD,     // user provided bad fd signature.
    EMP_MALLOC,    // unable to malloc - out of space!
} employee_status;

struct employee_t {
    char name[35];
    char address[35];
    unsigned int hours;
};

employee_status parse_employee(char *addstr, struct employee_t **employeeOut);
void list_employees(unsigned int count, struct employee_t *employees);

#endif