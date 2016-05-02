#include "sys/socket.h"	   //System Socket Library
#include "unistd.h"	   //Unix Standard
#include "sys/types.h"	   //System Data Types
#include "sys/wait.h"	   //Flags for waiting functions
#include "netinet/in.h"
#include "string.h"
#include "errno.h"
#include "netdb.h"
#include "fcntl.h"

#include "stdlib.h"
#include "stdio.h"

#define BUF_SIZE 500  //500 characters
#define NAME_SIZE 10  //10 Character Handle
#define EX_CHARS 2    //'>' and '\0' space

//Checks argc, argv
int checkArgs(int, char*[]);
//Gets user handle
int getHandle(char*);
//Connects to remote server
int connectToChatServe(char*, char*);
//Handshakes with server
int validateWithServer(int, char*, char*);
//Gets user input from stdin
char* getInput(char*,char*);
//Sends message through socket
void sendMsg(int, char*);
//Gets response from socket
void getResponse(FILE*, char*);

int main(int argc, char* argv[]){

   int i;
   i = checkArgs(argc, argv);
   if(i){
      return i;
   }

   char* handle = (char*)malloc(NAME_SIZE);
   memset(handle, '\0', NAME_SIZE);
   char* peerHandle = (char*)malloc(NAME_SIZE);
   memset(peerHandle, '\0', NAME_SIZE);

   while(getHandle(handle));
   printf("Handle: %s\n", handle);
   int sockfd = 0;
   if(argc == 2){
      //localhost
      sockfd = connectToChatServe(NULL, argv[1]);
   } else {
      //non-localhost
      sockfd = connectToChatServe(argv[1], argv[2]);
   }

   int connected = validateWithServer(sockfd, handle, peerHandle);
   char* key = ">";
   if(connected){

      pid_t pid = fork();
      if(pid == 0){
	 //Open a file stream for incoming messages
	 FILE* sockStream = fdopen(sockfd, "r");
	 //Set aside memory for incoming messages
	 char* msg = (char*)malloc(BUF_SIZE+NAME_SIZE+EX_CHARS);
	 //While still connected...
	 int count = 0;
	 while(connected){
	    //Get messages as they arrive
	    memset(msg, '\0', BUF_SIZE+NAME_SIZE+EX_CHARS); 
	    getResponse(sockStream, msg);
	    printf("%s", msg);
	    fflush(stdout);
	    if(!strcmp(msg+strcspn(msg,key)+1, "\\quit\n")){
	       break;
	    }
	 }
	 free(msg);
	 exit(0);
      } else {
	 //Set aside memory for piped messages
	 char* outMsg = (char*)malloc(BUF_SIZE+NAME_SIZE+EX_CHARS);
	 //While connected...
	 while(connected){
	    int exit = waitpid(pid, NULL, WNOHANG);

	    if(!exit){
	       //Get the chat message
	       getInput(outMsg,handle);
	       //Send the chat message
	       sendMsg(sockfd, outMsg);
	    } 
	    
	    //If the sent message was quit, wait for child to exit and close connection
	    if(!strcmp(outMsg+strcspn(outMsg,key)+1, "\\quit\n")){
	       exit = waitpid(pid, NULL, 0);
	       if(exit){
		  close(sockfd);
		  printf("\n\nConnection closed.\n");
		  fflush(stdout);
		  connected = 0;
	       }
	    }
	 }
      }
   }

   close(sockfd);
   return 0;
}

int checkArgs(int argc, char* argv[]){
   //Check if argc != 3 or 4
   if (argc < 2 || argc > 3){
      printf("Usage: %s [host] port_number\n", argv[0]);
      fflush(stdout);
      return -1;
   // else
   } else {
      int i = 0;
      //if argc == 2, localhost = host and argv[1] == portnumber
      if(argc == 2){
	 //Check for valid port number characters
	 while(i < strlen(argv[1])){
	    if(!isdigit(argv[1][i])) {
	       printf("Usage: port_number must be an integer\n");
	       fflush(stdout);
	       return -2;
	    }
	    i++;
	 }
	 //Check to ensure valid port number range
	 if(atoi(argv[1]) < 3000){
	    printf("Usage: port_number must be greater than 3000\n");
	    fflush(stdout);
	    return -2;
	 }
      //else argc == 3, argv[1] = host, and argv[2] = portnumber
      } else {
	 //Check if arg 2 is valid port number characters
	 while(argv[i] != '\0'){
	    if(!isdigit(argv[2][i])) {
	       printf("Usage: port_number must be an integer\n");
	       fflush(stdout);
	       return -2;
	    }
	    i++;
	 }
	 //Check if port number is valid range
	 if(atoi(argv[2]) < 3000){
	    printf("Usage: port_number must be greater than 3000\n");
	    fflush(stdout);
	    return -2;
	 }
      }
   }
   return 0;
}; 

int getHandle(char* h){
   printf("What is your handle (up to 10 characters)? ");
   fflush(stdout);
   //Gets user input
   fgets(h, NAME_SIZE, stdin);
   char key = '\n';
   int c, inCheck = strcspn(h, &key);
   h[inCheck] = '\0';

   if(inCheck == NAME_SIZE-1){
      c = getchar();
      while(c != '\n' && c != EOF){
	 c = getchar();
      }
   }

   printf("Your handle is: %s\nDo you accept (y/n)? ", h);
   fflush(stdout);
   char ans = getchar();

   c = getchar();
   while(c != '\n' && c != EOF){
      c = getchar();
   }
   if(ans == 'y' || ans == 'Y'){
      return 0;
   } else {
      return 1;
   }
}


int connectToChatServe(char* host, char* portno){
   
   struct addrinfo hints;
   memset(&hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;

   struct addrinfo *results, *rp;

   int status = getaddrinfo(host, portno, &hints, &results);

   if(status != 0){
      printf("Client Error: %s\n", gai_strerror(status));
      exit(-1);
   }

   int sockfd = 0;
   for(rp = results; rp != NULL; rp = rp->ai_next){

      sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

      if(sockfd == -1) continue;

      if(connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1){
	 break;
      }

      close(sockfd);
   }

   if(rp != NULL){
      freeaddrinfo(results);
      return sockfd;
   } else {
      printf("Client: Can't connect to server...\n");
      freeaddrinfo(results);
      exit(-1);
   }

}

int validateWithServer(int sfd, char* handle, char* peer){

   int i, n;
   write(sfd, handle, NAME_SIZE);
   read(sfd, peer, NAME_SIZE);
   
   return 1;
} 

int readPipe(int pfd, char* buffer){
   memset(buffer, '\0', BUF_SIZE+NAME_SIZE+EX_CHARS);
   int n = read(pfd, buffer, BUF_SIZE+NAME_SIZE+EX_CHARS);
   return n;
} 

char* getInput(char* buffer, char* handle){

   memset(buffer, '\0',BUF_SIZE+NAME_SIZE+EX_CHARS);
   memcpy(buffer, handle, strlen(handle));
   buffer[strlen(handle)] = '>';
   fgets(buffer+strlen(handle)+1, BUF_SIZE, stdin);
   return buffer;
}

void sendMsg(int sfd, char* msg){
   pid_t pid = fork();

   if(pid == 0){
      int progress = 0;
      int msgLen = strlen(msg);
      while(msg[progress] != '\0'){
	progress = write(sfd, msg, msgLen);
      }
      exit(0);
   } else {
      return;
   }
}   

void getResponse(FILE* sockStream, char* buffer){
   fgets(buffer, BUF_SIZE+NAME_SIZE+EX_CHARS, sockStream);
   return;
}
