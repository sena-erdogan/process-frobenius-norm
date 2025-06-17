#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_TRUNC | O_CREAT | O_EXCL)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)

int main(int argc, char *argv[]){

	int tofd;
	char* to;
	to = (char*)malloc(strlen(argv[1]));
	strcpy(to, argv[1]);
	
	perror("\nhello\n");

	if((tofd = open(to, WRITE_FLAGS, WRITE_PERMS)) == -1){
		perror("\nFailed to create output file\n");
		return 1;
	}else	perror("\ncreated output file\n");
	
	struct flock lock;
	
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_RDLCK;
	fcntl(tofd, F_SETLKW, &lock);
	
	/*operation(argv[1], fromfd, argv[2]);*/
	
	lock.l_type = F_UNLCK;
	fcntl(tofd, F_SETLKW, &lock);
	
	if(close(tofd) == -1){
		perror("\nFailed to close output file\n");
		return;
	}else fprintf(stdout, "\noutput file closed successfully\n");

}
