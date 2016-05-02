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

#include "inetLib.hpp"

void inetSock::_getAddressInfo(char* port){
   //Get the address info in results
   int s = getaddrinfo(NULL, port, &this->hints, &this->addrRes);
   if(s != 0){
      std::cout << "Server: Could not retrieve address info" << std::endl;
      std::cout << "Error: " << gai_strerror(s) << std::endl;
      exit(-1);
   }
}


inetSock::inetSock(){
   this->sFD = 0;
   this->pNo = 0;
   this->b = 0;
   memset(&this->addr, 0, sizeof(struct sockaddr_in));
   //Prepare address hints
   memset(&this->hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
      hints.ai_protocol = 0;

}

inetSock::inetSock(const inetSock &sock){
   this->sFD = sock.getFileDescriptor();
   this->pNo = sock.getPortNumber();
   this->b = sock.isBound();
   if(this->b == 1){
      memset(&this->addr, 0, sizeof(struct sockaddr_in));
   } else {
      this->addr.sin_family = sock.getAddr().sin_family;
      this->addr.sin_port = sock.getAddr().sin_port;
      this->addr.sin_addr = sock.getAddr().sin_addr;
   }
   this->hints = sock.getHints();
}
   
inetSock::inetSock(char* portno){
    memset(&this->hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
      hints.ai_protocol = 0;
  
   _getAddressInfo(portno);
   
   struct addrinfo *ip;
   for(ip = this->addrRes; ip != NULL; ip = ip->ai_next){
      this->sFD = socket(ip->ai_family, ip->ai_socktype, ip->ai_protocol);
      if(sFD == -1) continue;

      if(!bind(this->sFD, ip->ai_addr, ip->ai_addrlen)){
	 break;
      }
      close(sFD);
   }

   if(ip == NULL){
      std::cout << "inetSock: Could not bind" << std::endl;
      exit(-1);
   }

   this->pNo = atoi(portno);
   this->b = 1;
   freeaddrinfo(this->addrRes);

}

inetSock& inetSock::operator=(const inetSock& target){
   if(this != &target){
      this->sFD = target.getFileDescriptor();
      this->pNo = target.getPortNumber();
      this->b = target.isBound();
      this->addr = target.getAddr();
      this->hints = target.getHints();
   }
   return *this;
}

int inetSock::getFileDescriptor() const {return this->sFD;}
int inetSock::getPortNumber() const {return this->pNo;}
int inetSock::isBound() const {return this->b;}
struct sockaddr_in inetSock::getAddr() const {return this->addr;}
struct addrinfo inetSock::getHints() const{return this->hints;}

void inetSock::setFileDescriptor(int a){this->sFD = a;}
void inetSock::setPortNumber(char* portno){new (this) inetSock(portno);}

