#ifndef _COMMANDS_H
#define _COMMANDS_H
#include <unistd.h> 
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <bits/stdc++.h>
#include <vector>
#include <fstream>
#include <iostream>
#define MAX_LINE_SIZE 80
#define MAX_ARG 20
typedef enum { FALSE , TRUE } Bool;
//typedef enum {"Foreground", "Background" , "Stopped"} job_States;
using namespace std;

class job{
	int jobId;
	int pid;
	string cmd;
	double insertingTime;
	bool isStopped;
	
public:
	job(){};
	job(int jobId, int pid, string cmd, double insertingTime, bool isStopped);
	~job(){};
	int getJobId(){return jobId;}
	int getPid(){return pid;}
	string getCmd(){return cmd;}
	double getInsertingTime(){return insertingTime;}
	bool getIsStopped(){return isStopped;}
	
};



int ExeComp(char* lineSize);
int BgCmd(char* lineSize, vector<job*> jobs);
int ExeCmd(vector<job*> jobs, char* lineSize, char* cmdString, char *past_directory, char *past_temp);
void ExeExternal(char *args[MAX_ARG], char* cmdString);
#endif

