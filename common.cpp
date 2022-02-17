#include "common.h"

using namespace std;

double getCurrentTime() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    double seconds = (double) currentTime.tv_sec + (double) currentTime.tv_usec / 1000000;
    return seconds;
}

void writeClientLog(FILE* fp, double current_time, string direction, string cmd) {
    char processed_cmd[100] = {0};

    if (cmd[0] == 'T' || cmd[0] == 'D') {
        // Parse the command and create the string to put in log
        string n_str = cmd.substr(1);
        string instruction = cmd.substr(0, 1);
        int n = atoi(n_str.c_str());
        sprintf(processed_cmd, "%s%3d", instruction.c_str(), n);

        fprintf(fp, "%.2f: %s (%s)\n", current_time, direction.c_str(), processed_cmd);
    }
    else {
        // Sleep scenraio
        string n_str = cmd.substr(1);
        int n = atoi(n_str.c_str());

        fprintf(fp, "Sleep %d units\n", n);
    }

}

void writeServerLog(FILE* fp, double current_time, int transaction_number, string cmd, string client_name) {
    char processed_cmd[50];
    memset(processed_cmd, 0, sizeof(processed_cmd));

    // Parse the command and create the string to put in log
    if (cmd[0] == 'T') {
        string n_str = cmd.substr(1);
        string instruction = cmd.substr(0, 1);
        int n = atoi(n_str.c_str());

        sprintf(processed_cmd, "%s%3d", instruction.c_str(), n);
    }
    else {
        sprintf(processed_cmd, "%s", cmd.c_str());
    }

    fprintf(fp, "%.2f: # %2d (%4s) from %s\n", current_time, transaction_number, processed_cmd, client_name.c_str());
    memset(processed_cmd, 0, sizeof(processed_cmd));
}

void writeClientSummary(FILE* fp, int total_transaction) {
    fprintf(fp, "Sent %d transactions\n", total_transaction);
}

void writeServerSummary(FILE* fp, pair<string, int> total_clients[], double time_interval, int client_no) {
    fprintf(fp, "\nSUMMARY\n");
    int total_transactions = 0;

    for (int i = 0; i < client_no; i++) {
        fprintf(fp, "%4d, transactions from %s\n", total_clients[i].second, total_clients[i].first.c_str());
        total_transactions += total_clients[i].second;
    }

    double trans_per_sec = total_transactions / time_interval;
    fprintf(fp, "%4.1f transactions/sec  (%d/%.2f)\n", trans_per_sec, total_transactions, time_interval);
}
 
FILE* createLog(char* fileName) {
    FILE* fp = fopen(fileName, "w+");
    
    return fp;
}

int closeLog(FILE** fp) {
    fclose(*fp);
    return 0;
}