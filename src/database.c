#include "database.h"

// private

void host_to_network_info(INFO_T *info) {
    info->magic = htonl(info->magic);
    info->filesize = htonl(info->filesize);
    info->count = htons(info->count);
    info->version = htons(info->version);
}

void network_to_host_info(INFO_T *info) {
    info->magic = ntohl(info->magic);
    info->filesize = ntohl(info->filesize);
    info->count = ntohs(info->count);
    info->version = ntohs(info->version);
}

void __save(int fd, INFO_T *info) {
    // convert to network eudian
    info->filesize = INFO_SIZE + EMP_SIZE * info->count;

    // if we have more data in the file, truncate to the appropriate size.
    if( ftruncate(fd, info->filesize) != 0 ) {
        // something happen here
        perror("ftruncate");
        return;
    }

    host_to_network_info(info);

    // write to file.
    lseek(fd, 0, SEEK_SET);
    write(fd, info, INFO_SIZE);

    // convert back to host eudian
    network_to_host_info(info);
}

database_status validate_database(int fd, DB_T *self) {   
    struct database_info_t *info = malloc(INFO_SIZE);
    if( info == NULL ) {
        return DB_MALLOC;
    }

    if (read(fd, info, INFO_SIZE) != INFO_SIZE) {
        printf("Failed to read via validate database method\n");
        perror("read");
        return DB_READFAIL;
    }
    
    network_to_host_info(info);
    
    if (info->magic != HEADER_MAGIC || info->version != 0x1) {
        return DB_INVALIDDATA;
    }
    
    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (info->filesize != dbstat.st_size) {
        return DB_CORRUPTED;
    }
    
    self->fd = fd;
    self->info = info;

    return DB_SUCCESS;
}

database_status create_database(char *filepath, DB_T **databaseOut) {
    int fd = open(filepath, O_RDWR | O_CREAT, PERMISSION);
    if ( fd == -1 )
    {
        perror("open");
        return DB_CORRUPTED;
    }
    
    DB_T *database = malloc(DB_SIZE);
    INFO_T *info = malloc(INFO_SIZE);

    if (database == NULL || info == NULL ) {
        return DB_MALLOC;
    }

    // initial values
    info->version = 0x1;
    info->count = 0;
    info->magic = HEADER_MAGIC;
    info->filesize = 0; // this information will be populated in the __save() command below

    // save data
    __save(fd, info);

    // assign pointers
    database->fd = fd;
    database->info = info;
    *databaseOut = database;

    return DB_SUCCESS;
}

database_status read_database(DB_T *self, EMP_T **employeesOut) {
    uint16_t count = self->info->count;
    size_t emp_size = EMP_SIZE * count;
    EMP_T *emp = malloc(count * emp_size);

    if( read(self->fd, emp, emp_size) != emp_size ) {
        perror("read");
        return DB_READFAIL;
    }
    
    for (int i = 0; i<count; i++ ) {
        emp[i].hours = ntohs(emp[i].hours);
    }
    
    *employeesOut = emp;

    return DB_SUCCESS;
}

// public

bool valid_connection(DB_T *db) {
    if ( db == NULL || db->info == NULL ) {
        return false;
    }
    return true;
}

database_status open_database(char *filepath, DB_T **databaseOut, EMP_T **employeesOut)  {
    if ( filepath == NULL ) {
        return DB_INVALIDINPUT;
    }

    if (  *databaseOut != NULL ) {
        return DB_CONNECTED;
    }
    
    int fd = open(filepath, O_RDWR, PERMISSION);
    if ( fd == -1 ) {
        close(fd);
        printf("[WARN] File do not exist.\n");
        *employeesOut = calloc(0, EMP_SIZE);
        return create_database(filepath, databaseOut);
    }

    DB_T *database = malloc(DB_SIZE);
    if( database == NULL ) {
        close(fd);
        return DB_MALLOC;
    }

    database_status result = validate_database( fd, database );
    switch ( result ) {
        case DB_CORRUPTED:
        case DB_INVALIDDATA:
        case DB_READFAIL:
            free(database->info);
        case DB_MALLOC:
            close(fd);
            return result;
        case DB_SUCCESS:
            *databaseOut = database;
            break;
    }
    
    if( read_database(database, employeesOut) == DB_READFAIL ) {
        printf("[ERROR] Unable to read database\n");
        result = DB_CORRUPTED;
    }

    return result;
} 

database_status save_database(DB_T *self, EMP_T* employees) {
    unsigned short count = self->info->count;
    __save(self->fd, self->info);
    
    size_t emp_size = EMP_SIZE;
    for(int i = 0; i < count; i++) {
        employees[i].hours = htons(employees[i].hours);
        write(self->fd, &employees[i], emp_size);
        employees[i].hours = ntohs(employees[i].hours);
    }

    return DB_SUCCESS;
}

database_status close_database(DB_T *self) {
    if ( &self->fd == NULL ){
        return DB_SUCCESS;
    }

    // try to run valgrind?
    free(self->info);
    close(self->fd);
    free(self); // wonder if this is a good idea? should this responsibility be above?
    return DB_SUCCESS;
}

database_status add_employee(char *addstr, DB_T *database, EMP_T **employees) {
    EMP_T *emp = NULL;

    if( parse_employee(addstr, &emp) == EMP_MALLOC ) {
        return DB_MALLOC;
    }

    uint16_t count = database->info->count + 1;
    uint16_t total_size = count * EMP_SIZE;
    EMP_T *new_ptr = NULL;  
     
    new_ptr = realloc(*employees, total_size);
    if( new_ptr == NULL ) {
        perror("realloc");
        return DB_MALLOC; 
    }  
    new_ptr[count - 1] = *emp;
    *employees = new_ptr;
    database->info->count = count;
    return DB_SUCCESS;
}

database_status remove_employee(char *filter, DB_T *database, EMP_T **employees) {
    uint16_t count = database->info->count - 1;
    uint16_t i = 0;
    
    for(;i<=count;i++) {
        if( strcmp( (*employees + i)->name, filter ) == 0 ) 
        {
            if( i < count ) { 
                // swap target to last element of array ( Last array element will be truncated )
                (*employees)[i] = (*employees)[count];
            }

            EMP_T *ptr = NULL;
            ptr = realloc(*employees, count * EMP_SIZE);
            if ( ptr == NULL ) {
                perror("realloc");
                return DB_MALLOC;
            }

            database->info->count = count;
            *employees = ptr;
            return DB_SUCCESS;
        }
    }

    return DB_NOTFOUND;
}

database_status update_employee(char *newstr, DB_T *database, EMP_T **employees) {
    EMP_T *target = NULL;
    if( parse_employee(newstr, &target) == EMP_MALLOC ) {
        return DB_MALLOC;
    }
    
    uint16_t count = database->info->count;
    uint16_t i = 0;
    database_status status = DB_NOTFOUND;
    
    for(;i<count;i++) {
        if(strcmp((*employees + i)->name, target->name) == 0 ) {
            (*employees)[i] = *target;
            status = DB_SUCCESS;
            break;
        }
    }
    
    return status;
}
