#include "server.h"
#include "tands.h"
#include "common.h"

using namespace std;

void parseClientMessage(string client_message, string &client_name, string &command, bool &transaction_ended) {
    int index = client_message.find(" ");

    client_name = client_message.substr(0, index);
    command = client_message.substr(index+1);

    if (command[0] != 'T') {
        transaction_ended = true;
    }
}

// code for server functionality
// Reference: https://www.binarytides.com/server-client-example-c-sockets-linux/
//            https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
int start_server(int port) {
    int master_sock, client_len , read_size, max_sd, activity;
    int client_sockets[10];
    int opt = 1;
    int transaction_number = 0;
	struct sockaddr_in server_addr, client_addr;
	char client_message[2000];

    double start_time, current_time;

    char host_name[50];
    pid_t pid;
    char server_name[100];

    FILE* fp = NULL;

    // get host name and pid of this server, then process server name
    gethostname(host_name, sizeof(host_name));
    pid = getpid();
    sprintf(server_name, "%s.%d", host_name, pid);

    // Create server log
    fp = createLog(server_name);

    if (fp == NULL) {
        perror("Create log for server failed 222!");
        return -1;
    }

    // Empty all client sockets 
    for (int i = 0; i < MAX_CLIENT; i++) {
        client_sockets[i] = 0;
    }

    // Create socket
    master_sock = socket(AF_INET, SOCK_STREAM, 0); // TCP/IP protocal
    if (master_sock < 0) {
        perror("Create server socket failed!");
        return -1;  // indicate an error
    }

    // Set master socket to allow multiple connections
    if( setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("Setsockopt failed");  
        return -2;
    } 

    // prepare socket_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    printf("Using port %d\n", port);
    fprintf(fp, "Using port %d\n", port);

    // Bind
    if (bind(master_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("Bind server socket failed!");
        return -3;
    }

    // listen
    if (listen(master_sock, 10) < 0) {
        perror("Listen to server socket failed!");
        return -4;
    }



    fd_set sock_set;
    int new_socket;
    string client_name, command;
    bool transaction_started = false;
    bool transaction_ended = false;
    double transaction_start_time, last_transaction_time; // for data used in summary
    pair<string, int> total_clients[10];
    int client_count = 0;
    int exit_client_index = 0;

    memset(client_message, 0, sizeof(client_message));

    start_time = getCurrentTime();

    while(true) {
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        // clear the socket set
        FD_ZERO(&sock_set);

        // add master socket to socket set
        FD_SET(master_sock, &sock_set);
        max_sd = master_sock;

        // add child sockets to set
        for (int i = 0; i < MAX_CLIENT; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &sock_set);
            }

            // Get highest socket number
            if (client_sockets[i] > max_sd) {
                max_sd = client_sockets[i];
            }
        }

        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &sock_set , NULL , NULL , &tv);  

        if ((activity < 0) && (errno!=EINTR)) {
            perror("Socket selection error!");
            return -5;
        }

        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_sock, &sock_set)) {

            if ((new_socket = accept(master_sock, (struct sockaddr *) &client_addr, (socklen_t*) &client_len))<0)  
            {  
                perror("Accept new connection failed!");  
                return -6;  
            }  
             
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(client_addr.sin_addr) , ntohs(client_addr.sin_port));  
            client_count++;
            //add new socket to array of sockets 
            for (int i = 0; i < MAX_CLIENT; i++)  
            {  
                //if position is empty 
                if( client_sockets[i] == 0 )  
                {  
                    client_sockets[i] = new_socket;  
                    break;  
                }  
            }  
        }

        //else its some IO operation on some other socket
        for (int i = 0; i < MAX_CLIENT; i++)  
        {  
            int sd = client_sockets[i];  
                 
            if (FD_ISSET(sd, &sock_set))  
            {   
                //Check if it was for closing , and also read the 
                //incoming message 
                read_size = recv(sd, client_message , 2000 , 0);
                if ( read_size == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&client_addr , (socklen_t*) &client_len);  
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(client_addr.sin_addr) , ntohs(client_addr.sin_port));  
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close(sd);  
                    client_sockets[i] = 0; 
                }  
                else if (read_size > 0)
                {  

                    start_time = getCurrentTime(); // Update the start time when there's new message inbound
                    
                    // Get the transaction start time if this is the first transaction
                    if (!transaction_started) {
                        transaction_start_time = getCurrentTime();
                        transaction_started = true;
                    }

                    // process client message
                    // string client_message_str(client_message);
                    parseClientMessage(client_message, client_name, command, transaction_ended);

                    if (transaction_ended) {
                        total_clients[exit_client_index].first = client_name;
                        total_clients[exit_client_index].second = atoi(command.c_str());
                        transaction_ended = false;
                        exit_client_index++;
                    }
                    else {

                        // increase the transaction number
                        transaction_number++;

                        // Write log
                        writeServerLog(fp, getCurrentTime(), transaction_number, command, client_name);


                        printf("-------> %s from %s\n", command.c_str(), client_name.c_str());
                        
                        // Do the task
                        string paramStr = command.substr(1);
                        int param = atoi(paramStr.c_str());
                        Trans(param);


                        // Send reply back to client
                        string reply = "D" + to_string(transaction_number);
                        write(sd , reply.c_str() , strlen(reply.c_str()));

                        //Write log
                        writeServerLog(fp, getCurrentTime(), transaction_number, (string) "Done", client_name);

                        // Get last transaction time
                        last_transaction_time = getCurrentTime();
                    }

                    memset(client_message, 0, sizeof(client_message));
                }
                else {
                    perror("recv failed");
                    break;
                }  
            }  
        }  

        // Exit the loop when tere's no new message in 30 seconds
        current_time = getCurrentTime();
        if (current_time - start_time > 30) {
            printf("Server exit!\n");
            break;
        }
    }

    double transaction_interval = last_transaction_time - transaction_start_time;

    writeServerSummary(fp, total_clients, transaction_interval, client_count);

    close(master_sock);
    
    closeLog(&fp);

    return 0;
}


int main(int argc, char* argv[]) {
    int port;

    if (argc == 2) {
        port = atoi(argv[1]);
    }
    else {
        printf("Input invalid, please only include the port number as an argument\n");
        return -1;
    }

    if (port < 5000 or port > 64000) {
        printf("Input invalid, the port number should be between 5000 and 64000\n");
        return -1;
    }

    int ret = start_server(port);

    if (ret < 0) {
        return ret;
    }

    return 0;
}