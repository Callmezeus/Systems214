#ifndef 	_LIBNETFILES_H_
#define    	_LIBNETFILES_H_

#define PORT  8005
#define TRUE    1
#define FALSE   0
#define FAILURE -1
#define SUCCESS 0
#define FAIL -1
#define BUFFER_SIZE  500 


typedef enum {
    UNRESTRICTED = 1,
    EXCLUSIVE   = 2,
    TRANSACTION = 3,
    INVALID_FILE = 99
} CONNECTION_MODE;

typedef enum {
    NET_SERVERINIT = 1,
    NET_OPEN  = 2,
    NET_READ  = 3,
    NET_WRITE = 4,
    NET_CLOSE = 5,
    INVALID   = 99
} NET_FUNCTION_TYPE;


extern int netserverinit(char *hostname, int filemode);
extern int netopen(const char *pathname, int flags);
extern ssize_t netread(int fildes, void *buf, size_t nbyte); 
extern ssize_t netwrite(int fildes, const void *buf, size_t nbyte); 
extern int netclose(int fd);
int getSock(const char * hostname);


#endif    // _LIBNETFILES_H_

