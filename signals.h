#ifndef _SIGS_H
#define _SIGS_H
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include "commands.h"


void print_ctrlc();
void print_ctrlz();

void ctrlc_handle(vector <job*>& fg);
void ctrlz_handle(vector<job*>& jobs, vector<job*>& fg );

#endif

