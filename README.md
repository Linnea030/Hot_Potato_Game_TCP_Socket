# Hot_Potato_Game_TCP_Socket
Developed a pair of programs that will interact to model a game, which is described below:

The game is simple, but the assignment will give you hands-on practice with creating a multi-process application, processing command line arguments, setting up and monitoring network communication channels between the processes (using TCP sockets), and reading/writing information between processes across sockets.

The server program is invoked as:

`ringmaster <port_num> <num_players> <num_hops>`

(example: ./ringmaster 1234 3 100)

The player program is invoked as:

`player <machine_name> <port_num>`

(example: ./player vcm-xxxx.vm.duke.edu 1234)
