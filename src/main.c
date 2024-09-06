#include "dblib.h"
#include "employee.h"
#include "database.h"

// TODO: add functionality to remove employee by name using -r flag
// TODO: update employee's hour using -u

int handle_open_database(char *optarg, struct database_t **databaseOut, struct employee_t **employeesOut) {
    switch(open_database(optarg, databaseOut)) {
        case DB_BADFD:
            break;
        case DB_MALLOC:
            printf("Cannot allocate memory for the file! Ran out of space!\n");
            return -1;
        case DB_READFAIL:
            printf("Fail to read database file!\n");
            return -1;
        case DB_INVALIDDATA:
            printf(" Failed to validate database header\n");
            return -1;
        case DB_CORRUPTED:
            printf("File may be corrupted!\n");
            // here we can just simply create a new database file and live our lives normally.
            // but for now let's break out of this operation until we need to handle this specifically.
            return -1;
        case DB_SUCCESS: 
            switch( read_database(*databaseOut, employeesOut) ) {
                case EMP_BADFD:
                    printf("Bad file descriptor provided by user!\n");
                    return -1;
                case EMP_MALLOC: 
                    printf("Unable to allocate memory for employees - ran out of space!\n");
                    return -1; 
            }
        break;
    }

    return 0;
}

int handle_creating_employee( char *optarg, struct database_t *database, struct employee_t *employeesOut ) {
    struct employee_t *employee = NULL;
    if( parse_employee(optarg, database, &employee) == DB_MALLOC ) {
        printf("Unable to allocate new space!\n");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    int c;
    char *filepath = NULL;
    struct database_t *database = NULL;
    struct employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:l")) != -1 ) {
        switch( c ) {
            case 'f':
                filepath = optarg;
                if ( filepath == NULL ) {
                    printf("Invalid arguments! Please provide the database path! after -f\n");
                    return -1;
                }

                if ( database != NULL ) {
                    printf("Database connection is already established! Ignoring -f path");
                    return -1;
                }

                handle_open_database( filepath, &database, &employees );
                break;

            case 'n':
                if( database != NULL ) {
                    printf("Database exist! Ignoring -n flag...");
                    break;
                }

                if ( filepath == NULL ) {
                    printf("Missing database path!\n");
                    return -1;
                }

                if ( create_database( filepath, &database ) == DB_BADFD ) {
                    printf("User provided bad file path!\n");
                    return -1;
                }
                printf("Database created: count: %d\n", database->dbhdr->count);
                break;

            case 'a':
                if ( database == NULL ) {
                    printf("Please connect to database first!\n");
                }
                
                if ( optarg == NULL ) {
                    printf("Invalid arguments! Please provide \"name,address,hours\" after -a\n");
                    return -1;
                }

                printf("About to create employee\n");
                print_database(&database);
                handle_creating_employee(optarg, database, employees);
                break;
            case 'l' :
                list_employees(database->dbhdr->count, employees);
                break;
            case '?':
                printf("Unknown option - %c\n", c);
                break;
            default:
                printf("default was called... should not happen");
                return -1;
        }
    }
                
    if (database != NULL && &database->fd != NULL) {

        // this should be preconfigurable. Auto save? 
        if( save_database(database, employees) == DB_BADFD ) {
            printf("Got a bad FD from user!\n");
            return -1;
        }
        // close_database(database);
    }

    return 0;
}