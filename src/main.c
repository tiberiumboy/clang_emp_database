#include "dblib.h"
#include "database.h"
#include "employee.h"

// TODO: add functionality to remove employee by name using -r flag
// TODO: update employee's hour using -u

int main(int argc, char **argv) {
    int c;
    char *filepath = NULL;
    struct database_t *database = NULL;
    struct employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:lr:")) != -1 ) {
        switch( c ) {
            case 'f':
                filepath = optarg;
                if ( filepath == NULL ) {
                    printf("Invalid arguments! Please provide the database path! after -f\n");
                    return -1;
                }

                if ( database != NULL ) {
                    printf("Database connection is already established! Ignoring -f arguments");
                    break;
                }

                switch( open_database(filepath, &database, &employees) ) {
                    case DB_BADFD:
                        break;
                    case DB_MALLOC:
                        printf("Cannot allocate memory for the file! Ran out of space!\n");
                        return -1;
                    case DB_READFAIL:
                        printf("Fail to read database file!\n");
                        return -1;
                    case DB_INVALIDDATA:
                        printf("Failed to validate database header\n");
                        return -1;
                    case DB_CORRUPTED:
                        printf("Validation failed: File size mismatch! The data may have been corrupted!\n");
                        return -1;
                }
                break;

            case 'n':
                // wink wink ;)
                printf("Database created!\n");
                return 0;

            case 'a':
                if ( database == NULL || database->info == NULL ) {
                    printf("Please connect to database first!\n");
                    return -1;
                }
                
                if ( optarg == NULL ) {
                    printf("Invalid arguments! Please provide \"name,address,hours\" after -a\n");
                    return -1;
                }

                struct employee_t *employee = NULL;
                if( add_employee(optarg, database, &employees) == DB_MALLOC ) {
                    printf("Unable to allocate new space!\n");
                    break;
                }

                printf("Successfully added employee\n");

                if( save_database(database, employees) == DB_BADFD ) {
                    printf("Got a bad FD from user!\n");
                }

                printf("Successfully save\n");
                
                break;
            case 'l' :
                if ( database == NULL || database->info == NULL ) {
                    printf("Please connect to database first!\n");
                    return -1;
                }
                printf("Listing employees\n");
                list_employees(database->info->count, employees);
                close_database(database);
                return 0;
            case 'r' :
                if  ( database == NULL || database->info == NULL ) {
                    printf("Please connect to database first!\n");
                    return -1;
                }
            case '?':
                printf("Unknown option - %c\n", c);
                return 0;
            default:
                printf("default was called... should not happen");
                return 0;
        }
    }

    if ( database != NULL ) {
        close_database(database);
    }
    
    printf("Exit program\n");
    return 0;
}