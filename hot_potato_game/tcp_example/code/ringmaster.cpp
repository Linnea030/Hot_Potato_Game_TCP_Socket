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

//the socket function is referred to ECE650 tcp_example code
class Master{
public:
    int port_num;                   //the port number
    //const char* port;             //the port number in char
    int num_players;                //the number of players
    int num_hops;                   //the number of hops
    int fd_master;                  //master's fd

    std::vector<int> fd_players;    //players socket fd
    std::vector<int> port_players;  //players port number as server
    std::vector<std::string> ip_players;    //players ip

    Master(int argv1, int argv2, int argv3) {
        //check validness of input arguments
        validCheck(argv1, argv2, argv3);

        //initilize arguments
        port_num = argv1;
        num_players = argv2;
        num_hops = argv3;

        //initilize vector of players info
        fd_players.clear();
        port_players.clear();
        ip_players.clear();
        //fd_players.resize(num_players);
        //port_players.resize(num_players);
        //ip_players.resize(num_players);
    }

    ~Master() {}
    
    void validCheck(int argv1, int argv2, int argv3) {
       if(argv1 < 1024 || argv1 > 65535){
            std::cout<<"invalid port number!\n";
            exit(EXIT_FAILURE);
        }
        if(argv2 <= 1) {
            std::cout<<"No enough players!\n";
            exit(EXIT_FAILURE);
        }
        if(argv3 < 0 || argv3 > 512) {
            std::cout<<"invalid hops number!\n";
            exit(EXIT_FAILURE);            
        }
    }
    void init_print() {
        std::cout<<"Potato Ringmaster\n";
        std::cout<<"Players = "<<num_players<<"\n";
        std::cout<<"Hops = "<<num_hops<<"\n";
    }

    void print_player(int x) {
        std::cout<<"Player "<<x<<" is ready to play\n";
    }

    void print_begin(int x) {
        std::cout<<"Ready to start the game, sending potato to player "<<x<<"\n";
    }

    void print_player_port() {
        std::cout<<"port: ";
        for(int i = 0; i < num_players; i++) {
            std::cout<<port_players[i]<<",";
        }
        std::cout<<"\n";
    }

    void print_player_ip() {
        std::cout<<"ip: ";
        for(int i = 0; i < num_players; i++) {
            std::cout<<ip_players[i]<<",";
        }
        std::cout<<"\n";
    }

    void print_player_fd() {
        std::cout<<"fd: ";
        for(int i = 0; i < num_players; i++) {
            std::cout<<fd_players[i]<<",";
        }
        std::cout<<"\n";
    }


    void master_init() {
        init_print();
        int status;
        int socket_fd;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;

        memset(&host_info, 0, sizeof(host_info));

        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        host_info.ai_flags    = AI_PASSIVE;

        //trans int to const char*
        std::stringstream ss;
        ss<<port_num;
        std::string port_num_s = ss.str();
        char * port_num_c = new char[port_num_s.length() + 1];
        std::strcpy(port_num_c, port_num_s.c_str());

        status = getaddrinfo(NULL, port_num_c, &host_info, &host_info_list);
        if (status != 0) {
            std::cerr << "Error: cannot get address info for host\n";
            exit(EXIT_FAILURE);
            delete[] port_num_c;  //free memory of path
        } //if

        delete[] port_num_c;  //free memory of path

        socket_fd = socket(host_info_list->ai_family, 
                    host_info_list->ai_socktype, 
                    host_info_list->ai_protocol);
        if (socket_fd == -1) {
            std::cerr << "Error: cannot create socket\n";
            exit(EXIT_FAILURE);
        } //if
        fd_master = socket_fd;

        int yes = 1;
        status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
            std::cerr << "Error: cannot bind socket\n";
            exit(EXIT_FAILURE);
        } //if

        status = listen(socket_fd, 100);
        if (status == -1) {
            std::cerr << "Error: cannot listen on socket\n"; 
            exit(EXIT_FAILURE);
        } //if

        //connect to each player
        for(int i = 0; i < num_players; i++) {
            struct sockaddr_storage socket_addr_p;
            socklen_t socket_addr_len = sizeof(socket_addr_p);
            int new_fd;
            new_fd = accept(socket_fd, (struct sockaddr *)&socket_addr_p, &socket_addr_len);
            if (new_fd == -1) {
                std::cerr << "Error: cannot accept connection on socket\n";
                exit(EXIT_FAILURE);
            } //if
            
            //save info of player to master
            struct sockaddr_in * portIpInfo = (struct sockaddr_in *) &socket_addr_p;
            char* ip_c = inet_ntoa(portIpInfo->sin_addr);
            std::string ip_s(ip_c);
            ip_players.push_back(ip_s);
            fd_players.push_back(new_fd);

            //receive info from player
            int port_playerS;
            recv(new_fd, &port_playerS, sizeof(port_playerS), MSG_WAITALL);//receive port of player as server A
            port_players.push_back(port_playerS);
            
            //test!!!
            //std::cout<<"after send port to player: "<<port_playerS<<"\n";

            //send info to player
            //send(new_fd, &fd_master, sizeof(fd_master), 0);   //send master fd to player B
            send(new_fd, &i, sizeof(i), 0);                     //assign id to player C
            send(new_fd, &num_players, sizeof(num_players), 0); //send num_players to player D
            send(new_fd, &num_hops, sizeof(num_hops), 0);       //send num_players to player E

            //print Player <number> is ready to play
            print_player(i);
        }
        //print_player_fd();
        //print_player_ip();
        //print_player_port();

