// I created this based on the echoClient.c you provided in week 14

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h> 
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <signal.h>

using namespace std;

const int MAXLINE = 4096;

// From my project 2 part 1
string parsetime(time_t &t) {
	struct tm local;
	localtime_r(&t, &local);
	char timestring[200];
	strftime(timestring, sizeof(timestring), "%c", &local);
	return string(timestring);
}

static void alarm_handler(int sig) {
	printf("Time is up!!!\n");
	exit(0);
}

int main(int argc, char **argv) 
{
	auto action = (struct sigaction){.sa_handler = alarm_handler};
	sigaction(SIGALRM, &action, NULL);
	alarm(300);

	FILE *infile, *outfile;
	int sockfd;
	struct sockaddr_in servaddr;
	char recvline[MAXLINE];

	//basic check of the arguments
	//additional checks can be inserted
	if (argc != 5) {
		perror("Usage: program_name <server_ip> <port_number> <file_name> <outputfile_name>"); 
		exit(1);
	}

	//Create a socket for the client
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Problem in creating the socket");
		exit(2);
	}

	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(argv[1]);
	servaddr.sin_port =  htons((int) strtol(argv[2], (char**) NULL, 10));

	//Connection of the client to the socket 
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
		perror("Problem in connecting to the server");
		exit(3);
	} 
	printf("Connected to the server\n");
	infile = fopen(argv[3], "r");
	outfile = fopen(argv[4], "w");
	if (infile == NULL || outfile == NULL) {
		perror("Cannot open input or output file");
		exit(4);
	}

	char *sendline = NULL;
	size_t len = 0;
	ssize_t bytes;
	while ((bytes = getline(&sendline, &len, infile)) != -1) {
		time_t t;
		time(&t);
		char line[MAXLINE];
		memset(line, '\0', sizeof(char)*MAXLINE); // null terminate character
		strcpy(line, sendline);
		for (int i = 0; i < MAXLINE; i++)
			if (line[i]=='\n')
				line[i]='\0';
		if (string(line)=="end") break;

		fprintf(outfile, " ** Client %d at %s sent %s \n", getpid(), parsetime(t).c_str(), line);

		send(sockfd, line, MAXLINE, 0);

		memset(recvline, '\0', sizeof(char)*MAXLINE);
		if (recv(sockfd, recvline, MAXLINE,0) == 0){
			perror("The server terminated prematurely"); 
			exit(4);
		}

		fprintf(outfile, " ** Client %d at %s received:\n%s \n", getpid(), parsetime(t).c_str(), recvline);
	}
	printf("Processing completed, attempting to close socket and file\n");
	free(sendline);
	fclose(infile);
	fclose(outfile);
	close(sockfd);

	exit(0);
}

