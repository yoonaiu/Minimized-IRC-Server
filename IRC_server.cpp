// #define FD_SETSIZE 3000
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>/* 亂數相關函數 */
#include <string.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <cstdint> 
// #include <string.h>
#include <time.h> /* 時間相關函數 */
#include <map>
#include "function.h"
#include <vector>
#include <set>
#include <string>
#include <sstream>
#define MAX 80
#define MAXLINE 10000
#define SA struct sockaddr
#define LISTENQ 1024
using namespace std;

class client_info {
public:
	int connfd;
	struct sockaddr_in connect_info;
	string nick_name;
    bool nick_sent;
    bool user_sent;
    bool welcome_msg_sent;
    bool registered;

    client_info(){}  // map[i] 沒東西的話會 call 到這個，所以這個沒有的時候編譯編不過

    client_info(int connfd_in, struct sockaddr_in connect_info_in){
        connfd = connfd_in;
        connect_info = connect_info_in;
        nick_sent = false;
        user_sent = false;
        welcome_msg_sent = false;
    }

};

// TODO-1 motd
void send_welcome_msg(client_info client, int cur_user_num){
    string sent_msg;
    sent_msg = ":mircd 001 " + client.nick_name + " :Welcome to the minimized IRC daemon!\n"
             + ":mircd 251 " + client.nick_name + " :There are " + to_string(cur_user_num) + " users and 0 invisible on 1 server\n"
             + ":mircd 375 " + client.nick_name + " :- mircd Message of the day -\n"
             + ":mircd 372 " + client.nick_name + " :-  Hello, World!\n"
             + ":mircd 372 " + client.nick_name + " :-               @                    _ \n"
             + ":mircd 372 " + client.nick_name + " :-   ____  ___   _   _ _   ____.     | |\n"
             + ":mircd 372 " + client.nick_name + " :-  /  _ `'_  \\ | | | '_/ /  __|  ___| |\n"
             + ":mircd 372 " + client.nick_name + " :-  | | | | | | | | | |   | |    /  _  |\n"
             + ":mircd 372 " + client.nick_name + " :-  | | | | | | | | | |   | |__  | |_| |\n"
             + ":mircd 372 " + client.nick_name + " :-  |_| |_| |_| |_| |_|   \\____| \\___,_|\n"
             + ":mircd 372 " + client.nick_name + " :-  minimized internet relay chat daemon\n"
             + ":mircd 372 " + client.nick_name + " :-\n"
             + ":mircd 376 " + client.nick_name + " :End of message of the day\n";

    Write(client.connfd, sent_msg.c_str(), sent_msg.length());
}

// TODO-2  /list
void list_channel(client_info client, map<string, set<int>> channel_map, map<string, string> channel_topic){
    
    string sent_msg = ":mircd 321 " + client.nick_name + " Channel :Users Name\n";
    for(auto x : channel_map) {
        sent_msg += ":mircd 322 " + client.nick_name + " " + x.first + " " + to_string(x.second.size()) + " :" + channel_topic[x.first] + "\n";
    }
    sent_msg += ":mircd 323 " + client.nick_name + " :End of List\n";

    Write(client.connfd, sent_msg.c_str(), sent_msg.length());
}

// TODO-3  /join
void join_msg(client_info client, string channel_name, map<string, set<int>> channel_map, map<string, string> channel_topic, map<int, client_info> client_map){
    string sent_msg = ":" + client.nick_name + " JOIN " + channel_name + "\n";
    if(channel_topic[channel_name] == "") sent_msg += ":mircd 331 " + client.nick_name + " " + channel_name + " :No topic is set\n";
    else sent_msg += ":mircd 332 " + client.nick_name + " " + channel_name + " :" + channel_topic[channel_name] + "\n";
    set<int> user_in_channel = channel_map[channel_name];
    sent_msg += ":mircd 353 " + client.nick_name + " " + channel_name + " :";
    for(auto x : user_in_channel){
        sent_msg += client_map[x].nick_name + " ";
    }
    sent_msg += "\n";
    sent_msg += ":mircd 366 " + client.nick_name + " " + channel_name + " :" + "End of Names List\n";
    Write(client.connfd, sent_msg.c_str(), sent_msg.length());
   
    cout << sent_msg;
}

// TODO-4  /users
string create_space(int n) {
    string result = "";
    for(int j = 0; j < n; j++) result += " ";
    return result; 
}