        freeaddrinfo(host_info_list);
////////////////////////////////////////////////////
        //send info to player
        for(int j = 0; j < num_players; j++) {
            if(j != num_players - 1) {
                //test!!!
                //std::cout<<"\n Sending to player "<<j<<"\n";

                //int flag;
                //send right player port as server to player
                send(fd_players[j], &port_players[j + 1], sizeof(port_players[j + 1]), 0);  //F
                //test!!!
                //std::cout<<"flag "<<flag<<"\n"; 
                
                //trans string to char*
                //send right player ip address to player
                int len = ip_players[j + 1].length();
                send(fd_players[j], &len, sizeof(len), 0); //G
                //test!!!
                //std::cout<<"len "<<len<<"\n";

                char ip_addr_c[512];
                memset(ip_addr_c, 0, sizeof(ip_addr_c));
                std::strcpy(ip_addr_c, ip_players[j + 1].c_str());

                //test!!!
                //std::cout<<"before send ip: "<<ip_addr_c<<"\n";

                int flag_s;
                flag_s = send(fd_players[j], &ip_addr_c, ip_players[j + 1].length(), 0);      //H
                //test!!!
                //std::cout<<"flag_s = "<<flag_s<<" after send ip: "<<ip_addr_c<<"\n";
                //delete[] ip_addr_c;  //free memory
                //send the ip addr size
                //send(fd_arr[i],&ip_len,sizeof(ip_len),0);
            }
            else {
                //test!!!
                //std::cout<<"\n Sending to player "<<j<<"\n";
                //send right player port as server to player
                send(fd_players[j], &port_players[0], sizeof(port_players[0]), 0);  
                
                //trans string to char*
                //send right player ip address to player
                int len = ip_players[0].length();
                send(fd_players[j], &len, sizeof(len), 0); //G
                //test!!!
                //std::cout<<"len "<<len<<"\n";

                char ip_addr_c[512];
                memset(ip_addr_c, 0, sizeof(ip_addr_c));
                std::strcpy(ip_addr_c, ip_players[0].c_str());

                //test!!!
                int flag_s;
                //std::cout<<"before send ip: "<<ip_addr_c<<"\n";
                flag_s = send(fd_players[j], &ip_addr_c, ip_players[0].length(), 0); 
                
            }

        }
    }

    void master_run() {
        //int status;
        //get random number
                    //test!!!
        //std::cout<<"in master_run\n"; 
        srand((unsigned int)time(NULL) + num_players);
        int random = rand() % num_players;

        //initailiza a potato
        Potato potato;
        potato.hops = num_hops;
        potato.total_hops = num_hops;
        //if hops == 0
        if(potato.hops == 0) {
            close_fds();
            return;
        }

        //print message
        print_begin(random);
        //send potato to player[random]
        send(fd_players[random], &potato, sizeof(potato), 0);

//        while(1) {
            //minor if potato is sent back
            fd_set readfds;
            FD_ZERO(&readfds);
            //get max fd of this vector
            int maxfd = fd_players[num_players - 1];
            for(int i = 0; i < num_players; i++ ) {
                FD_SET(fd_players[i], &readfds);
            }
            select(maxfd + 1, &readfds, NULL, NULL, NULL);
            int x = 0;
            while(x < num_players) {
                if (FD_ISSET(fd_players[x], &readfds)) {
                    //get potato from player
                    recv(fd_players[x], &potato, sizeof(potato), MSG_WAITALL);
                    //potato.print_potato();
                    //close all fds
                    close_fds();
                    //print track pf potato
                    potato.print_track();
                    return;
                }
                x++;
            }
//       }
    }

    void close_fds() {
        //get an potato with close flag
        Potato isClose;
        isClose.total_hops = 0;
        isClose.hops = 0;
        //send message to player to let them close
        int flag;
        for (int i = 0; i < num_players; i++) {
            flag = send(fd_players[i], &isClose, sizeof(isClose), 0);
            //std::cout<<"flag for send empty "<<flag<<"\n";
        }
        //close player port as server
        for(int j = 0; j < num_players; j++) {
            close(fd_players[j]);
        }
        //close master port
        close(fd_master);
    }
};

int main(int argc, char *argv[]) {
    //if invalid input
    if(argc != 4) {
        std::cout<<"Invalid input!\n";
        std::cout<<"Please use: ./ringmaster <port_num> <num_players> <num_hops>\n";
        return EXIT_FAILURE;
    }
    int argv1 = atoi(argv[1]);
    int argv2 = atoi(argv[2]);
    int argv3 = atoi(argv[3]);

    //create Master object
    Master m(argv1, argv2, argv3);
    //init master
    m.master_init();
    //run master
    m.master_run();

    return EXIT_SUCCESS;
}
