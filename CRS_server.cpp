#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <bits/stdc++.h>

//#define CRS_PORT 5000
using namespace std;

int CRS_PORT;
string ROOT;

string repofilename, aliasfilename;

int countx = 0;

class Database
{
public:
	multimap<string, pair<string, string> > fileLocAliasList;
	unordered_map<string, vector<string> > clientList;
	void initialize(string, string);
	void updateRepo(void);
	pair<multimap<string, pair<string, string> >::iterator, multimap<string, pair<string, string> >::iterator> searchPrefix(string input);
	//string searchPrefix(string input);
};

Database database;

void Database::initialize(string repofilename, string aliasfilename)
{
	ifstream is(repofilename);
	string input[3];
	int i = 0;
	char c, tc;
	while(is.get(c))
	{
		if(c == ':')
			i++;
		else if(c == '\n')
		{
			fileLocAliasList.insert({input[0], {input[1], input[2]} });
			input[0].clear();input[1].clear();input[2].clear();
			i = 0;
		}
		else
			input[i] += c;

		tc = c;
	}
	if(input[0].size() != 0 && input[1].size() != 0 && input[2].size() != 0)
		fileLocAliasList.insert({input[0], {input[1], input[2]} });
	
	is.close();

	ifstream os(aliasfilename);
	vector<string> input1;
	string val;
	int loopCount = 0;
	while(os.get(c))
	{
		if(c == ':')
		{
			input1.push_back(val);
			val.clear();
		}
		else if(c == '\n')
		{
			input1.push_back(val);
			val.clear();
			clientList[input1[0]] = vector<string>({input1[1], input1[2], input1[3]});
			input1.clear();
		}
		else
		{
			val += c;
		}
		tc = c;
		loopCount++;
	}
	if(tc != '\n' && loopCount > 0)
	{
		input1.push_back(val);
		clientList.insert({input1[0], vector<string>({input1[1], input1[2], input1[3]})});
	}
	val.clear();
	input1.clear();
	os.close();
}

void Database::updateRepo(void)
{
	ofstream os1(repofilename);
	ofstream os2(aliasfilename);
	for(auto it = fileLocAliasList.begin(); it != fileLocAliasList.end(); it++)
	{
		for(int i = 0; i < it->first.size(); i++)
		{
			os1.put(it->first[i]);
		}
		os1.put(':');
		for(int i = 0; i < it->second.first.size(); i++)
		{
			os1.put(it->second.first[i]);
		}
		os1.put(':');
		for(int i = 0; i < it->second.second.size(); i++)
		{
			os1.put(it->second.second[i]);
		}
		os1.put('\n');
	}
	os1.close();
	for(auto it = clientList.begin(); it != clientList.end(); it++)
	{
		for(int i = 0; i < it->first.size(); i++)
			os2.put(it->first[i]);
		os2.put(':');
		for(int i = 0; i < it->second[0].size(); i++)
			os2.put(it->second[0][i]);
		os2.put(':');
		for(int i = 0; i < it->second[1].size(); i++)
			os2.put(it->second[1][i]);
		os2.put(':');
		for(int i = 0; i < it->second[2].size(); i++)
			os2.put(it->second[2][i]);
		os2.put('\n');
	}
	os2.close();
	// write data structure on the file
}

pair< multimap<string, pair<string, string>>::iterator, multimap<string, pair<string, string>>::iterator> Database::searchPrefix(string input)
{
	string orig;
	orig = input;
	char c = input.back();
	input.erase(input.size()-1, 1);
	input += (char)(c+1);
	auto it = fileLocAliasList.lower_bound(orig);
	auto jt = fileLocAliasList.lower_bound(input);
	return {it, jt};
}

void acceptClient(int new_socket)	
{
	//cout << "waiting for client request " << countx << endl;
        //countx++;
	char buffer[1024] = {0};
	//char sen[1024];
    read( new_socket , buffer, 1024);
    string s(buffer);
    
    if(s.find("#1#") == 0)	// SEARCH COMMAND HANDLER
    {
    	string t = s.substr(3, s.size()-3);

	    auto Pair = database.searchPrefix(t);
	    
	    auto it = Pair.first;
	    auto jt = Pair.second;

	    string searchResult;
		while(it != jt)
		{
			searchResult += it->first;
			searchResult += ":";
			searchResult += it->second.first;
			searchResult += ":";
			searchResult += it->second.second;
			searchResult += ":";
			searchResult += database.clientList[it->second.second][0];
			searchResult += ":";
			searchResult += database.clientList[it->second.second][2];
			searchResult += "#";
			it++;
		}
		send(new_socket , searchResult.c_str() , strlen(searchResult.c_str()) , 0 );
		cout << "end";
	}
	else if(s.find("#2#") == 0)	// SHARE COMMAND HANDLER
	{
		string t = s.substr(3, s.size()-3);
		string fileName, path, alias;
		int loc = 0;
		while(t[loc] != '#')
			fileName += t[loc++];
		loc++;
		while(t[loc] != '#')
			path += t[loc++];
		loc++;
		while(loc < t.size())
			alias += t[loc++];

		database.fileLocAliasList.insert({fileName, {path, alias}});	// INSERTING NEW ELEMENT IN MAIN REPO DS

		database.updateRepo();											// UPDATING FILE ACCORDING TO DS

		

	}
	else if(s.find("#3#") == 0)	// DELETE COMMAND HANDLER
	{
		string t = s.substr(3, s.size()-3);
		string fileName, path, alias;
		int loc = 0;
		while(t[loc] != '#')
			fileName += t[loc++];
		loc++;
		while(t[loc] != '#')
			path += t[loc++];
		loc++;
		while(loc < t.size())
			alias += t[loc++];

		auto Pair = database.searchPrefix(fileName);
		auto it = Pair.first;
		auto jt = Pair.second;

		while(it != jt)
		{
			if(it->first == fileName && it->second.first == path && it->second.second == alias )
				break;

			it++;
		}
		if(it != jt)
		{
			database.fileLocAliasList.erase(it);
			database.updateRepo();
		}

			
	}
	else			//	UPDATING CLIENT DATA ON CLIST LIST WHEN A CLIENT JOINS NETWORK FOR THE FIRST TIME
	{
		string aliasEntries[4];
		int p = 0;
		for(unsigned int i = 0; i < s.size(); i++)
		{
			if(s[i] == '#')
			{
				p++;
			}
			else
			{
				aliasEntries[p] += s[i];
			}
		}
		if(database.clientList.find(aliasEntries[0]) == database.clientList.end())
		{
			//cout << "\n" << aliasEntries[0] << " " << aliasEntries[1] << " " << aliasEntries[2] << " " << aliasEntries[3];
			database.clientList[aliasEntries[0]] = vector<string>({aliasEntries[1], aliasEntries[2], aliasEntries[3]});
		}
		database.updateRepo(); // test to write database on files
	}
	close(new_socket);
}

void server()//Database &database)
{
	int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( CRS_PORT );	// CRS PORT
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    

    vector<thread> uploadThreads;

    while(true)
    {

        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        uploadThreads.push_back(thread(acceptClient, new_socket));
    }

    for(unsigned int i = 0; i < uploadThreads.size(); i++)
    {
        uploadThreads[i].join();
    }
}

int main(int argc, char ** argv)
{
	CRS_PORT = atoi(argv[2]);
	ROOT = string(argv[5]);
	repofilename = string(argv[3]);
	aliasfilename = string(argv[4]);
	database.initialize(ROOT + '/' + string(argv[3]), ROOT + '/' + string(argv[4]));
	server();
}