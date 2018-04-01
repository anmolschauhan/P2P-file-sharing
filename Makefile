all: client crs
client: client_sc.cpp
		g++ -std=c++11 -pthread client_sc.cpp -o client_20172099
crs:	CRS_server.cpp
		g++ -std=c++11 -pthread CRS_server.cpp -o server_20172099