# Author: Sean Mulholland
# File: ftclient.py
# Description: Use with ftserv executable to transfer files line by line
#	       and save to the current directory ftclient.py is being
#	       executed in. This can function for plain text files, or other
#	       encoded files.

import sys, socket, os
from time import sleep

#Definitions
MSG_SIZE = 512			 #Maximum message size managed 
BAD_FILE_NAME = "Bad file name"  #Message String for bad file name

def checkArgs():

   #Not enough args
   if len(sys.argv) < 4:
      print(str(sys.argv[0]) + " <host> <portnumber> <command> [parameter] [dataport]")
      exit(-1)
   #Not the correct number of args for -g
   elif sys.argv[3] == "-g" and len(sys.argv) != 6: 
      print(str(sys.argv[0]) + " <host> <portnumber> -g <filename> <dataport>")
      exit(-1)
   #Not the correct number of args for -l
   elif sys.argv[3] == "-l" and len(sys.argv) != 5:
      print(str(sys.argv[0]) + " <host> <portnumber> -l <dataport>")
      exit(-1)
   #Not the correct number of args for cd
   elif sys.argv[3] == "cd" and len(sys.argv) != 5:
      print(str(sys.argv[0]) + " <host> <portnumber> cd <path_info>")
      exit(-1)
   #Else assumed fine
   elif sys.argv[3] != "-l" and sys.argv[3] != "-g" and sys.argv[3] != "cd":
      print(str(sys.argv[0]) + " <host> <portnumber> <command> [parameter] [dataport]")
      print("Commands: -l list directory listing on <dataport>")
      print("          -g get file <parameter> on <dataport>")
      print("          cd change server working directory to <parameter>")
      exit(-1)
   else:
      return

def sendMessage(sock, message):
   try:			      #Try to send a message
      n = sock.send(message)
   except socket.error, msg:  #Catch, print error, close sock
      print("Client: Socket error send")
      print("Client: ERROR CODE: " + str(msg[0]) + " MSG: " + msg[1])
      sock.close()
      exit()

def receiveMessage(sock):
   try:			      #Try to receive a message
      res = sock.recv(MSG_SIZE)
      return res
   except socket.error, msg:  #Catch, print error, close sock
      print("Client: Socket error receive")
      print("Client: ERROR CODE: " + str(msg[0]) + " MSG: " + msg[1])
      sock.close()
      exit()

def createDataConnection():
   if sys.argv[3] == "-g":    #If getting a file, define HOST/PORT
      HOST = socket.gethostname()
      DATA = int(sys.argv[5])
   elif sys.argv[3] == "-l":  #Else if listing, define HOST/PORT
      HOST = socket.gethostname()
      DATA = int(sys.argv[4])

   #Create a listening socket
   datas = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
   datas.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
   datas.bind((HOST, DATA))
   datas.listen(1)
   #Accept a data connection
   conn, addr = datas.accept()
   #Close listening socket
   datas.close()
   #Return data connection
   return conn 
   

def createConnections():
   #Create command connection
   sock = socket.create_connection((sys.argv[1], sys.argv[2]))
   sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
   comm = ""
   #Build command from command line arguments
   for i in range(3, len(sys.argv)):
      comm += sys.argv[i]
      if i < len(sys.argv)-1:
	 comm += " "
   #Fork to allow time for listening socket to set up
   pid = os.fork()
   if pid == 0:
      sleep(0.001)
      sendMessage(sock, comm)
      exit(0)
   else:
      #Create listening data socket, get data connection
      if sys.argv[3] == "-g" or sys.argv[3] == "-l":
	 datas = createDataConnection()
	 os.wait()
	 return datas
      #Other requests don't require a dataconnection
      else:
	 return sock 

def main():
   checkArgs()			    #Validate number of arguments
   datas = createConnections() #Set up connections

   msg = "placeholder"	    
   if sys.argv[3] == "-g":	  #If getting a file
      if os.path.isfile(sys.argv[4]):	#If filename exists add .cpy to target
	 filename = sys.argv[4] + ".cpy"
      else:				#Else file can have same name
	 filename = sys.argv[4]
      targF = open(filename, "w")
   
      while len(msg):			#While msg isn't empty string
	 msg = receiveMessage(datas)  
	 if msg != BAD_FILE_NAME:		#If msg isn't BAD_FILE_NAME error msg
	    targF.write(msg)		   #Write to file
	 else:				#Else write to STDOUT 
	    os.remove(filename)
	    print(msg)
      targF.close()
   else:				  #Not getting a file
      while len(msg):		     #While msg isn't empty string
	 msg = receiveMessage(datas) 
	 print(msg)		     #Print msg

   datas.close()			  #Close data connection
   exit(0)			  #Child exits
   return 0

main()
