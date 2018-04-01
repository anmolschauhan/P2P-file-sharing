#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <unistd.h>

using namespace std;

string ROOT;

string alias, client_ip, client_port, downloading_port;
char* IP_CRS;
int PORT_SERVER;
int PORT_CRS;


int cou = 0;
void acceptClient(int new_socket)
{
    char fileBuffer[1024] = {0};
    read(new_socket, fileBuffer, 1024);
    string uploadFileAddress(fileBuffer); 


    uploadFileAddress = ROOT + '/' + uploadFileAddress; //  ADDING ROOT TO RELATIVE ADDRESS

    FILE *f = fopen( uploadFileAddress.c_str(), "rb");

    while(1)
    {
        char buffer[256] = {0};
        int size = fread(buffer, 1, 256, f);
        //cout << "sending bytes : " << size << endl;
        write(new_socket, buffer, size);
        if(size == 0)//< 256) 
        {
            break;
        }
    }
    fclose(f);
    close(new_socket);
}

vector<string> searchCRS(string searchFilename)
{
    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_CRS);
    inet_pton(AF_INET, IP_CRS, &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    searchFilename = "#1#" + searchFilename;

    send(sock , searchFilename.c_str() , strlen(searchFilename.c_str()) , 0);   // SENDING REQUEST TO CRS

    char result[1024*8] = {0};
    int q = read( sock , result, 1024*8);                                       // RECEIVING SEARCH DATA
    int loc = 0;

    vector<string> searchResults;
    string temp;
    while(result[loc] != '\0')
    {

        if(result[loc] == '#')
        {
            searchResults.push_back(temp);
            temp.clear();
        }
        else
        {
            temp += result[loc];
        }
        loc++;
    }
    printf("FOUND:%ld\n", searchResults.size());
    for(int i = 0; i < searchResults.size(); i++)
    {
        printf("[%d] %s\n", i+1, searchResults[i].c_str());
    }
    close(sock);
    return searchResults;
}

int shareCRS(string fileName, string path)
{
    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_CRS);
    inet_pton(AF_INET, IP_CRS, &serv_addr.sin_addr);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "FAILURE:SERVER_OFFLINE\n";
        close(sock);
    }

    string shareMessage;
    shareMessage = "#2#" + fileName + '#' + path + '#' + alias;

    send(sock , shareMessage.c_str() , strlen(shareMessage.c_str()) , 0);

    cout << "SUCCESS:FILE SHARED\n";

    close(sock);
}

int deleteCRS(string fileName, string path)
{
    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_CRS);
    inet_pton(AF_INET, IP_CRS, &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    string shareMessage;
    shareMessage = "#3#" + fileName + '#' + path + '#' + alias;

    send(sock , shareMessage.c_str() , strlen(shareMessage.c_str()) , 0);

    cout << "DELTED IF CLIENT WAS THE OWNER\n";

    close(sock);
}

void getFileFromServer(string uploadFileAddress, string downloadFileAddress, string IP_SERVER, int PORT_DOWNLOAD_SERVER)
{
    string relative = downloadFileAddress;
    downloadFileAddress = ROOT + '/' + downloadFileAddress; // ADDING ROOT ADRESS TO RELATIVE ADDRESS

    if(access(downloadFileAddress.c_str(), F_OK) != -1) // changes
    {
        cout << "FAILURE:ALREADY_EXISTS\n";
        return;
    }
    //struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_DOWNLOAD_SERVER);
    inet_pton(AF_INET, IP_SERVER.c_str(), &serv_addr.sin_addr);
    if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 )
    {
        cout << "FAILURE:CLIENT_OFFLINE\n";
        close(sock);
        return;
    }
    
    send(sock, uploadFileAddress.c_str(), strlen(uploadFileAddress.c_str()), 0);
    
    
    FILE *f = fopen(downloadFileAddress.c_str(), "wb");
    while(1)
    {
        char buffer[256] = {0};
        int size = read(sock, buffer, 256);
        fwrite(buffer, 1, size, f);
        //cout << size << endl;
        if(size == 0)
            break;
    }
    cout << "SUCCESS:" << relative << endl;
    fclose(f);
    close(sock);
}

