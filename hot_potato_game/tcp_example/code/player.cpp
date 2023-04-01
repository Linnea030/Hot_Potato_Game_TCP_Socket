#include "potato.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

//the socket function is referred to ECE650 tcp_example code
class Player{
public:
    const char *hostname_master;           //ringmaster hostname
    const char *port_master;               //ringmaster port
    const char *hostname_player;           //player hostname

    int id_num;                     //players' id
    int fd_self;                    //players' listen fd
    int fd_master;                  //master's fd
    int fd_left;                    //left player's fd
    int fd_right;                   //right player's fd
    int num_players;                //number of players
    int num_hops;                   //number of hops

    int port_server;                //player's port as server
    std::string ip_right;           //right player's ip
    int port_right;                 //right player's port

    Player(char * argv1, char * argv2){
        hostname_master = argv1;
        port_master = argv2;
    }

    void print_connect(int x) {
        std::cout<<"Connected as player "<<x<<" out of "<<num_players<<" total players\n";
    }

    void print_send(int x) {
        std::cout<<"Sending potato to "<<x<<"\n";
    }

    void print_end() {
        std::cout<<"I'm it\n";
    }

    const char * get_host() {
        int status;
        char * hname;
        status = gethostname(hname, sizeof(hname));
        if (status == -1) {
            std::cerr << "Error: cannot find hostname\n";
            exit(EXIT_FAILURE);
        } //if
        return hname;
    }
    
    void get_port() {
        int status;
        struct sockaddr_in addr_local;
        int sock = 0;
        sock++;
        socklen_t len = sizeof(addr_local);
        status = getsockname(fd_self, (struct sockaddr*) &addr_local, &len);
        //cout<<"get port after\n";  
        if (status == -1) {
            std::cout << "cannot find port\n";
            exit(EXIT_FAILURE);
        } //if
        port_server = ntohs(addr_local.sin_port);
    }

