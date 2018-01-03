#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "libnetfiles.h"


int main(){
	
	int run = 1;
	int i = 0;
	int j = 0;
	int k = 0;
	int a = 0;

	while(run){
		
		char* buffer = malloc(500);
		
		printf("Enter a function type:\n");
		scanf("%d", &i);
		printf("You entered: %d\n", i);
		
		switch(i){
			
			case NET_SERVERINIT:
				printf("Enter hostname and connection mode:\n");
				scanf("%s %d", buffer, &j);
				printf("You entered: %s %d\n", buffer, j);
				a = netserverinit(buffer, j);
				printf("netserverinit returns: %d\n", a);
				break;
			
			case NET_OPEN:
				printf("Enter file pathname and access flags:\n");
				scanf("%s %d", buffer, &j);
				printf("You entered: %s %d\n", buffer, j);
				if(j==1){
					a = netopen(buffer, O_RDONLY);
					printf("netopen returns: %d\n", a);
					break;
				}
				if(j==2){
					a = netopen(buffer, O_WRONLY);
					printf("netopen returns: %d\n", a);
					break;
				}
				if(j==3){
					a = netopen(buffer, O_RDWR);
					printf("netopen returns: %d\n", a);
					break;
				}
				
			case NET_READ:
				printf("Enter file descriptor and number of bytes:\n");
				scanf("%d %d", &j, &k);
				printf("You entered: %d %d\n", j, k);
				a = netread(j, buffer, k);
				printf("netread returns: %d\n", a);
				printf("buffer: %s\n", buffer);
				break;
				
			case NET_WRITE:
				printf("Enter file descriptor, buffer, and number of bytes:\n");
				scanf("%d %s %d", &j, buffer, &k);
				printf("You entered: %d %s %d\n", j, buffer, k);
				a = netwrite(j, buffer, k);
				printf("netwrite returns: %d\n", a);
				break;

			case NET_CLOSE:
				printf("Enter file descriptor:\n");
				scanf("%d", &j);
				printf("You entered: %d\n", j);
				a = netclose(j);
				printf("netclose returns: %d\n", a);
				break;

			default:
				printf("Invalid function type\n");
		}

		free(buffer);
		printf("Continue running? Enter 1 or 0:\n");
		scanf("%d", &run);
	}

	return 0;
}









