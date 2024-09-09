#include "dblib.h"
#include "database.h"
#include "employee.h"

int check_connection( DB_T *db) {
    if( !valid_connection(db) ) {
        printf("Please connect to database first!\n");
        return EXIT_FAILURE;
    } 
    
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    int c;
    struct database_t *database = NULL;
    struct employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:r:u:l")) != -1 ) {
        switch( c ) {
            case 'f':
                switch( open_database(optarg, &database, &employees) ) {
                    case DB_BADFD:
                        break;
                    case DB_MALLOC:
                        printf("Cannot allocate memory for the file! Ran out of space!\n");
                        return EXIT_FAILURE;
                    case DB_READFAIL:
                        printf("Fail to read database file!\n");
                        return EXIT_FAILURE;
                    case DB_INVALIDDATA:
                        printf("Failed to validate database header\n");
                        return EXIT_FAILURE;
                    case DB_CORRUPTED:
                        printf("Validation failed: File size mismatch! The data may have been corrupted!\n");
                        return EXIT_FAILURE;
                    case DB_INVALIDINPUT:
                        printf("Invalid arguments! Please provide the database path! after -f\n");
                        return EXIT_FAILURE;
                    case DB_CONNECTED:
                        printf("Database connection is already established! Ignoring -f arguments\n");
                        break;
                }
                break;

            case 'n':
                // wink wink ;D
                printf("[SUCCESS] Database created!\n");
                return EXIT_SUCCESS;

            case 'a':
                if( check_connection(database) == EXIT_FAILURE ) {
                    return EXIT_FAILURE;
                }

                if ( optarg == NULL ) {
                    printf("Invalid arguments! Please provide \"name,address,hours\" after -a\n");
                    return EXIT_FAILURE;
                }

                struct employee_t *employee = NULL;
                if( add_employee(optarg, database, &employees) == DB_MALLOC ) {
                    printf("[FAILED] Unable to allocate new space!\n");
                    break;
                }

                if( save_database(database, employees) == DB_BADFD ) {
                    printf("[FAILURE] Got a bad FD from user!\n");
                    break;
                }

                printf("[SUCCESS] Employee added\n");
                break;
            case 'l' :
                if( check_connection(database) == EXIT_FAILURE ) {
                    return EXIT_FAILURE;
                }

                list_employees(database->info->count, employees);
                close_database(database);
                return EXIT_SUCCESS;
            case 'r' :
                if( check_connection(database) == EXIT_FAILURE ) {
                    return EXIT_FAILURE;
                }

                if ( optarg == NULL ) {
                    printf("Invalid arguments! Please provide \"name\" after -r\n");
                    return EXIT_FAILURE;
                }   

                if( remove_employee(optarg, database, &employees) == DB_SUCCESS ) {
                    save_database(database, employees);
                    printf("[SUCCESS] Employee removed!\n");
                } else {
                    printf("[FAILED] Unable to remove employee!\n");
                }

                break;
            case 'u' :
                if( check_connection(database) == EXIT_FAILURE ) {
                    return EXIT_FAILURE;
                }

                if( optarg == NULL ) {
                    printf("Invalid arguments! Please provide \"name,address,hours\" after -u\n");
                    return EXIT_FAILURE;
                }

                switch( update_employee(optarg, database, &employees) ) {
                    case DB_SUCCESS:
                        save_database(database, employees);
                        printf("[SUCCESS] Updated one employee!");
                        break;
                    case DB_NOTFOUND:
                        printf("[FAILED] Could not find target employee to update to!\n");
                        return EXIT_FAILURE;
                    case DB_MALLOC:
                        printf("[FAILED] Unable to allocate memory for employee!\n");
                        return EXIT_FAILURE;
                }

                break;
            case '?':
                printf("Unknown option - %c\n", c);
                return EXIT_SUCCESS;
            default:
                printf("default was called... should not happen\n");
                return EXIT_SUCCESS;
        }
    }

    if ( database != NULL ) {
        close_database(database);
    }

    if ( employees != NULL ) {
        free(employees);
    }
    
    return EXIT_SUCCESS;
}