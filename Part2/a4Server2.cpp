#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <signal.h>

using namespace std;

const int MAXLINE = 4096; /* why make it a define and waste namespace*/
const int LISTENQ = 8; /* this looks better too */

// I am using popen discussed in APUE to get the output of system command. 
string runCommand(char s[]) {
	FILE* result;	
	result=popen(s, "r");
	string ans;
	char buffer[MAXLINE];
	while (result && !feof(result))  
		if (fgets(buffer, MAXLINE, result))
			ans += buffer;
		else break;
	pclose(result);
	return ans;
}

static void alarm_handler(int sig) {
	printf("Time is up!!!\n");
	exit(0);
}


int main (int argc, char **argv)
{
	auto action = (struct sigaction){.sa_handler = alarm_handler};
	sigaction(SIGALRM, &action, NULL);
	alarm(300);

	int listenfd, connfd, n;
	pid_t childpid;
	socklen_t clilen;
	char buf[MAXLINE];
	struct sockaddr_in cliaddr, servaddr;

	//Test if the correct number of arguments was given
	if (argc !=2) {
		perror("Usage: program_name <port_number>");
		exit(1);
	}

	//Create a socket for the socket
	//If sockfd<0 there was an error in the creation of the socket
	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Problem in creating the socket");
		exit(2);
	}

	//preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons((int) strtol(argv[1], (char**) NULL, 10));

	//bind the socket
	bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	//listen to the socket by creating a connection queue, then wait for clients
	listen (listenfd, LISTENQ);

	printf("%s\n","Server running...waiting for connections.");

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		//accept a connection

		connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
		printf("%s\n","Received request...");

		if ( (childpid = fork ()) == 0 ) {	//if it’s 0, it’s child process
			printf ("%s\n","Child created for dealing with client requests");
			close (listenfd);

			while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {
				printf("String %s received from Client\n", buf);
				for (int i =0; i<MAXLINE; i++)
					if (buf[i] == '\n')
						buf[i]='\0';
				if (string(buf).size()==1) {
					printf("Ignore request because command is empty");
				} else {
					string result = runCommand(buf);
					send(connfd, result.c_str(), result.size()*sizeof(char), 0);
				}
			}
			if (n < 0)
				printf("%s\n", "Read error");
			close(connfd);
			exit(0);
		}
		//close socket of the server
		close(connfd);
	}
}

