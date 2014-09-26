FSRT(File Send/Recv Tool)
==============

     Linux File Send/Recv Tool
     Version 0.2.0
     
     You can send file or recv file from one linux to other linux.
     You can switch which to be a server or a client.

###Usage:

     Option:
     -s    (send)send file mode
     -r    (recv)recv file mode
     -c    (client)client mode
     -a    (accept)server mode
     -f [file]   (file)Send file(If -s is set then this must be set)
     -i [ip]     (ip)Set conn or listen ip
     -p [port]   (port)Set conn or listen port
     -v/-h   (version/help)

###Example:

     1. Server recv file and client send file.
          Server: ./send -r -a -i 0.0.0.0 -p 6500
          Client: ./send -s -c -i X.X.X.X -p 6500 -f send_file.tar.gz
     2. Server send file and client recv file.
          Server: ./send -s -a -i 0.0.0.0 -p 6500 -f send_file.tar.gz
          Client: ./send -r -c -i X.X.X.X -p 6500 
     
     Make sure Server always run before Client
     
###Notic:

     1. The recv file with save the name as temp.file in program path, 
     we should change it name to what we want now.(Next version will change name by auto)
     2. More and more function will add in this tool on furture version.
