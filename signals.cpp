// signals.c
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */
#include "signals.h"

using namespace std;

void ctrlc_handle(vector <job*>& fg)
{
	if(fg.size()!=0)
	{
	      kill(fg.back()->getPid(),SIGKILL);
	      fg.erase(fg.begin()+fg.size()-1);
	}

}
void ctrlz_handle(vector<job*>& jobs, vector<job*>& fg )
{
	if(fg.size()==0)
        {
	        return;
        }
        kill(fg.back()->getPid(),SIGSTOP);
        cout << "smash: process "<< fg.back()->getPid()<< " was stopped"<< endl;
        fg.back()->setjob_St(Stopped);
        jobs.push_back(fg.back());
        fg.erase(fg.begin()+fg.size()-1);
}


