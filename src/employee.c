#include <string.h>
#include "employee.h"
#include "database.h"

// private

void host_to_network_employee(EMP_T *employee ) {
    employee->hours = htonl(employee->hours);
}

void network_to_host_employee(EMP_T *employee) {
    employee->hours = ntohl(employee->hours);
}

// public

void list_employees(uint16_t count, EMP_T *employees) {
    if ( count == 0 ) {
        printf("[INFO] Database collection is empty!\n");
        return;
    }

    printf("[INFO] Employees list:\n");
    for(uint16_t i = 0; i<count; i++) {
        // what if this is the culprit?
        printf("Employee name:%s\n", employees[i].name);
        printf("\tAddr: %s\n", employees[i].address);
        printf("\tHour: %d\n", employees[i].hours); 
    }
}

employee_status parse_employee(char *addstr, EMP_T **employeeOut) {
    EMP_T *emp = malloc(EMP_SIZE);
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
