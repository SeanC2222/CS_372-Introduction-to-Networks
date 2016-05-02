#include <sys/socket.h>	   //System Socket Library
#include <unistd.h>	   //Unix Standard
#include <sys/types.h>	   //System Data Types
#include <sys/wait.h>	   //Flags for waiting functions
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>

#include "inetLib.hpp"

/************************************************************************************************************
 * 
 *			   Socket Notes:
 * 
 *
 * Socket address structs:
 *
 * NOTE: sockaddr only exists for casting of sockaddr_in in functions below.
 * !-------DON'T create a stand alone struct sockaddr!--------!
 * struct sockaddr {
 *    sa_family_t	   sa_family;	     AF_INET/AF_INET6
 *    char		   sa_data[14];	     Relevant sockaddr data
 * }
 *
 * struct sockaddr_un{
 *    sa_family_t	   sun_family;	     AF_UNIX
 *    char		   sun_path[108];    Local pathname for communcations
 * }
 *
 * struct sockaddr_in{ 
 *    short		   sin_family;	     AF_INET/AF_INET6
 *    unsigned short	   sin_port;	     Network byte order portno; eg htons(portno)
 *    struct		   in_addr;	     Struct representing IPv4 internet address
 *    char		   sin_zero[8];	     Zero padding; memset to 0
 * }
 *
 * struct in_addr {
 *    union {
 * 	 struct {			     IPv4 address formatted as u_chars
 *	    u_char s_b1, s_b2, s_b3, s_b4;
 *	 } S_un_b;			     Stored in S_un_b
 *
 *	 struct {			     IPv4 address formatted as u_shorts 
 *	    u_short s_w1, s_w2;		     
 *	 } S_un_w;			     Stored in S_un_w
 *	 
 *	 u_long S_addr;			     IPv4 address formatted as u_long
 *    } S_un;				     Union of bytes as S_un
 * };
 *
 *
 * Socket Functions:
 *    
 *    #include <sys/socket.h>
 *
 *    int      accept(int socket, struct sockaddr *address, socklen_t *address_len);
 *	 
 *	    Accepts a new connection on a socket. Accepts first connection from queue, 
 *	    creates new socket with same type protocol and address family as specified in socket.
 *	    Returns new sockets file descriptor
 *
 *    int      bind(int socket, const struct sockaddr *address, socklen_t address_len);
 *
 *	    Binds a name to a socket. Accepts socket file descriptor and directs it to
 *	    address. Use when creating a socket with socket() as these sockets are initially unnamed.
 *
 *    int      connect(int socket, const struct sockaddr *address,socklen_t address_len);
 *
 *	    Connects a socket. Requests a connection with address to be formed on socket.
 *
 *    int      listen(int socket, int backlog);
 *
 *	    Listen for socket connections and limit queue of possible connections.
 *	    backlog indicates size of queue.
 *
 *    ssize_t  recv(int socket, void *buffer, size_t length, int flags);
 *	    
 *	    Receives a message from a connected socket into buffer.
 *
 *    ssize_t  recv_from(int socket, void *buffer, size_t length,int flags, 
 *		     struct sockaddr *address, socklen_t *address_len);
 *
 *	    Receives a message from a socket, but allows retrieval of source address information.
 *
 *    ssize_t  send(int socket, const void *message, size_t length, int flags);
 *
 *	    Sends a message to a socket.
 *
 *    ssize_t  sendto(int socket, const void *message, size_t length, int flags,
 *		     const struct sockaddr *dest_addr, socklen_t dest_len);
 *
 *	    Sends a message to a socket. If connectionless, message will be sent to dest_addr.
 *
 *    int      shutdown(int socket, int how);
 *	    
 *	    Disables send/recieve operations. dependent on how: SHUT_RD, SHUT_WR, SHUT_RDWR
 *
 *    int      socket(int domain, int type, int protocol);
 *
 *	    Creates an unnamed socket and returns socket file descriptor. 
 *
 *	       domain:
 *		     AF_UNIX	       File system pathway of communication
 *		     AF_INET	       Internet address communication
 *
 *	       type:
 *		     SOCK_STREAM       TDP
 *		     SOCK_DGRAM	       UDP
 *
 *		     | SOCK_NONBLOCK   Bitwise OR with base type for non-blocking read/write
 *		     | SOCK_CLOEXEC    Bitwise OR with base type for close-on-execution flag for socket FD
 *
 *    int      socketpair(int domain, int type, int protocol, int socket_vector[2]);
 *
 *	    Creates an unnamed (unbound) pair of connected sockets in domain. Only used for interprocess
 *	    communications (i.e. parent/child communications).
 *
 *************************************************************************************************************/

#define QUEUE_LENGTH 5	//Number of active connections between hosts
#define BUF_SIZE 500
#define NAME_SIZE 10
#define EX_CHARS 2


