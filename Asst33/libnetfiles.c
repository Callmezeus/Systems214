#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "libnetfiles.h"

typedef struct {
    char hostname[64];
    CONNECTION_MODE fMode;
} SERVER_CONN;

SERVER_CONN clientConn;


int getSock( const char * hostname ){
    int sockfd = 0;

    struct sockaddr_in serv_addr;
    struct hostent *server = NULL;

/**********************************Create New Socket**********************************/   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr,"libnetfiles: socket() failed, errno= %d\n", errno);
		return FAIL;
    }
  
   
    server = gethostbyname(hostname);													          //Get address of server inputed name
    if (server == NULL) {
       	errno = 0;
        h_errno = HOST_NOT_FOUND;
        printf("stderr,libnetfiles: host not found, h_errno");
		return FAIL;
    }

/**********************************Initialize Address**********************************/  
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(PORT);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    {
        fprintf(stderr,"libnetfiles: cannot connect to %s, h_errno= %d\n", hostname, h_errno);
    return FAIL;
    }

    return sockfd;
}
/**********************************************************************************************************************************************************
**                                                              NET_SERVERINIT                                                                           **
** netserverinit makes a socket and connection given a hostname. The Server will respond with SUCCESS (1) No Errors From Server Side                     **
** Client Errors; HOST_NOT_FOUND and INVALID_FILE_MODE                                                                                                   **
**********************************************************************************************************************************************************/

int netserverinit(char *hostname, int filemode){
    int n = 0;
    int sockfd = -1;
    char buffer[BUFFER_SIZE] = "";

    errno =  0;		
    h_errno = 0;


    strcpy(clientConn.hostname, buffer);													
	clientConn.fMode = INVALID_FILE;

    switch (filemode) {
        case UNRESTRICTED:
        case EXCLUSIVE:   
        case TRANSACTION:   
            break;

        default:
            h_errno = INVALID_FILE;
            fprintf(stderr, "netserverinit: invalid file connection mode\n");
            return FAIL;
            break;
    }


    if (hostname == NULL || strlen(hostname) == 0) {
        errno = EINVAL;  
        return FAIL;
    }

/**********************************Get a socket to connect to file SERVER_CONN************************************/
    
    sockfd = getSock(hostname);
    if (sockfd < 0 ) {
        errno = 0;
       	h_errno = HOST_NOT_FOUND;
        return FAIL;
    }
    

   
/***********Compose NET Command to send to the clientConn.  The format is:   netCmd,0,0,0 **/
   
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "%d,0,0,0", 1);

    //printf("netserverinit: send to clientConn - \"%s\"\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if ( n < 0 ) {

       	h_errno = ECOMM;  
        printf("Failed to write to SERVER_CONN");
        return FAIL;
    }

    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE-1);                                                    // Reads result, *(int *) 
	//printf("Ends up here%d\n", n);
    if ( n < 0 ) {
        h_errno = ECOMM;  
        printf("Failed to read from socket\n");
        if ( sockfd != 0 ) close(sockfd);
        return FAIL;
    }

    close(sockfd);  
    																			
    sscanf(buffer, "%d,", &n);
    if (n == 0){                                                                                // If read successfull, save the hostname 
        strcpy(clientConn.hostname, hostname);
        clientConn.fMode = (CONNECTION_MODE)filemode;
    }

    return n;
}


int netopen(const char *pathname, int flags)
{
    int netFd  = -1;
    int sockfd = -1;
    int n     = 0;
    char buffer[BUFFER_SIZE] = "";

    errno = 0;
    h_errno = 0;

    if (pathname == NULL || strlen(pathname)<1) {
        printf("Pathname is Invalid\n");
        errno = EINVAL;  
        return FAIL;
    } 

    if(flags != O_RDONLY && flags != O_WRONLY && flags != O_RDWR){
	printf("Invalid flags argument\n");
	errno = EINVAL;
	return FAIL;
    }

    //printf("netopen: pathname= %s, flags= %d\n", pathname, flags);

  /* if ( isNetSERVER_CONNInitialized( NET_OPEN ) != TRUE ) {
        errno = EPERM;  // 1 = Operation not permitted
        return FAIL;
    }*/

    sockfd = getSock(clientConn.hostname);
    if (sockfd < 0) {
        errno = 0;
        h_errno = HOST_NOT_FOUND;
        fprintf(stderr, "netopen: host not found, %s\n", clientConn.hostname);
        return FAIL;
    }
                                                                                                // netCmd,connectionMode,fileOpenFlags,pathname
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "%d,%d,%d,%s", NET_OPEN, clientConn.fMode, flags, pathname);


    //printf("netopen: send to SERVER_CONN - \"%s\"\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if ( n < 0 ) {
        fprintf(stderr, "netopen: failed to write cmd to clientConn.  n= %d\n", n);
        return FAIL;
    }
    
                                                                                                    
    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE-1);                                                         // The response buffer format is result,errno,h_errno,netFd
    if ( n < 0 ) {
        if ( sockfd != 0 ) close(sockfd);
        return FAIL;
    }

    close(sockfd);  

    //printf("netopen: received from SERVER_CONN - \"%s\"\n", buffer);


    
    sscanf(buffer, "%d,%d,%d,%d", &n, &netFd, &errno, &h_errno);                                    // Get the response from the SERVER_CONN
    if ( n == FAIL ) {
        printf("netopen: SERVER_CONN returns FAILURE, errno= %d (%s), h_errno=%d\n", errno, strerror(errno), h_errno);
        return FAIL;
    }

    return netFd;
}

