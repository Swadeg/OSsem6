#ifndef _COMMANDS_H
#define _COMMANDS_H
#include <unistd.h> 
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
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
	char* cmd;
	double insertingTime;
	bool isStopped;
	
public:
	job(){};
	job(int jobId_, int pid_, char* cmd_, double insertingTime_, bool isStopped_);
	~job(){delete[] cmd;};
	int getJobId();
	int getPid();
	string getCmd();
	double getInsertingTime();
	bool getIsStopped();
	
};



int ExeComp(char* lineSize);
int BgCmd(char* lineSize, vector<job*>& jobs);
int ExeCmd(vector<job*>& jobs, char* lineSize, char* cmdString, char *past_directory, char *past_temp);
void ExeExternal(char *args[MAX_ARG], char* cmdString);
#endif
