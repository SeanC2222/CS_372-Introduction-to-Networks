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

   inetSock();
   inetSock(const inetSock&);
   inetSock(char*);
   
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