//Checks input argc, argv
int checkArgs(int, char*[]);
//Main Chat loop
int chatProcess(int, std::string);
//Gets server Handle
std::string getHandle();
//validates connection with client, and exchanges handles
int validateWithClient(int, std::string&, std::string&);
//Gets response from socket
std::string getResponse(FILE*);
//Gets input from stdin
std::string getInput();
//Sends message through socket
voi sendMessage(int, std::string);

int main(int argc, char* argv[]){
   //Check Command line arguments
   if(checkArgs(argc, argv)){
      return -1;
   }

   //Create new inetSock from portNumber argument
   inetSock servSock(argv[1]);
   
   //Sets listening socket to non-blocking  
//   fcntl(servSock.getFileDescriptor(), F_SETFL, O_NONBLOCK);

   std::string servHandle = getHandle();

   //Listen on file descriptor
   listen(servSock.getFileDescriptor(), QUEUE_LENGTH);	 //QUEUE_LENGTH macro defined

   //Client/connection information
   std::vector< std::pair<std::string, inetSock> > clients;

   int tempFD;
   struct sockaddr_in tempAddr;
   socklen_t tempLen = sizeof(struct sockaddr_in);

   while(1){
      std::cout << "Waiting on peer connection..." << std::endl;
      tempFD = accept(servSock.getFileDescriptor(), (struct sockaddr*)&tempAddr, &tempLen);
      if(errno != EWOULDBLOCK && errno != EAGAIN){
	 fcntl(tempFD, F_SETFL, fcntl(tempFD, F_GETFL) & (~O_NONBLOCK));
	 chatProcess(tempFD, servHandle);
      } 
   }

   return 0;
}

int checkArgs(int argc, char* argv[]){
   if (argc != 2){
      std::cout << "Usage: " << argv[0] << " [port number] " << std::endl;
      return -1;
   } else {
      int i = 0;
      while(argv[i] != '\0'){
	 if(!isdigit(argv[1][i])) {
	    std::cout << "Usage: [port number] must be an integer" << std::endl;
	    return -2;
	 }
	 i++;
      }
      if(atoi(argv[1]) < 3000){
	 std::cout << "Usage: [port number] must be greater than 3000" << std::endl;
	 return -2;
      }
   }
   return 0;
};

std::string getHandle(){
   std::string handle;
   do{
      std::cout << "What is your handle (up to " << NAME_SIZE << " characters)? ";
      std::getline(std::cin, handle);
   }while(handle.size() > NAME_SIZE);

   return handle;
}

int chatProcess(int sockfd, std::string servHandle){
   std::string peerHandle;
   int connected = validateWithClient(sockfd, servHandle, peerHandle);
   std::cout << peerHandle << " has connected." << std::endl;

   while(connected){
     
      pid_t pid = fork();
      if(pid == 0){ 
	 while(connected){
	    //Get message from client 
	    FILE* sockStream = fdopen(sockfd, "r");
	    std::string msg = getResponse(sockStream);
	    //If message is quit, close both file descriptors
	    if(msg == peerHandle + ">\\quit\n"){
	       msg = "\\quit\n";
	       sendMessage(sockfd, msg);
	       connected = 0;
	    } else {
	       std::cout << msg;
	    }
	 }
	 exit(0);
      //Else write to second file descriptor
      } else {
	 while(connected){
	    
	    std::string newMsg = servHandle + ">";
	    //Check if quit before getting input
	    int exit = waitpid(pid, NULL, WNOHANG);

	    std::string msgContent;
	    if(!exit){
	       msgContent = getInput();
	       newMsg += msgContent;
	       sendMessage(sockfd, newMsg);
	    }
	    if(msgContent == "\\quit\n"){
	       exit = waitpid(pid, NULL, 0);
	       if(exit){
		  std::cout << "\nConnection closed.\n" << std::endl;
		  connected = 0;
		  break;
	       }
	    }
	 }
      }
   }

   std::cout << "Server: Exiting..." << std::endl;
   return 0;
}

int validateWithClient(int sockfd, std::string& servHandle, std::string& peerHandle){

   char* id = (char*)malloc(NAME_SIZE);

   int i;
   i = read(sockfd, id, NAME_SIZE);
   peerHandle = id;

   i = write(sockfd, servHandle.c_str(), NAME_SIZE);

   free(id);
   return 1;
}

std::string getResponse(FILE* sockStream){
   char* buffer = (char*)malloc(BUF_SIZE+NAME_SIZE+EX_CHARS);
   fgets(buffer, BUF_SIZE+NAME_SIZE+EX_CHARS, sockStream);
   std::string message(buffer);
   free(buffer);

   return message;
}
   
std::string getInput(){
   std::string message;
   //Ensure message fits in buffer
   do{
      std::getline(std::cin, message);
      message.push_back('\n');
   }while(message.size() >= BUF_SIZE+NAME_SIZE+EX_CHARS);
   return message;
}

void sendMessage(int sockfd, std::string message){
   int progress = 0;
   while(progress < message.size()){
      progress += write(sockfd,(void*)&message.at(progress),(size_t)1);
   }
   return;
}
