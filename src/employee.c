#include <string.h>
#include "employee.h"
#include "database.h"

// private

void host_to_network_employee(struct employee_t *employee ) {
    employee->hours = htonl(employee->hours);
}

void network_to_host_employee(struct employee_t *employee) {
    employee->hours = ntohl(employee->hours);
}

// public

void list_employees(uint16_t count, struct employee_t *employees) {
    for(uint16_t i = 0; i<count; i++) {
        printf("Employee %d\n", i);
        printf("\tName: %s\n", employees[i].name);
        printf("\tAddr: %s\n", employees[i].address);
        printf("\tHour: %d\n", employees[i].hours); 
    }
}

employee_status parse_employee(char *addstr, struct employee_t **employeeOut) {
    struct employee_t *emp = malloc(sizeof(struct employee_t));
    if( emp == NULL ) {
        return EMP_MALLOC;
    }
    
    // This code seems unsafe and may potentially break without some safeguard to protect comma separate format.
    // internally strtok tracks the previous pointer position - pass in NULL to use that same reference.
    // this field never going to change.
    static char *delim = ",";
    char *name = strtok(addstr, delim);
    char *addr = strtok(NULL, delim);
    char *hours = strtok(NULL, delim);

    strncpy(emp->name, name, sizeof(emp->name));
    strncpy(emp->address, addr, sizeof(emp->address));
    emp->hours = atoi(hours);

    *employeeOut = emp;
    return EMP_SUCCESS;
}

employee_status set_hours(struct employee_t *self, uint16_t newHour) {
    self->hours = newHour;
    return EMP_SUCCESS;    
}