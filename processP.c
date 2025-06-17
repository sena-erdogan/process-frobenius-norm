#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <math.h>

#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_TRUNC | O_CREAT | O_EXCL)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)

extern char **environ;

char **charbuffer;
int *temp;

int **buffer;
int size;

/*int main(int argc, char *argv[]){

	char **ep;
	
	for(ep = environ; *ep != NULL; ep++)	puts(*ep);
	
	exit(EXIT_SUCCESS);

}*/

void readInput(int fromfd){

	char *buf;
	
	int count = 0;
	int i;
	int bytesread;
	
	for(i=0; i<30*50; i++)	temp[i] = -1;
	
	for( ; ; ){
	
		buf=(char*)malloc(30);
	
		while(((bytesread = read(fromfd, buf, 30)) == -1) && (errno == EINTR)) ;
		if(bytesread <= 0)	return;
			
		if(strlen(buf) < 30)	break;
		
		for(i=0; i<30; i++){
			temp[(count*30)+i] = buf[i];
		}
		
		count++;
		
		free(buf);
	}
	
	i=0;
	
	size = 0;
	int j;
	
	while(temp[i] != -1){
		size++;
		i++;
	}
	
	buffer = (int**)malloc(sizeof(int*)*(size/30));
	charbuffer = (char**)malloc(sizeof(char*)*(size/30));
	
	for(i=0; i<size/30; i++){
		buffer[i] = (int*)malloc(sizeof(int)*30);
		for(j=0; j<30; j++)	buffer[i][j] = temp[(i*30)+j];
	}
}

void frobenius(char* to){ /* children will call this */

	float res = 0, i, j;
	int num, t;
	int tofd;
	int byteswritten = 0;
	
	for(i=0; i<size/30; i++){	
		for(j=0; j<30; j++){
			t = (i*30)+j;
			num = *(temp+t);
			res += pow(num, 2);
		}
		
		res = sqrt(res);
		
		if((tofd = open(to, WRITE_FLAGS, WRITE_PERMS)) == -1){
			perror("\nFailed to open output file for writing\n");
			return;
		}
		
		struct flock lock;
	
		memset(&lock, 0, sizeof(lock));
		lock.l_type = F_RDLCK;
		fcntl(tofd, F_SETLKW, &lock);

		while(((byteswritten = write(tofd, "Process R_", strlen("Process R_"))) == -1) && (errno == EINTR));
		if(byteswritten < 0)
			break;
		if(byteswritten == -1)
			break;
		
		while(((byteswritten = write(tofd, i, sizeof(int))) == -1) && (errno == EINTR));
		if(byteswritten < 0)
			break;
		if(byteswritten == -1)
			break;
			
		while(((byteswritten = write(tofd, " ", 1)) == -1) && (errno == EINTR));
		if(byteswritten < 0)
			break;
		if(byteswritten == -1)
			break;
			
		while(((byteswritten = write(tofd, res, sizeof(float))) == -1) && (errno == EINTR));
		if(byteswritten < 0)
			break;
		if(byteswritten == -1)
			break;
			
		while(((byteswritten = write(tofd, "\n", 1)) == -1) && (errno == EINTR));
		if(byteswritten < 0)
			break;
		if(byteswritten == -1)
			break;
		
		lock.l_type = F_UNLCK;
		fcntl(tofd, F_SETLKW, &lock);
		
		if(close(tofd) == -1){
			perror("\nError closing output file\n");
			return;
		}
	}
}

int main(int argc, char *argv[]){

	int fromfd, tofd;
	char* from;
	char* to;
	
	if(argc != 5){
		perror("\nUsage: Have a process read an input file, interpret its contents as coordinates, and forward them for calculations to children processes\n");
		return 1;
	}
	
	if(strcmp(argv[1], "-i") == 0){
		from = (char*)malloc(strlen(argv[2]));
		strcpy(from, argv[2]);
		if(strcmp(argv[3], "-o") == 0){
			to = (char*)malloc(strlen(argv[4]));
			strcpy(to, argv[4]);
		}else{
			perror("\nInvalid command (input file path must be after '-i' and output file path must be after '-o')\n");
			return 1;
		}
	}
	else if(strcmp(argv[1], "-o") == 0){
		to = (char*)malloc(strlen(argv[2]));
		strcpy(to, argv[2]);
		if(strcmp(argv[3], "-i") == 0){
			from = (char*)malloc(strlen(argv[4]));
			strcpy(from, argv[4]);
		}else{
			perror("\nInvalid command (input file path must be after '-i' and output file path must be after '-o')\n");
			return 1;
		}
	}else{
		perror("\nInvalid command (input file path must be after '-i' and output file path must be after '-o')\n");
		return 1;
	}
	
	if((fromfd = open(from, READ_FLAGS)) == -1){
		perror("\nFailed to open input file\n");
		return 1;
	}
	
	temp=(int *)calloc(30 * 50, sizeof(int));
	
	readInput(fromfd);
	
	if(close(fromfd) == -1){
		perror("\nFailed to close input file\n");
		return;
	}else fprintf(stdout, "\ninput file closed successfully\n");
	
	fprintf(stdout, "the main program process ID is %d\n", (int) getpid());
	
	int numDead;
	pid_t childPid;
	int i, j, k;
	
	frobenius(to);
	
	for(i=0; i<size/30; i++){
		switch(fork()){
			case -1:
				perror("\nfork unsuccessful\n");
				return 1;
			case 0:
				perror("\nfork successful: \n");
				fprintf(stdout, "\nchild pid: %d\n", getpid());
				fprintf(stdout, "\nparent pid: %d\n", getppid());
				
				fprintf(stdout, "Created R_%d with ", i+1);
				
				for(j=0; j<30; j += 3){
					fprintf(stdout, "(%d,", buffer[i][j]);
					fprintf(stdout, " %d,", buffer[i][j+1]);
					fprintf(stdout, " %d) ", buffer[i][j+1]);
				}
				
				fprintf(stdout, "\n");
				
				return 1;
			deafult:	break;
		}
	}
	
	float *arr;
	float *t;
	i=0;
	int bytesread=0;
	
	if((tofd = open(to, READ_FLAGS)) == -1){
		perror("\nFailed to open output file for reading\n");
		return 1;
	}
	
	for( ; ; ){
	
		t=(float*)malloc(sizeof(float));
		arr=(float*)malloc(sizeof(float)*size);
	
		while(((bytesread = read(tofd, t, sizeof(float))) == -1) && (errno == EINTR)) ;
		if(bytesread <= 0)	return;
		
		arr[i] = *t;
		i++;
		
		free(t);
	}
	
	float min = arr[0];
	int min_i = 0;
	
	for(i=0; i<size/30; i++){
		if(arr[i] < min){
			min = arr[i];
			min_i = i;
		}
	}
	
	fprintf(stdout, "The smallest distance is %f with processR_%d\n", min, min_i);
	
	numDead = 0;
	
	for( ; ; ){
		childPid = wait(NULL);
		if(childPid == -1){
			if(errno == ECHILD){
				fprintf(stdout, "\nall children terminated successfully\n");
				return 1;
			}else{
				perror("\nunexpected error\n");
				return 1;
			}
		}
		numDead++;
		fprintf(stdout, "wait() returned child PID %ld (numDead = %d)\n", (long) childPid, numDead);
	}
	
	free(temp);
	
	return 0;
}
