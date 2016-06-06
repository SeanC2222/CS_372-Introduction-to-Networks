/* Author: Sean Mulholland
 * File: ftserver.cpp
 * Description: When used with ftclient.py can serve files to the client.
 *	        Can change process working directory and transfers files
 *	        byte by byte, line by line from requested file. Can also
 *	        serve current working directory listings.
 */

#include <sys/socket.h>	   //System Socket Library
#include <unistd.h>	   //Unix Standard
#include <sys/types.h>	   //System Data Types
#include <sys/wait.h>	   //Flags for waiting functions
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <time.h>

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "inetLib.hpp"

#define QUEUE_LENGTH 5	//Number of active connections between hosts
#define BUF_SIZE 512
#define BAD_FILE_NAME "Bad file name"

//Checks input argc, argv
int checkArgs(int, char*[]);
//Main ft loop
int ftProcess(int, struct sockaddr_in, socklen_t);
//Gets response from socket
std::string getMessage(int);
//Sends message through socket
int sendMessage(int, std::string);

int main(int argc, char* argv[]){
   //Check Command line arguments
   if(checkArgs(argc, argv)){
      return -1;
   }

   //Create new inetSock from portNumber argument
   inetSock servSock(argv[1]);

   //Sets listening socket to non-blocking

   //Listen on file descriptor
   listen(servSock.getFileDescriptor(), QUEUE_LENGTH);	 //QUEUE_LENGTH macro defined

   int tempfd;
   struct sockaddr_in tempAddr;
   socklen_t tempLen = sizeof(struct sockaddr_in);

   fcntl(servSock.getFileDescriptor(), F_SETFL, fcntl(servSock.getFileDescriptor(), F_GETFL) | O_NONBLOCK);
   while(1){
      tempfd = accept(servSock.getFileDescriptor(), (struct sockaddr*)&tempAddr, &tempLen);
      setsockopt(tempfd, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
      if(tempfd != -1){
	 ftProcess(tempfd, tempAddr, tempLen);
	 close(tempfd);
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

std::string getMessage(int fd){
   //Create Buffer and \0 it
   char* buffer = (char*)malloc(BUF_SIZE);
   memset(buffer, '\0', BUF_SIZE * sizeof(char));
   int n = 0;
   //Wait for message to arrive
   while(n == 0){
      n = recv(fd, buffer, BUF_SIZE-1, MSG_PEEK);
   }
   //Receive message
   n = recv(fd, buffer, BUF_SIZE-1, 0);
   std::string message(buffer);
   free(buffer);
   return message;
}
   
int sendMessage(int fd, std::string message){
   int progress = 0;
   while(progress < message.size()){
      progress += write(fd, message.c_str()+progress, message.size());
   }
   return progress;
}

int connectToDataSock(struct sockaddr_in& cliaddr, socklen_t &clilen, int dataport){

   //Look for correct dataport at address
   cliaddr.sin_port = htons(dataport);
   //Create socket
   int datafd = socket(cliaddr.sin_family, SOCK_STREAM, 0);
   //Set socket for reuse
   setsockopt(datafd, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
   //If connection isn't successful return -1
   if(connect(datafd, (struct sockaddr*)&cliaddr, clilen) == -1){
      perror("Server: Data connection error");
      close(datafd);
      return 0;
   //Else connection is successful return 0
   } else {
      return datafd;
   }
}

int ftProcess(int sockfd, struct sockaddr_in cliaddr, socklen_t clilen){

   //Get Raw Command
   std::string rawCommand = getMessage(sockfd);
   std::cout << "Server: Command - " << rawCommand << std::endl;

   std::stringstream scomm(rawCommand);
   std::string command, arg1, arg2;
   scomm >> command >> arg1 >> arg2;

   if (command == "-g"){
      int datafd = connectToDataSock(cliaddr, clilen, atoi(arg2.c_str()));
      if(datafd){
	 pid_t pid = fork();
	 //Child
	 if (pid == 0){
	    close(sockfd);
	    std::string msg;
	    std::ifstream file(arg1.c_str(), std::ifstream::in);
	    if(!file.is_open()){
	       msg = BAD_FILE_NAME;
	       sendMessage(datafd, msg);
	    } else {
	       int n = 0;
	       while(getline(file, msg)){
		  msg += "\n";
		  n = sendMessage(datafd, msg);
		  if(n == 0){
		     std::cout << "Server: Error sending data" << std::endl;
		     close(datafd);
		     exit(-1);
		     break;
		  }
	       }
	       file.close();
	    }
	    close(datafd);
	    exit(0);
	 //Parent
	 } else {
	    close(datafd);
	    int status = 0;
	    wait(&status);
	    if(status != 0){
	       std::cout << "Server: Data sending error" << std::endl;
	    }
	 }

      }
   } else if (command == "-l"){
      int datafd = connectToDataSock(cliaddr, clilen, atoi(arg1.c_str()));
      if(datafd){
	 pid_t pid = fork();
	 //Child
	 if(pid == 0){
	    dup2(datafd,STDOUT_FILENO);
	    execl("/bin/ls", "ls", 0);
	    //should not run....
	    exit(255);
	 //Parent
	 } else {
	    //Wait for ls child to complete
	    int status = 0;
	    wait(&status);
	    if(status != 0){
	       std::cout << "Server: Data sending error" << std::endl;
	    }
	 }
      }
      close(datafd);
   } else if (command == "cd"){
      int fail = chdir(arg1.c_str());
      if(!fail) {
	 char* wd = get_current_dir_name();
	 std::string response(wd);
	 sendMessage(sockfd, response);
	 free(wd);
      } else {
	 std::string response("Directory not changed");
	 sendMessage(sockfd, response);
      }
   } else {
      sendMessage(sockfd, "Bad command");
   }

   return 0;
}
