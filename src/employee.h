#ifndef EMPLOYEE_H
#define EMPLOYEE_H
#define EMP_T struct employee_t
#define EMP_SIZE sizeof(EMP_T)

#include "dblib.h"

typedef enum {
    EMP_SUCCESS = 0,    
    EMP_BADFD,     // user provided bad fd signature.
    EMP_MALLOC,    // unable to malloc - out of space!
} employee_status;

struct employee_t {
    char name[24];
    char address[32];
    uint16_t hours;
};

employee_status parse_employee(char *addstr, EMP_T **employeeOut);
employee_status set_hours(EMP_T *, uint16_t newHour);
void list_employees(uint16_t count, EMP_T *employees);

#endif