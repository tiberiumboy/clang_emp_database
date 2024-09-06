#include "database.h"

void host_to_network_database(struct database_info_t *dbhdr) {
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(dbhdr->filesize);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->version = htons(dbhdr->version);
}

void network_to_host_database(struct database_info_t *dbhdr) {
    dbhdr->magic = ntohl(dbhdr->magic);
    dbhdr->filesize = ntohl(dbhdr->filesize);
    dbhdr->count = ntohs(dbhdr->count);
    dbhdr->version = ntohs(dbhdr->version);
}

database_status create_database(char *filepath, struct database_t **databaseOut) {
    int fd = open(filepath, O_RDONLY);
    if( fd != -1 ) {
        close(fd);
        return DB_EXIST;
    }

    fd = open(filepath, O_RDWR | O_CREAT, PERMISSION);
    if ( fd == -1)
    {
        perror("open");
        return DB_CORRUPTED;
    }
    
    struct database_t *database = malloc(sizeof(struct database_t));
    struct database_info_t *header = malloc(sizeof(struct database_info_t));

    if (database == NULL || header == NULL ) {
        return DB_MALLOC;
    }

    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct database_info_t);

    lseek(fd, 0, SEEK_SET);
    write(fd, header, sizeof(struct database_info_t));

    database->fd = fd;
    database->dbhdr = header;
    *databaseOut = database;
    return DB_SUCCESS;
}

database_status open_database(char *filepath, struct database_t **databaseOut)  {
    int fd = open(filepath, O_RDWR, PERMISSION);
    if ( fd == -1 ) {
        close(fd);
        perror("open");
        return DB_BADFD;
    }

    struct database_t *database = malloc(sizeof(struct database_t));
    database->fd = fd;
    *databaseOut = database;
    return validate_database( database );
} 

database_status close_database(struct database_t *self) {
    if ( &self->fd == NULL ){
        return DB_SUCCESS;
    }

    // wonder if I have any memory leaks?
    free(self->dbhdr);
    close(self->fd);
    return DB_SUCCESS;
}

database_status save_database(struct database_t *self, struct employee_t *employees) {
    unsigned short count = self->dbhdr->count;
    printf("%d\n", count);
    self->dbhdr->filesize = sizeof(struct database_info_t) + sizeof(struct employee_t) * count;

    host_to_network_database(self->dbhdr);
    lseek(self->fd, 0, SEEK_SET);
    write(self->fd, self->dbhdr, sizeof(struct database_info_t));
    network_to_host_database(self->dbhdr);
    
    size_t struct_size = sizeof(struct employee_t);
    for(int i = 0; i < count; i++) {
        printf("%i/%d\n", i, count);
        employees[i].hours = htonl(employees[i].hours);
        write(self->fd, &employees[i], struct_size);
        employees[i].hours = ntohl(employees[i].hours);
    }

    return DB_SUCCESS;
}

database_status validate_database(struct database_t *self) {
    struct database_info_t *dbhdr = malloc(sizeof(struct database_info_t));
    if( dbhdr == NULL ) {
        return DB_MALLOC;
    }

    if (read(self->fd, dbhdr, sizeof(struct database_info_t)) != sizeof(struct database_info_t)) {
        perror("read");
        return DB_READFAIL;
    }

    network_to_host_database(dbhdr);

    if (dbhdr->magic != HEADER_MAGIC) {
        free(dbhdr);
        return DB_INVALIDDATA;
    }

    if (dbhdr->version != 1) {
        free(dbhdr);
        return DB_INVALIDDATA;
    }
    
    struct stat dbstat = {0};
    fstat(self->fd, &dbstat);
    if (dbhdr->filesize != dbstat.st_size) {
        return DB_CORRUPTED;
    }

    self->dbhdr = dbhdr;
    return DB_SUCCESS;
}

database_status read_database(struct database_t *self, struct employee_t **employeesOut) {
    unsigned long count = self->dbhdr->count;
    size_t emp_size = sizeof(struct employee_t) * count;
    if( read(self->fd, employeesOut, emp_size) != emp_size ) {
        perror("read");
        return DB_READFAIL;
    }
    
    for (int i = 0; i<count; i++ ) {
        employeesOut[i]->hours = ntohl(employeesOut[i]->hours);
    }

    return DB_SUCCESS;
}

void print_database(struct database_t *self) {
    if ( self == NULL ) {
        printf("Database is not initialized!");
        return;
    }
    printf("Content of Database:\n\tfd:%d\n", self->fd);
    print_info(self->dbhdr);
}

void print_info(struct database_info_t *info) {
    // segment fault here?
    if ( &info == NULL ) {
        printf("Database info is not initialized!");
        return;
    }
    printf("\tinfo:\n\t\tcount:%d\n", info->count);
}