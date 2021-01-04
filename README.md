# tinyproxy
A light-weight proxy server written through C, supporting http proxy, sock5 and port mapping.


# Build in Windows:
execute build.bat

PS: A Visual Studio 2008 ~ 2015 compiling environment is requested for building this proxy server.


# Build in Linux:
execute ./build.sh

PS: gcc is requested for building this proxy server.


# Usage
http-proxy:
	# Windows:
	proxy.exe -h -l your_port
	
	# linux:
	./proxy -h -l your_port
	
sock5-proxy:
	# Windows:
	proxy.exe -s -l your_port
	
	# linux:
	./proxy -s -l your_port
	
	PS: the BIND and UDP ASSOCIATE capabilities of sock5 proxy have not been finished yet.
	
port mapping:
	# Windows:
	proxy.exe -m -l your_port -a remote_host -p remote_port
	
	# linux:
	./proxy -m -l your_port -a remote_host -p remote_port
	
	