/**********************************************************************************************************************************************************
**                                                              NET_CLOSE                                                                                **
** The client exectuting netclose will create and socket and connect to server and a file descriptor. The server will close a file and return 0 on       **
** bytes actually read, or returns -1 if failed and sets errno to indicate Errors                                                                        **
** ERRORS : ETIMEDOUT, EBADF, ECONNRESET                                                                                                                 **
**********************************************************************************************************************************************************/

extern int netclose(int fd){
    
    if(fd >= 0){
        errno = EBADF;
        return FAILURE;
    }
    int netFd  = -1;
    int sockfd = -1;
    int n     = 0;
    char buffer[BUFFER_SIZE] = "";

    errno = 0;
    h_errno = 0;

    sockfd = getSock(clientConn.hostname);
    if (sockfd < 0) {
        errno = 0;
        h_errno = HOST_NOT_FOUND;
        fprintf(stderr, "netopen: host not found, %s\n", clientConn.hostname);
        return FAIL;
    }
                                                                                                // netCmd,connectionMode,fileOpenFlags,pathname
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "%d,%d", NET_CLOSE, fd);

    n = write(sockfd, buffer, strlen(buffer));

    if ( n < 0 ) {

        h_errno = ECOMM;  
        printf("Failed to write to SERVER_CONN");
        return FAIL;
    }

    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE-1);                                                    // Reads result, *(int *) 
    //printf("Ends up here%d\n", n);
    if ( n < 0 ) {
        h_errno = ECOMM;  
        printf("Failed to read from socket\n");
        if ( sockfd != 0 ) close(sockfd);
        return FAIL;
    }

    close(sockfd); 

    sscanf(buffer, "%d,%d,%d", &n, &errno, &h_errno);  

    return n; 

    
}
extern ssize_t netread(int fildes, void *buf, size_t nbyte){
    int netFd  = -1;
    int sockfd = -1;
    int n     = 0;
    char * stringBuf = (char *)buf;
    char buffer[BUFFER_SIZE];
    errno = 0;
    h_errno = 0;
    //printf("Makes it here netread\n");
    if(fildes >= 0){
        errno = EBADF;
        return FAILURE;
    }

     errno = 0;
    h_errno = 0;

    sockfd = getSock(clientConn.hostname);
    if (sockfd < 0) {
        errno = 0;
        h_errno = HOST_NOT_FOUND;
        fprintf(stderr, "netopen: host not found, %s\n", clientConn.hostname);
        return FAIL;
    }
                                                                                                // netCmd,connectionMode,fileOpenFlags,pathname
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "%u,%d,%d", NET_READ, fildes, (int)nbyte);

    n = write(sockfd, buffer, strlen(buffer));

    if ( n < 0 ) {

        h_errno = ECOMM;  
        printf("Failed to write to SERVER_CONN");
        return FAIL;
    }

    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE-1);                                                    // Reads result, *(int *) 
    //printf("Ends up here%d\n", n);
    //printf("buffer: %s\n", buffer);
    if ( n < 0 ) {
        h_errno = ECOMM;  
        printf("Failed to read from socket\n");
        if ( sockfd != 0 ) close(sockfd);
        return FAIL;
    }

    close(sockfd); 

    sscanf(buffer, "%d,%d,%d", &n, &netFd, &h_errno);  
    strncpy(stringBuf, buffer+strlen(buffer)-nbyte, nbyte);
    //printf("%d %d %d %s\n", n, netFd, h_errno, stringBuf);
    if(n == FAIL) return FAIL;
    //else printf("String from Read : %s\n", stringBuf);


    return netFd; 
    
}

extern ssize_t netwrite(int fildes, const void *buf, size_t nbyte){

    int netFd  = -1;
    int sockfd = -1;
    int n     = 0;
    char * stringBuf = (char *)buf;
    char buffer[BUFFER_SIZE];
    errno = 0;
    h_errno = 0;
    //printf("Makes it here netwrite\n");
    if(fildes >= 0){
        errno = EBADF;
        return FAILURE;
    }

     errno = 0;
     h_errno = 0;

    sockfd = getSock(clientConn.hostname);
    if (sockfd < 0) {
        errno = 0;
        h_errno = HOST_NOT_FOUND;
        fprintf(stderr, "netopen: host not found, %s\n", clientConn.hostname);
        return FAIL;
    }
                                                                                                // netCmd,connectionMode,fileOpenFlags,pathname
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "%u,%d,%d,%d,%s", NET_WRITE, fildes, (int)nbyte, strlen(stringBuf), stringBuf);

    n = write(sockfd, buffer, strlen(buffer));

    if ( n < 0 ) {

        h_errno = ECOMM;  
        printf("Failed to write to SERVER_CONN");
        return FAIL;
    }

    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE-1);                                                    // Reads result, *(int *) 
    //printf("Ends up here%d\n", n);
    if ( n < 0 ) {
        h_errno = ECOMM;  
        printf("Failed to read from socket\n");
        if ( sockfd != 0 ) close(sockfd);
        return FAIL;
    }

    close(sockfd); 

    sscanf(buffer, "%d,%d,%d", &n, &netFd, &errno);  
    if(n == FAIL) return FAIL;
   
    return netFd; 

}