    void player_init_server() {
        //test!!!
        //std::cout<<"in player_init_server()\n";
        
        int status;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;
        const char *port = "";

        //hostname_player = get_host();
        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        host_info.ai_flags    = AI_PASSIVE;
        
        //test!!!
        //std::cout<<"before getaddrinfo\n";

        status = getaddrinfo(NULL, port, &host_info, &host_info_list);
        if (status != 0) {
            std::cerr << "Error: getaddrinfo get error\n";
            exit(EXIT_FAILURE);
        } //if

        //test!!!
        //std::cout<<"after getaddrinfo\n";

        //assign port by OS
        struct sockaddr_in * host_list_in = (struct sockaddr_in *)(host_info_list->ai_addr);
        //make sin_port as 0  
        host_list_in->sin_port = 0;
        //test!!!
        //std::cout<<"after assign port\n";

        fd_self = socket(host_info_list->ai_family, 
                    host_info_list->ai_socktype, 
                    host_info_list->ai_protocol);
        if (fd_self == -1) {
            std::cerr << "Error: sockert get error\n";
            std::cerr << "  (" << hostname_player << "," << port << ")\n";
            exit(EXIT_FAILURE);
        } //if

        //test!!!
        //std::cout<<"after socket\n";

        int yes = 1;
        status = setsockopt(fd_self, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        status = bind(fd_self, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
            std::cerr << "Error: bind get error\n";
            std::cerr << "  (" << hostname_player << "," << port << ")\n";
            exit(EXIT_FAILURE);
        } //if

        status = listen(fd_self, 100);
        if (status == -1) {
            std::cerr << "Error: listen get error\n"; 
            std::cerr << "  (" << hostname_player << "," << port << ")\n";
            exit(EXIT_FAILURE);
        } //if

        //test!!!
        //std::cout<<"after listen\n";
        get_port();//get player port as server

        freeaddrinfo(host_info_list);//free memory
    }

    void player_init_cliecnt() {
        int status;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;

        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        //test!!!
        //std::cout<<"before getinfo\n";

        status = getaddrinfo(hostname_master, port_master, &host_info, &host_info_list);
        if (status != 0) {
            std::cerr << "Error: getaddrinfo get error\n";
            std::cerr << "  (" << hostname_master << "," << port_master << ")'n";
            exit(EXIT_FAILURE);
        } //if

        //test!!!
        //std::cout<<"after getinfo\n";
        fd_master = socket(host_info_list->ai_family, 
                    host_info_list->ai_socktype, 
                    host_info_list->ai_protocol);
        // //test!!!
        //std::cout<<"fd_master "<<fd_master<<"\n";
        if (fd_master == -1) {
            std::cerr << "Error: socket get error\n";
            std::cerr << "  (" << hostname_master << "," << port_master << ")\n";
            exit(EXIT_FAILURE);
        } //if

                
        //test!!!
        //std::cout<<"after socket\n";
        status = connect(fd_master, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
            std::cerr << "Error: connect get error\n";
            std::cerr << "  (" << hostname_master << "," << port_master << ")\n";
            exit(EXIT_FAILURE);
        } //if
        //test!!!
        //std::cout<<"after connect\n";

        //send info to master after become server
        send(fd_master, &port_server, sizeof(port_server), 0);//receive port of player as server
        
        //receive info from master
        recv(fd_master, &id_num, sizeof(id_num), MSG_WAITALL);           //recv id to player
        recv(fd_master, &num_players, sizeof(num_players), MSG_WAITALL); //recv num_players from master
        recv(fd_master, &num_hops, sizeof(num_hops), MSG_WAITALL); //recv num_players from master
        print_connect(id_num);

        //recv right player port info
        recv(fd_master, &port_right, sizeof(port_right), MSG_WAITALL); 
        //test!!!
        //std::cout<<"port_right "<<port_right<<"\n"; 

        //trans char* to string
        //recv right player ip address
        int len;
        recv(fd_master, &len, sizeof(len), MSG_WAITALL); 
        //test!!!
        //std::cout<<"len "<<len<<"\n";
        
        //test!!!
        //std::cout<<"before receive ip\n";
        char ip_addr_c[512];
        memset(ip_addr_c,0,sizeof(ip_addr_c));
        int flag_r;

        flag_r = recv(fd_master, &ip_addr_c, len, MSG_WAITALL);
        //test!!!
        std::string ip(ip_addr_c);
        ip_right = ip;

        //test!!!
         //std::cout<<"ip_right "<<ip_right<<"\n";

        //free info list
        freeaddrinfo(host_info_list);
    }

    void connect_neigh() {
        //std::cout<<"in connect_neigh\n";
        //connect to right
        int status;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;

        //trans int to const char*
        std::stringstream ss;
        ss<<port_right;
        std::string port_right_s = ss.str();

        //get port as string
        char port[port_right_s.length() + 1];
        memset(&port, 0, sizeof(port));
        std::strcpy(port, port_right_s.c_str());
        //std::cout<<port;
        
        //get ip as string
        int len = ip_right.length();
        char hostname [len + 1];
        memset(&hostname, 0, sizeof(hostname));
        std::strcpy(hostname, ip_right.c_str());
        //std::cout<<hostname;
        
        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;

        status = getaddrinfo(hostname, port, &host_info, &host_info_list);
        if (status != 0) {
            std::cerr << "Error: getaddrinfo get error\n";
            std::cerr << "  (" << hostname << "," << port << ")\n";
            exit(EXIT_FAILURE);
            //delete[] port;  //free memory
            //delete[] hostname;  //free memory
        } //if
        //delete[] port;  //free memory
        //delete[] hostname;  //free memory

        fd_right = socket(host_info_list->ai_family, 
                    host_info_list->ai_socktype, 
                    host_info_list->ai_protocol);
        if (fd_right == -1) {
            std::cerr << "Error: socket get error\n";
            std::cerr << "  (" << hostname << "," << port << ")\n";
            exit(EXIT_FAILURE);
        } //if
        ///////
        //std::cout << "  (" << hostname << "," << port << ")\n";
        
        status = connect(fd_right, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
            std::cerr << "Error: connect get error\n";
            std::cerr << "  (" << hostname << "," << port << ")\n";
            exit(EXIT_FAILURE);
        } //if

        //accept left
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        fd_left = accept(fd_self, (struct sockaddr *)&socket_addr, &socket_addr_len);
        if (fd_left == -1) {
            std::cerr << "Error: accept get error\n";
            exit(EXIT_FAILURE);
        } //if
                //test!!!
        //std::cout<<"accept to player_fd: "<<fd_left<<" successfully\n";
    }

    void player_run() {
        //test!!!
        //std::cout<<"in player_run\n";

        Potato potato;
        potato.total_hops = num_hops;
        //get random number seed
        srand((unsigned int)time(NULL) + id_num);
        //test!!!
        //std::cout<<"wait\n";
        int i =0;
        while(1) {
            //minor if potato is sent back
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(fd_left, &readfds);
            FD_SET(fd_right, &readfds);
            FD_SET(fd_master, &readfds);
            int max_fd = std::max(fd_master, fd_right);
            int maxi = std::max(max_fd, fd_left);
            select(maxi + 1, &readfds, NULL, NULL, NULL);

            //if receive potato
            if(FD_ISSET(fd_master, &readfds)) {
                //get potato from others
                recv(fd_master, &potato, sizeof(potato), MSG_WAITALL);
                //std::cout<<"get potato from fd_master: "<<fd_master<<"\n";
                //std::cout<<"potato.hops "<<potato.hops<<"\n";
            }
            else if(FD_ISSET(fd_right, &readfds)) {
                //get potato from others
                recv(fd_right, &potato, sizeof(potato), MSG_WAITALL);
                //std::cout<<"get potato from fd_right: "<<fd_right<<"\n";
                //std::cout<<"potato.hops "<<potato.hops<<"\n";
            }
            else if (FD_ISSET(fd_left, &readfds)) {
                //get potato from others
                recv(fd_left, &potato, sizeof(potato), MSG_WAITALL);
                //std::cout<<"get potato from fd_left: "<<fd_left<<"\n";
                //std::cout<<"potato.hops "<<potato.hops<<"\n";
            }

            //std::cout<<"here before if for potato\n";
            //potato.print_potato();

            if(potato.total_hops == 0){//receive close signal
                //std::cout<<"here in potato close\n";
                //std::cout<<"gameover\n";
                break;
            }
            if(potato.hops == 1) {//receive potato with only 1 hops
                //std::cout<<"here in potato hops is 0\n";
                print_end();
                
                potato.track[num_hops - potato.hops] = id_num;
                potato.hops--;

                send(fd_master, &potato, sizeof(potato),0);
                //std::cout<<"send to master fd:"<<fd_master<<"\n";
                //continue;
            }
            else if(potato.hops > 1) {//receive potato and send it to others
                // std::cout<<"here in potato hops > 1\n";
                // std::cout<<"num_hops: "<<num_hops<<"\n";
                // std::cout<<"potato.hops: "<<potato.hops<<"\n";
                // //std::cout<<"potato.track.size(): "<<potato.track.size()<<"\n";
                // std::cout<<"num_hops - potato.hops: "<<num_hops - potato.hops<<"\n";
                // std::cout<<"id_num: "<<id_num<<"\n";
                // //potato.track[num_hops - potato.hops] = id_num;
                // //potato.track.push_back(id_num);
                // //std::cout<<"id_num: "<<id_num<<"\n";

                //potato.hops--;
                potato.track[num_hops - potato.hops] = id_num;
                potato.hops--;

                int random = rand() % 2;
                if(random == 0) {//send it to left
                    //std::cout<<"send to left\n";
                    print_send((id_num - 1 + num_players) % num_players);
                    send(fd_left, &potato, sizeof(potato),0);
                    //std::cout<<"send to player left fd:"<<fd_left<<"\n";
                }
                else if (random == 1) {//send it to right
                    //std::cout<<"send to right\n";
                    print_send((id_num + 1 + num_players) % num_players);
                    send(fd_right, &potato, sizeof(potato),0);
                    //std::cout<<"send to player right fd:"<<fd_right<<"\n";
                }
            }
            i++;
        }
    }
        
    void close_fds() {
        //close the socket
        close(fd_left);
        close(fd_right);
        close(fd_self);
        close(fd_master);   
    }

    ~Player() {
        close_fds();
    }
};

int main(int argc, char *argv[]) {
    //if invalid input
    if(argc != 3) {
        std::cout<<"Invalid input!\n";
        std::cout<<"Please use: ./player <machine_name> <port_num>\n";
        return EXIT_FAILURE;
    }
    //initialize player object
    Player p(argv[1], argv[2]);
    //test!!!
    //std::cout<<"1\n";

    //make player as a server
    p.player_init_server();
    //test!!!
    //std::cout<<"2\n";

    //make player as a client and connect to master
    p.player_init_cliecnt();
    //test!!!
    //std::cout<<"3\n";

    //connect to its right neighber and accept its left neighber
    p.connect_neigh();
    //std::cout<<"4\n";

    //play game
    p.player_run();
    //std::cout<<"5\n";
    //while(1);
    return EXIT_SUCCESS;
}