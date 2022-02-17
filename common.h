#ifndef __COMMON__
#define __COMMON__

#include <sys/time.h>
#include <stdio.h>
#include <string>
#include <cstring>


double getCurrentTime();
void writeClientLog(FILE* fp, double current_time, std::string direction, std::string cmd);
void writeServerLog(FILE* fp, double current_time, int transaction_number, std::string cmd, std::string client_name);
void writeClientSummary(FILE* fp, int total_transaction);
void writeServerSummary(FILE* fp, std::pair<std::string, int> total_clients[], double time_interval, int client_no);
FILE* createLog(char* fileName);
int closeLog(FILE** fp);



#endif