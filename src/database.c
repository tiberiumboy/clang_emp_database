#include "database.h"

// private
void host_to_network_info(struct database_info_t *info) {
    info->magic = htonl(info->magic);
    info->filesize = htonl(info->filesize);
    info->count = htons(info->count);
    info->version = htons(info->version);
}

void network_to_host_info(struct database_info_t *info) {
    info->magic = ntohl(info->magic);
    info->filesize = ntohl(info->filesize);
    info->count = ntohs(info->count);
    info->version = ntohs(info->version);
}

void update_file_size(struct database_t *self ) {
    size_t info_size = sizeof(struct database_info_t);
    size_t emp_size = sizeof(struct employee_t);
    self->info->filesize = info_size + emp_size * self->info->count;
}

void print_info(struct database_info_t *info) {
    // segment fault here?
    if ( &info == NULL ) {
        printf("Database info is not initialized!");
        return;
    }
    printf("\tinfo:\n\t\tcount:%d\n", info->count);
}

void __save(int fd, struct database_info_t *info) {
    // convert to network eudian
    host_to_network_info(info);

    // write to file.
    lseek(fd, 0, SEEK_SET);
    write(fd, info, sizeof(struct database_info_t));

    // convert back to host eudian
    network_to_host_info(info);
}

database_status validate_database(int fd, struct database_t *self) {   
    size_t info_size = sizeof(struct database_info_t); 
    struct database_info_t *info = malloc(info_size);
    if( info == NULL ) {
        return DB_MALLOC;
    }

    if (read(fd, info, info_size) != info_size) {
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

database_status create_database(char *filepath, struct database_t **databaseOut) {
    int fd = open(filepath, O_RDWR | O_CREAT, PERMISSION);
    if ( fd == -1 )
    {
        perror("open");
        return DB_CORRUPTED;
    }
    
    struct database_t *database = malloc(sizeof(struct database_t));
    struct database_info_t *info = malloc(sizeof(struct database_info_t));

    if (database == NULL || info == NULL ) {
        return DB_MALLOC;
    }

    // initial values
    info->version = 0x1;
    info->count = 0;
    info->magic = HEADER_MAGIC;
    info->filesize = sizeof(struct database_info_t);

    __save(fd, info);

    // assign pointers
    database->fd = fd;
    database->info = info;
    *databaseOut = database;

    return DB_SUCCESS;
}

database_status read_database(struct database_t *self, struct employee_t **employeesOut) {
    uint16_t count = self->info->count;
    size_t emp_size = sizeof(struct employee_t) * count;
    
    if( read(self->fd, employeesOut, emp_size) != emp_size ) {
        printf("Failed to read via read database method\n");
        perror("read");
        return DB_READFAIL;
    }
    
    for (int i = 0; i<count; i++ ) {
        employeesOut[i]->hours = ntohl(employeesOut[i]->hours);
    }

    return DB_SUCCESS;
}

// public 

database_status open_database(char *filepath, struct database_t **databaseOut, struct employee_t **employeesOut)  {
    int fd = open(filepath, O_RDWR, PERMISSION);
    if ( fd == -1 ) {
        close(fd);
        printf("File do not exist. Creating a new database instead...\n");
        return create_database(filepath, databaseOut);
    }

    struct database_t *database = malloc(sizeof(struct database_t));
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
    
    switch( read_database(database, employeesOut) ) {
        case EMP_BADFD:
            printf("Bad file descriptor provided by user!\n");
            return DB_BADFD;
        case EMP_MALLOC: 
            printf("Unable to allocate memory for employees - ran out of space!\n");
            return DB_MALLOC; 
    }

    return result;
} 

database_status save_database(struct database_t *self, struct employee_t* employees) {
    if( self == NULL || self->info == NULL ) {
        return DB_READFAIL;
    }

    update_file_size(self);
    unsigned short count = self->info->count;
    __save(self->fd, self->info);
    
    size_t struct_size = sizeof(struct employee_t);
    for(int i = 0; i < count; i++) {
        employees[i].hours = htonl(employees[i].hours);
        write(self->fd, &employees[i], struct_size);
        employees[i].hours = ntohl(employees[i].hours);
    }

    return DB_SUCCESS;
}

database_status close_database(struct database_t *self) {
    if ( &self->fd == NULL ){
        return DB_SUCCESS;
    }

    // try to run valgrind?
    free(self->info);
    close(self->fd);
    free(self); // wonder if this is a good idea? should this responsibility be above?
    return DB_SUCCESS;
}

database_status add_employee(char *addstr, struct database_t *database, struct employee_t **employees) {
    struct employee_t *emp = {0};
    switch( parse_employee(addstr, &emp) ) {
        case EMP_MALLOC:
            return DB_MALLOC;
        case EMP_SUCCESS: 
            unsigned int count = database->info->count + 1; 
            employees = realloc(employees, count * sizeof(struct employee_t));  
            *employees[count-1] = *emp;
            break;
    } 

    return DB_SUCCESS;
}

void print_database(struct database_t *self) {
    if ( self == NULL ) {
        printf("Database is not initialized!");
        return;
    }
    printf("Content of Database:\n\tfd:%d\n", self->fd);

    if ( self->info == NULL ) {
        printf("database header is null?\n");
        return;
    }
    print_info(self->info);
}