void list_all_users(int id, map<int, client_info> client_map){
    string sent_msg = ":mircd 392 " + client_map[id].nick_name 
                    + " :USERID" + create_space(33-6) 
                    + "Terminal  Host\n";
    for(auto x : client_map){
        sent_msg += ":mircd 393 " + client_map[id].nick_name 
                  + " :" + x.second.nick_name + create_space(33-(x.second.nick_name.length()))
                  + "-" + create_space(9) + inet_ntoa(x.second.connect_info.sin_addr) + "\n";
    }
    sent_msg += ":mircd 394 " + client_map[id].nick_name + " :End of users\n";

    Write(client_map[id].connfd, sent_msg.c_str(), sent_msg.length());
}

// TODO-5  /topic
// get and set the channel topic


void tokenize(string const &str, const char delim, vector<string> &out)
{
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}

string clear(string s){
    string new_s = "";
    for(int i = 0; i < s.length(); i++){
        if(s[i] != '\n' && s[i] != '\r') new_s += s[i];
    }
    return new_s;
}

bool is_cmd(string cmd){
    set<string> cmd_set = {"NICK", "USER", "PING", "LIST", "JOIN", "TOPIC", "NAMES", "PART", "USERS", "PRIVMSG", "QUIT"};
    if(cmd_set.find(cmd) == cmd_set.end()) return false;  // cmd not found
    else return true;
}

void broadcast(string msg, int client[FD_SETSIZE], int maxi, int skip, fd_set rset, fd_set allset, int maxfd){

    for(int i = 0; i <= maxi; i++){
        if(client[i] == -1 || i == skip) continue;
        send(client[i], msg.c_str(), msg.length(), MSG_NOSIGNAL);
    }
}