void Client()
{
    unordered_map<string, vector<string> > searchCommandResult;     // FOR GET WITH ALIAS NAME
    vector<vector<string> > searchCommandResultVector;              // FOR GET WITH NUMERIC ORDER OF RESULTS
    while(1)
    {
        string command;
        getline(cin, command);
        if(command.find("search ") == 0)
        {

            searchCommandResult.clear();
            searchCommandResultVector.clear();
            string fileName;

            if(command.size() == 7)
            {
                cout << "INVALID_COMMAND\n";
                continue;
            }

            if(command[7] == '"' && command[command.size()-1] == '"')
                fileName = command.substr(8, command.size() - 9);
            else
                fileName = command.substr(7, command.size() - 7);
            
            fileName.erase(remove(fileName.begin(), fileName.end(), (char)92), fileName.end());

            vector<string> stringResult = searchCRS(fileName);  // SEARCH CRS FOR fileName

            for(int i = 0; i < stringResult.size(); i++)
            {
                vector<string> separate;
                string tempS;
                for(int j = 0; j < stringResult[i].size(); j++)
                {
                    if(stringResult[i][j] == ':')
                    {
                        separate.push_back(tempS);
                        tempS.clear();
                    }
                    else
                    {
                        tempS += stringResult[i][j];
                    }
                }
                separate.push_back(tempS);
                searchCommandResult[separate[2]] = vector<string>({separate[3], separate[4]});
                searchCommandResultVector.push_back(vector<string>({separate[1], separate[3], separate[4]}));
            }
        }
        else if(command.find("get ") == 0)
        {
            if(command.size() == 4)
            {
                cout << "INVALID_COMMAND\n";
                continue;
            }
            if(command[4] == '[')
            {
                string number, downloadAddress;
                int location = 5, resultLocation;
                while(command[location] != ']')
                    number += command[location++];
                resultLocation = stoi(number);
                location += 2;
                while(location < command.size())
                    downloadAddress += command[location++];
               
               getFileFromServer(searchCommandResultVector[resultLocation-1][0], downloadAddress, searchCommandResultVector[resultLocation-1][1], stoi(searchCommandResultVector[resultLocation-1][2]));
            }
            else    // TESTING REMAINING
            {
                string alias, path, downloadAddress;
                int location = 4;
                while(command[location] != ' ')
                    alias += command[location++];
                location++;
                while(command[location] != ' ')
                    path += command[location++];
                location++;
                while(location < command.size())
                    downloadAddress += command[location++];

                if(alias[0] == '"' && alias[alias.size()-1] == '"')
                    alias = alias.substr(1, alias.size()-2);
                
                if(path[0] == '"' && path[path.size()-1] == '"')
                    path = path.substr(1, path.size()-2);

                if(downloadAddress[0] == '"' && downloadAddress[downloadAddress.size()-1] == '"')
                    downloadAddress = downloadAddress.substr(1, downloadAddress.size()-2);

                alias.erase(remove(alias.begin(), alias.end(), (char)92), alias.end());
                path.erase(remove(path.begin(), path.end(), (char)92), path.end());
                downloadAddress.erase(remove(downloadAddress.begin(), downloadAddress.end(), (char)92), downloadAddress.end());

                getFileFromServer(path, downloadAddress, searchCommandResult[alias][0], stoi(searchCommandResult[alias][1]));
            }
        }
        else if(command.find("share ") == 0)
        {
            if(command.size() == 6)
            {
                cout << "INVALID_COMMAND\n";
                continue;
            }
            string path, fileName;
            int location = 6;
            while(location < command.size())
                path += command[location++];
            
            if(path[0] == '"' && path[path.size()-1] == '"')    // REMOVE INVERTED COMMAS AT THE ENDS IF ANY
                    path = path.substr(1, path.size()-2);

            path.erase(remove(path.begin(), path.end(), (char)92), path.end()); // REMOVE \ FROM THE STRING IF ANY

            if(path[path.size()-1] == '/')
                path = path.substr(0, path.size() - 1);

            location = 0;
            while(location < path.size())       // TESTING NOT DONE
            {
                if(path[location] == '/')
                    fileName.clear();
                else
                    fileName += path[location];
                location++;
            }

            shareCRS(fileName, path);
        }
        else if(command.find("del ") == 0)
        {
            if(command.size() == 4)
            {
                cout << "INVALID_COMMAND\n";
                continue;
            }
            string path, fileName;
            int location = 4;
            while(location < command.size())
                path += command[location++];
            
            if(path[0] == '"' && path[path.size()-1] == '"')    // REMOVE INVERTED COMMAS AT THE ENDS IF ANY
                    path = path.substr(1, path.size()-2);

            path.erase(remove(path.begin(), path.end(), (char)92), path.end()); // REMOVE \ FROM THE STRING IF ANY

            if(path[path.size()-1] == '/')
                path = path.substr(0, path.size() - 1);

            location = 0;
            while(location < path.size())       // TESTING NOT DONE
            {
                if(path[location] == '/')
                    fileName.clear();
                else
                    fileName += path[location];
                location++;
            }

            deleteCRS(fileName, path);
        }
        else
        {
            cout << "INVALID COMMAND\n";
        }
    }
}

void Server()
{
	int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT_SERVER );
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    

    vector<thread> uploadThreads;

    while(true)
    {
        //cout << "server side of client : waiting for connection\n";
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        uploadThreads.push_back(thread(acceptClient, new_socket));
        //uploadThreads.back().detach(); // TEST
    }

    for(int i = 0; i < uploadThreads.size(); i++)
    {
        uploadThreads[i].join();
    }
}

int main(int argc, char ** argv)
{
    //string #alias, #client_ip, #client_port, #server_ip, #server_port, #downloading_port, #client_root;
    
    IP_CRS = argv[4];
    //PORT_SERVER = atoi(argv[6]);
    PORT_CRS = atoi(argv[5]);
    ROOT = string(argv[7]);
    
    alias = string(argv[1]);
    client_ip = string(argv[2]);
    client_port = string(argv[3]);
    downloading_port = string(argv[6]);
    PORT_SERVER = stoi(downloading_port);

    //cout << "\n" << alias << " " << client_ip << " " << client_port << " " << downloading_port << " " << PORT_SERVER << endl; // BUG

    string selfData = alias + "#" + client_ip + "#" + client_port + "#" + downloading_port + "#";

    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_CRS);
    inet_pton(AF_INET, IP_CRS, &serv_addr.sin_addr);
    if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 )
    {
        cout << "CONNECTION FAILED\n";
        return 0;
    }

    send(sock , selfData.c_str(), strlen(selfData.c_str()) , 0);

    close(sock);
    //initialize aliaslist on CRS

    

	thread client(Client);
	thread server(Server);
	client.join();
	server.join();
	return 0;
}