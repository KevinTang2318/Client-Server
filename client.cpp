#include "client.h"
#include "tands.h"
#include "common.h"

using namespace std;

// code for client functionality
// Reference: https://www.binarytides.com/server-client-example-c-sockets-linux/
// Here I also reused some codes from my Assignment 2 solution: prodcon
int start_client(int port, char* server_ip) {
    int sock;
    int total_transactions = 0;  //TODO: use this to write log
	struct sockaddr_in server;
	char server_reply[2000];
    char host_name[50];
    pid_t pid;

    FILE* fp;

    // get host name and pid of this client, then process client name
    gethostname(host_name, sizeof(host_name));
    pid = getpid();
    char client_name[100];
    sprintf(client_name, "%s.%d", host_name, pid);

    // Create log file
    fp = createLog(client_name);
    if (fp == NULL) {
        perror("Create log for client failed!");
        return -1;
    }


    // create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock < 0)
	{
		perror("Create client socket failed!");
        return -1;
	}
	
    // Set up server addr
    server.sin_addr.s_addr = inet_addr(server_ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

    printf("Using port %d\n", port);
    fprintf(fp, "Using port %d\n", port);
    printf("Using server address %s\n", server_ip);
    fprintf(fp, "Using server address %s\n", server_ip);
    printf("Host %s\n", client_name);
    fprintf(fp, "Host %s\n", client_name);

    // Connect to server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("Connect to server failed!");
		return -2;
	}

    string cmd;
    char message[50];
    while (!cin.eof()) {
        cin >> cmd;

        // First process when there is empty emput
        if (cmd == "") {
            printf("Producer terminating!\n");
            break;
        }

        // Parse command and put command in queue
        if (cmd[0] == 'T') {
            total_transactions++;
            
            //write send log
            writeClientLog(fp, getCurrentTime(), "Send", cmd);

            // Create client message, lead with client name
            sprintf(message, "%s %s", client_name, cmd.c_str());

            //Send some data
            if( send(sock , message , strlen(message) , 0) < 0)
            {
                perror("Client message send failed!");
                return -3;
            }

            memset(server_reply, 0, sizeof(server_reply));
            //Receive a reply from the server
            if( recv(sock , server_reply , 2000 , 0) < 0)
            {
                perror("Receiving server reply failed");
                return -4;
            }

            string sreply(server_reply);

            // write receive log
            writeClientLog(fp, getCurrentTime(), "Recv", sreply);

            printf("Server reply: %s\n", server_reply);
        }
        else if (cmd[0] == 'S') {
            string sleepTimeStr = cmd.substr(1);
            int sleepTime = atoi(sleepTimeStr.c_str());
            // Write log
            writeClientLog(fp, getCurrentTime(), "N/A", cmd);
            Sleep(sleepTime);
        }

        memset(server_reply, 0, sizeof(server_reply));
        memset(message, 0, sizeof(message));
    }

    //Client send end message to server when finished
    string end_message = client_name + (string) " " + to_string(total_transactions);
    if( send(sock , end_message.c_str() , strlen(end_message.c_str()) , 0) < 0)
    {
        perror("Client message send failed!");
        return -3;
    }



    close(sock);

    //Write summary
    writeClientSummary(fp, total_transactions);

    // Close log
    closeLog(&fp);
    return 0;
}

int main(int argc, char* argv[]) {
    int port;
    char* server_ip;

    if (argc == 3) {
        port = atoi(argv[1]);
        server_ip = argv[2];
    }
    else {
        printf("Input invalid, please only include the port number as an argument\n");
        return -1;
    }

    if (port < 5000 or port > 64000) {
        printf("Input invalid, the port number should be between 5000 and 64000\n");
        return -1;
    }

    int ret = start_client(port, server_ip);

    if (ret < 0) {
        return ret;
    }

    return 0;
}