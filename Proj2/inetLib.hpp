#ifndef _INETLIB_H
#define _INTELIB_H

class inetSock{

private:
   int sFD;			 //Socket File Descriptor
   int pNo;			 //Socket port number
   int b;			 //Bool check for socket bound or not (i.e., is client or not)
   struct sockaddr_in addr;   //Stores client address info if b == 0
   struct addrinfo hints;
   struct addrinfo *addrRes;

   void _getAddressInfo(char*);

public:

   //Bare constructor
      //sFD		   = 0
      //pNo		   = 0
      //b		   = 0
      //addr		   = 0
      //hints.ai_family	   = AF_INET
      //hints.ai_socktype  = SOCK_STREAM
      //hints.ai_flags	   = AI_PASSIVE
      //hints.ai_protocol  = 0
   inetSock();
   //Copy constructor
   inetSock(const inetSock&);
   //Listening Socket constructor
   inetSock(char*);
   //Copy Operator
   inetSock& operator=(const inetSock&);

   int getFileDescriptor() const;
   int getPortNumber() const;
   int isBound() const;
   struct sockaddr_in getAddr() const;
   struct addrinfo getHints() const;

   void setFileDescriptor(int);
   void setPortNumber(char*);

};

#endif