int main(int argc, char **argv) {
	int					i, maxi, maxfd, listenfd, connfd, sockfd;
	int					nready, client[FD_SETSIZE];  // FD_SETSIZE is 1024, can handle 1000 client
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
    uint16_t SERV_PORT = atoi(argv[1]);
	map<int, client_info> client_map; // index in client array <-> client info
    map<string, set<int>> channel_map; // key: channel name, value: set of client id(i)
    map<string, string> channel_topic; // key: channel name, value: channel topic

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

/* end fig01 */

/* include fig02 */
	for ( ; ; ) {
		rset = allset;		/* structure assignment */
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);


        // PART1 - new client connection
		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
            int connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
#ifdef	NOTDEF
			printf("new client: %s, port %d\n",
					Inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
					ntohs(cliaddr.sin_port));
#endif
            // record the connfd of the client
			for (i = 0; i < FD_SETSIZE; i++){
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
                    
                    client_info tmp = client_info(connfd, cliaddr);
					client_map.insert(pair<int, client_info>(i, tmp));

                    break;
				}
            }

			if (i == FD_SETSIZE){
				cerr << "too many clients" << endl;
				exit(0);
			}

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */ // -> 處理好一個人了，如果只有他那不用跑下面的 for
		}

        // PART2 - handle old connection
        //         read return number
        //         == 0   leave
        //         == -1  read fail (maybe cause by the client leave the server -> can set client[i] to -1)
        //         > 0    read success
		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) { // check descriptor is ready -> leave, (broken), connect to host
				int n;
				memset(buf, 0, sizeof(buf));
				n = read(sockfd, buf, MAXLINE);

				if (n == 0) {  // someone leave -> collect the broadcast message and send to all user still online after the for loop
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
                    client_map.erase(i); // erase by key i

				} else if (n == -1){
					cout << "read fail" << endl;
                    char error_buf[256];
                    memset(error_buf, 0, sizeof(error_buf));
                    perror(error_buf);

				} else {
                    string str_user_input = buf;
                    // TODO-1 NICK & USER (client arrive message)
                    // cout << str_user_input << "|";
                    cout << "user input: " << str_user_input;
                    cout << "input length: " << str_user_input.length() << endl;

                    // 用空白切 input 存進 vector -> 比較好用
                    vector<string> input_vec;
                    input_vec.clear();
                    tokenize(clear(str_user_input), ' ', input_vec);

                    if(is_cmd(input_vec[0]) == false) {  // (421) ERR_UNKNOWNCOMMAND
                        string sent_msg = ":mircd 421 " + client_map[i].nick_name + " " + input_vec[0] + " :Unknown command\n";
                        Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());
                        continue; // can continue here to process next avaliable client
                    }

                    // if ( str_user_input.substr(0, 4) == "NICK") {
                    if( input_vec[0] == "NICK" ){
                        if(input_vec.size() == 1) {  // only 1 para 'NICK' and no nick_name given -> (431) ERR_NONICKNAMEGIVEN
                            string sent_msg = ":mircd 431 :No nickname given\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        } else {
                            
                            bool nick_name_used = false;
                            for(auto x : client_map) {
                                // x.first  -> i
                                // x.second -> client_info 
                                cout << "x.second.nick_name: " << x.second.nick_name << endl;
                                cout << "x.second.nick_name length: " << x.second.nick_name.length() << endl;
                                if(x.second.nick_name == input_vec[1]){
                                    nick_name_used = true;
                                    break;
                                }
                            }

                            cout << "nick_name: " << input_vec[1] << endl;
                            if(nick_name_used == true) {   // nick_name already been used (436) ERR_NICKCOLLISION
                                string sent_msg = ":mircd 436 " + input_vec[1] + " :Nickname collision KILL\n";
                                Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());
                                cout << "collision!!!!" << endl;

                            } else {
                                // string nick_name = str_user_input.substr(5, str_user_input.length()-7);  // start, range
                                string nick_name = input_vec[1];
                                client_map[i].nick_name = nick_name;
                                client_map[i].nick_sent = true;
                            }

                        }
                    }
                    // if (str_user_input.substr(0, 4) == "USER") {
                    if( input_vec[0] == "USER" ){
                        if(input_vec.size() < 4){  // (461) ERR_NEEDMOREPARAMS
                            string sent_msg = ":mircd 461 " + client_map[i].nick_name + " USER :Not enought parameters\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        } else {
                            client_map[i].user_sent = true;
                        }
                    
                    }
                    // if NICK & USER both sent -> write welcome msg to the client
                    if (client_map[i].welcome_msg_sent == false && (client_map[i].user_sent && client_map[i].nick_sent)){
                        send_welcome_msg(client_map[i], client_map.size());
                        client_map[i].registered = true;
                        client_map[i].welcome_msg_sent = true;
                    }

                    // TODO-2 LIST
                    // input_vec[1]: LIST
                    // input_vec[1]: #cha
                    if (input_vec[0] == "LIST") {
                        cout << "list case!!" << endl;
                        if( input_vec.size() == 1 ) {
                            list_channel(client_map[i], channel_map, channel_topic);

                        } else if ( input_vec.size() == 2 ) {
                            
                            string sent_msg = ":mircd 321 " + client_map[i].nick_name + " Channel :Users Name\n";
                            sent_msg += ":mircd 322 " + client_map[i].nick_name+ " " + input_vec[1] + " " + to_string(channel_map[input_vec[1]].size()) + " :" + channel_topic[input_vec[1]] + "\n";
                            sent_msg += ":mircd 323 " + client_map[i].nick_name + " :End of List\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        }
                        
                        
                    }

                    // TODO-3 JOIN
                    // if (str_user_input.substr(0, 4) == "JOIN") {
                    if(input_vec[0] == "JOIN") {
                        if(input_vec.size() < 2){  // (461) ERR_NEEDMOREPARAMS
                            string sent_msg = ":mircd 461 " + client_map[i].nick_name + " JOIN :Not enought parameters\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        } else {
                            // string channel_name = str_user_input.substr(5, str_user_input.length()-7);
                            string channel_name = input_vec[1];
                            cout << "channel name " << channel_name << endl;
                            cout << "channel name length " << channel_name.length() << endl;
                            if(channel_name.substr(0, 1) != "#") channel_name = "#" + channel_name;
                            if(channel_map.find(channel_name) == channel_map.end()){
                                // not found the channel -> create one
                                set<int> member;
                                member.clear();
                                member.insert(i);
                                // channel_map.insert((pair<string, vector<int>>(channel_name, member)));
                                channel_map[channel_name] = member;
                                channel_topic[channel_name] = "";
                            } else {
                                channel_map[channel_name].insert(i);
                            }
                            cout << "join success" << endl;
                            join_msg(client_map[i], channel_name, channel_map, channel_topic, client_map);
                        }
                    }

                    // TODO-4 USERS
                    // if (str_user_input.substr(0, 5) == "USERS") {
                    if( input_vec[0] == "USERS" ){
                        // list all users on the server
                        list_all_users(i, client_map);
                    }

                    // TODO-5 TOPIC
                    // if (str_user_input.substr(0, 5) == "TOPIC") {
                    if( input_vec[0] == "TOPIC" ){
                    
                        // input_vec[0] TOPIC
                        // input_vec[1] #cha
                        // input_vec[2] :tv_show

                        // *** can only set / show the topic when the user is in a channel
                        if(input_vec.size() < 2){  // (461) ERR_NEEDMOREPARAMS
                            string sent_msg = ":mircd 461 " + client_map[i].nick_name + " TOPIC :Not enought parameters\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        } else {
                            
                            set<int> channel_user = channel_map[input_vec[1]];
                            if(channel_user.find(i) == channel_user.end()) {  // (442) ERR_NOTONCHANNEL 
                                string sent_msg = ":mircd 442 " + client_map[i].nick_name + " " + input_vec[1] + " :You are not on that channel\n";
                                Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                            } else {
                                if(input_vec.size() == 2) {  // TOPIC #cha -> return the topic of the channel
                                    string sent_msg;
                                    if(channel_topic[input_vec[1]] == ""){
                                        sent_msg = ":mircd 331 " + client_map[i].nick_name + " " + input_vec[1] + " :" + "No topic is set\n";
                                    }else{
                                        sent_msg = ":mircd 332 " + client_map[i].nick_name + " " + input_vec[1] + " :" + channel_topic[input_vec[1]] + "\n";
                                    }
                                    Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                                } else if (input_vec.size() >= 3) {   // TOPIC #cha :tv_show -> set the topic of the channel(topic 可以有空格)
                                    input_vec[2] = input_vec[2].substr(1, str_user_input.length()-1);  // 去掉前面的冒號
                                    string topic = "";
                                    for(int j = 2; j < input_vec.size(); j++) {
                                        if(j == input_vec.size() -1) topic += input_vec[j];
                                        else topic += input_vec[j] + " ";
                                    }

                                    channel_topic[input_vec[1]] = topic;  // change the topic
                                    string sent_msg = ":mircd 332 " + client_map[i].nick_name + " " + input_vec[1] + " :" + channel_topic[input_vec[1]] + "\n"; // return the new topic of the channel
                                    Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());
                                }
                            }

                        }

                    }

                    // TODO-6 PART  (/part, /close)
                    if(input_vec[0] == "PART") {
                        // PART #chal1 :WeeChat 3.5
                        // vec[0]  PART
                        // vec[1]  #chal1
                        // vec[2]  :WeeChat 3.5
                        if(input_vec.size() < 2){  // (461) ERR_NEEDMOREPARAMS
                            string sent_msg = ":mircd 461 " + client_map[i].nick_name + " PART :Not enought parameters\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        } else if(channel_map.find(input_vec[1]) == channel_map.end()) { // (403) ERR_NOSUCHCHANNEL
                            string sent_msg = ":mircd 403 " + client_map[i].nick_name + " " + input_vec[1] + " :No such channel\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        } else {

                            set<int> channel_user = channel_map[input_vec[1]];
                            if(channel_user.find(i) == channel_user.end()) {  // not in the channel -> can not leave the channel -> (442) ERR_NOTONCHANNEL
                                string sent_msg = ":mircd 442 " + client_map[i].nick_name + " " + input_vec[1] + " :You are not on that channel\n";
                                Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                            } else {  // in the channel -> 辦離開手續
                                channel_map[input_vec[1]].erase(i);
                                string sent_msg = ":" + client_map[i].nick_name + " PART :" + input_vec[1] + "\n";
                                Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());
                            }

                        }
                    }

                    // TODO-7 QUIT
                    if(input_vec[0] == "QUIT") {
                        // close the connection of the client
                        // 1. remove the client from all channel
                        // 2. remove the client from the client map
                        // 3. set the client[i] to -1
                        // 4. close socket
                        for(auto x : channel_map){
                            // x.second is a set to record the clients
                            if(x.second.find(i) != x.second.end()) {
                                cout << "Erase " << client_map[i].nick_name << " out from channel " << x.first << endl;
                                channel_map[x.first].erase(i);  // erase from the original map, from x 是錯ㄉ
                            }
                        }
                        client_map.erase(i);

                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = -1;

                        cout << "successfully close" << endl;
                    }

                    // TODO-8 PING -> reponse pong(?) -> 應該是，看起來是過了 300 秒不回訊息 weechat 就會斷掉連線
                    // PING localhost -> PONG localhost
                    if(input_vec[0] == "PING") {
                        string sent_msg;
                        if(input_vec.size() == 1) {
                            sent_msg = "mircd 409 :No origin specified\n";
                        } else {
                            cout << input_vec[0] << " " << input_vec[1] << endl;
                            sent_msg = "PONG " + input_vec[1] + "\n";
                            cout << sent_msg;
                        }
                        Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());
                    }

                    // TODO-9 PRIVMSG
                    // input_vec[0]  PRIVMSG
                    // input_vec[1]  #cha7
                    // input_vec[2]  :ing
                    if(input_vec[0] == "PRIVMSG") {
                        cout << "private msg case" << endl;
                        string msg;
                        cout << "input_vec size: " << input_vec.size() << endl;

                        if(input_vec.size() == 1){  // only 1 para, which is 'PRIVMSG', // (411) ERR_NORECIPIENT
                            string sent_msg = ":mircd 411 " + client_map[i].nick_name + " :No recipient given (PRIVMSG)\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        }else if(input_vec.size() == 2){  // only 2 para, which is 'PRIVMSG, #cha7', // (412) ERR_NOTEXTTOSEND
                            string sent_msg = ":mircd 412 " + client_map[i].nick_name + " :No text to send\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        }else if(channel_map.find(input_vec[1]) == channel_map.end()) { // (401) ERR_NOSUCHNICK
                            string sent_msg = ":mircd 401 " + client_map[i].nick_name + " " + input_vec[1] + " :No such nick/channel\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        } else {  // the channel exit -> normal case
                            for(int j = 2; j < input_vec.size(); j++) {
                                if(j == 2 && j != input_vec.size()-1) msg += input_vec[2].substr(1, input_vec[2].length()-1) + " ";
                                else if(j == 2 && j == input_vec.size()-1) msg += input_vec[2].substr(1, input_vec[2].length()-1) + "\n";
                                else if(j == input_vec.size()-1) msg += input_vec[j] + "\n";
                                else msg += input_vec[j] + " ";
                            }
                            cout << "msg: " << msg;

                            string sent_msg;
                            for(auto x : channel_map[input_vec[1]]) {  // channel_map[input_vec[1]]: client 'set' in the channel
                                cout << "x: " << x << endl;
                                // x is client's i
                                if(x == i) continue;
                                //               <是下送的人的 nick_name>
                                sent_msg = ":" + client_map[i].nick_name + " PRIVMSG " + input_vec[1] + " :" + msg;
                                Write(client_map[x].connfd, sent_msg.c_str(), sent_msg.length());
                                cout << "sent_msg of channel: " << sent_msg;
                            }
                        }
                    }

                    // TODO-10 NAMES
                    // NAMES
                    // #cc
                    if(input_vec[0] == "NAMES") {
                        cout << "** names case **" << endl;
                        // 353 & 366 跟 join 一樣
                        cout << "channel name: " << input_vec[1] << endl;
                        cout << "channel name length(): " << input_vec[1].length() << endl;

                        string sent_msg;
                        if(input_vec.size() == 2){  // with appended channel parameter(1)
                            sent_msg = ":mircd 353 " + client_map[i].nick_name + " " + input_vec[1] + " :";
                            // cout << "in names caller nick_name: " << client_map[i].nick_name << endl;
                            for(auto x : channel_map[input_vec[1]] ) {
                                sent_msg += client_map[x].nick_name + " ";
                            }
                            sent_msg += "\n";
                            sent_msg += ":mircd 366 " + client_map[i].nick_name + " " + input_vec[1] + " :End of Names List\n";
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());

                        } else {
                            sent_msg = "";
                            for(auto channel : channel_map){
                                // channel.first -> channel_name
                                // channel.second -> clients' id in the channel(set)
                                sent_msg += ":mircd 353 " + client_map[i].nick_name + " " + channel.first + " :";
                                for(auto x : channel.second ) {
                                    sent_msg += client_map[x].nick_name + " ";
                                }
                                sent_msg += "\n";
                                sent_msg += ":mircd 366 " + client_map[i].nick_name + " " + channel.first + " :End of Names List\n";
                            }
                            Write(client_map[i].connfd, sent_msg.c_str(), sent_msg.length());
                        }
                        
                    }
                    
				}

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
        // sleep(0.01);
	}
}
/* end fig02 */
