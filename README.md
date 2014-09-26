FSRT(File Send/Recv Tool)
==============

     Linux File Send/Recv Tool
     Version 0.2.0
     It can run on author Linux system(Centos 6.5/Ubuntu 14.04) without add any lib 
     or do anything complicate, you just need to run command "make" and
     then you can run the program. 
     
     You can send file or recv file from one linux to other linux.
     You can switch which to be a server or a client.

###Usage:

     # Option:
     -s/--send                (send)send file mode
     -r/--recv                (recv)recv file mode
     -c/--client              (client)client mode
     -a/--server              (accept)server mode
     -f/--file [file]         (file)Send file(If -s is set then this must be set)
     -i/--ip [ip]             (ip)Set conn or listen ip
     -p/--port [port]         (port)Set conn or listen port
     -v/-h/--help/--version   (version/help)

###Example:

     1. Server recv file and client send file.
          Server: ./fsrt -r -a -i 0.0.0.0 -p 6500
          Client: ./fsrt -s -c -i X.X.X.X -p 6500 -f send_file.tar.gz
     2. Server send file and client recv file.
          Server: ./fsrt -s -a -i 0.0.0.0 -p 6500 -f send_file.tar.gz
          Client: ./fsrt -r -c -i X.X.X.X -p 6500 
     
     Make sure Server always run before Client
     
###Notic:

     1. The recv file with save the name as temp.file in program path, 
     we should change it name to what we want now.(Next version will change name by auto)
     2. More and more function will add in this tool on furture version.
