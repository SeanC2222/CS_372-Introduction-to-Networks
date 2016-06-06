ftserv:

   With the included makefile type you can either use the default target:
      make
   or use ftserv as a target:
      make ftserv
   
   Proper compiling will require inetLib.cpp and inetLib.hpp in the working
   directory.

   To run ftserv use the following command structure:
      ftserv <port_number>

   This will establish a server that is listening on port_number for
   for connections.

ftclient.py:

   The client script can be run once the server has been created. You can run
   the script with the following command:
      python ftclient.py <host> <port_number> <command> [parameter] [data_port]

   The host can be dot notation version of the IP address that the server is on
   or it can be an alias such as flip1. port_number should correllate to the 
   port that the server is listening on.
   
   The available command and optional arguments must be one of the following:

      -l <data_port>
   -l will provide the server's current directory listing, and the data will
   be communicated to the provided data_port number

      -g <file_name> <data_port>
   -g will tell the server to open the file named file_name, and send it line
   by line to data_port on the client. If an incorrect file_name is sent, the
   server will send the response "Bad file name". Files will be copied into
   the clients working directory. If a file with the same name exists, a ".cpy"
   will be appended to it. If a copy with the same name already exists, the 
   .cpy version will be clobbered (i.e. replaced).

      cd <path_info>
   cd will change the working directory of the ftserv process. When successful
   the new current path will be returned to the client and output to the
   terminal. When unsuccessful, "Directory not changed" is returned to the
   client.

Extra Credit:
   
   The changing of working directories is an additional feature not specified
   in the program requirements.

   Two features are handled by forking processes on both the server end and
   the client end. These features are:
      Server:
	 - Forks a process to handle sending data on the data port for -g or -l
	    commands
      Client:
	 - Forks a process to send the command to the server that waits a
	    fraction of a second before sending to allow the data socket to
	    be set up to listen.

   The server can actual transfer non-text files as well. I successfully 
   transferred executables between flip instances that still ran succesfully
   as well as an mp3 that I transferred between flip instances. 
