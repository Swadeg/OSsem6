//		commands.c
//********************************************
#include "commands.h"
using namespace std;


//*****************************
//implementinf class functions
//*****************************


job::job(int jobId_, int pid_, char* cmd_, double insertingTime_, bool isStopped_)
{
	jobId = jobId_;
	pid = pid_;
	cmd = cmd_;
	int cmd_len = strlen(cmd_)+1;
	cmd = new char[cmd_len];
	strncpy(cmd, (const char*)cmd_, cmd_len-1);
	insertingTime = insertingTime_;
	isStopped = isStopped_;
}
int job::getJobId()
{
	return jobId;
}
int job::getPid()
{
	return pid;
}
string job::getCmd()
{
	return cmd;
}
double job::getInsertingTime()
{
	return insertingTime;
}
bool job::getIsStopped()
{
	return isStopped;
}

//**********************************
//implementing helping functions
//**********************************

void deleteFinishedJobs(vector<job*>& jobs)
{
	int index = 0;
	for (unsigned int i = 0; i < jobs.size(); i++)
	{
	        int status;
		int waitResult = waitpid(jobs[i]->getPid(), nullptr, WNOHANG);
		if (waitResult != 0) // means job[i] has finished
		{
			jobs.erase(jobs.begin() + index);
		}
		index++;
	}
}

bool compareByJobId(job* a, job* b) {
	return (a->getJobId()) < (b->getJobId());
}

int getSize(char * line)
{
	int length=0;
	while (*(line++) !='\0' )
	{
		length ++;
	}
	return length;
}


//********************************************
// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//**************************************************************************************
int ExeCmd(vector<job*>& jobs, char* lineSize, char* cmdString, char* past_directory, char* past_temp)
{
	char* cmd;
	char* args[MAX_ARG];
	char pwd[MAX_LINE_SIZE];
	char* delimiters = " \t\n";
	int i = 0, num_arg = 0;
	bool illegal_cmd = FALSE; // illegal command

	static int cd_count = 0;

	cmd = strtok(lineSize, delimiters);
	if (cmd == NULL)
		return 0;
	args[0] = cmd;
	for (i = 1; i < MAX_ARG; i++)
	{
		args[i] = strtok(NULL, delimiters);
		if (args[i] != NULL)
			num_arg++;
	}
	
	/*************************************************/
	// Built in Commands PLEASE NOTE NOT ALL REQUIRED
	// ARE IN THIS CHAIN OF IF COMMANDS. PLEASE ADD
	// MORE IF STATEMENTS AS REQUIRED
	/*************************************************/
	if (!strcmp(cmd, "cd"))
	{
		char* directory = args[1];
		long size = pathconf(".", _PC_PATH_MAX);
		if (size == 0) return 1; // error
		long size_temp = pathconf(".", _PC_PATH_MAX);
		if (size_temp == 0) return 1; // error
		if (num_arg > 1) // error
		{
			cout << "‫‪smash‬‬ ‫‪error:‬‬ ‫‪cd:‬‬ ‫‪too‬‬ ‫‪many‬‬ ‫‪arguments‬‬" << endl;
			return 1;
		}
		if (cd_count == 0 && !strcmp(directory, "-")) // first time in cd and next path is - , error
		{
			cout << "‫‪smash‬‬ ‫‪error:‬‬ ‫‪cd:‬‬ ‫‪OLDPWD‬‬ ‫‪not‬‬ ‫‪set‬‬" << endl;
			return 1;
		}
		if (!strcmp(directory, "-"))
		{
			getcwd(past_temp, (size_t)size_temp);
			int ret;
			ret = chdir(past_directory);
			if (ret == -1) return 1;
			strcpy(past_directory, (const  char*)past_temp);
			cd_count++;
			return 0;
		}
		else
		{
			int ret;
			getcwd(past_directory, (size_t)size);
			ret = chdir(directory);
			if (ret == -1) return 1;
			cd_count++;
			return 0;
		}
	}

	/*************************************************/
	else if (!strcmp(cmd, "pwd"))
	{
		long size;
		char* buf;
		size = pathconf(".", _PC_PATH_MAX);//pathconf returns the max length of the relative path
		if (size == 0) return 1; // error
		buf = new char[size + 1];
		getcwd(buf, (size_t)size); // buf will contain the path
		cout << buf << endl;
		delete[] buf;
		return 0;
	}

	/*************************************************/
	else if (!strcmp(cmd, "mkdir"))
		{
			if (num_arg<2)
			{
				perror("invalid parameters");
				return 1;
			}

			mode_t permissions;
			//S_IRWXU | S_IRWXG | S_IRWXO;  // Default permissions: rwxrwxrwx


			 string permission_str = args[2];
			 if (permission_str.size()!=9)
			 {
				 perror("Error: invalid permissions");
				 return 1;
			 }


			 if(permission_str[0] == 'r')
			 {
				 permissions |= S_IRUSR ;
			 }
			 if(permission_str[1] == 'w')
			 {
		 		 permissions |= S_IWUSR ;
			 }
			 if(permission_str[2] == 'x')
			 {
			 	 permissions |= S_IXUSR ;
			 }
			 if(permission_str[3] == 'r')
			 {
			 	 permissions |= S_IRGRP ;
			 }
			 if(permission_str[4] == 'w')
			 {
			 	 permissions |= S_IWGRP ;
			 }
			 if(permission_str[5] == 'x')
			 {
			 	 permissions |= S_IXGRP ;
			 }
			 if(permission_str[6] == 'r')
			 {
			 	 permissions |= S_IROTH ;
			 }
			 if(permission_str[7] == 'w')
			 {
			 	 permissions |= S_IWOTH ;
			 }
			 if(permission_str[8] == 'x')
			 {
			 	 permissions |= S_IXOTH ;
			 }

			cout << permission_str <<endl;
			 cout << (mode_t)permissions << endl;
	 		int status=mkdir(args[1], permissions);


	 		if (status!=0)
	 		{
	 			perror("Error creating directory");
	 		}

	 		else
	 		{
	 			cout<< "directory " << args[1] << " created successfuly"<< endl;
	 			return 0;
	 		}



		}
	/*************************************************/
	else if (!strcmp(cmd, "jobs"))
	{
		double timeElapsed;
		deleteFinishedJobs(jobs);
		sort(jobs.begin(), jobs.end(), compareByJobId);
		for (int i = 0; i < jobs.size(); i++)
		{
			timeElapsed = difftime(time(NULL), jobs[i]->getInsertingTime());
			if (jobs[i]->getIsStopped())
			{
				cout << "[" << jobs[i]->getJobId() << "] " << jobs[i]->getCmd() << " : " << jobs[i]->getPid() << " " << timeElapsed << " secs " << "(stopped)" << endl;
			}
			else
			{
				cout << "[" << jobs[i]->getJobId() << "] " << jobs[i]->getCmd() << " : " << jobs[i]->getPid() << " " << timeElapsed << " secs" << endl;
			}
		}

		return 0;
	}


	/*************************************************/
	else if (!strcmp(cmd, "kill"))
	{
		if (num_arg != 2)
		{
			cout << "smash error: kill: invalid arguments" << endl;
			return 1;
		}

		for (int i = 0; i < jobs.size(); i++)
		{
			if (jobs[i]->getJobId() == atoi(args[2]))
			{
				kill(jobs[i]->getPid(), args[1][1]); //args[1][1] is the signal number.
				cout << "signal number " << args[1][1] << " was sent to pid " << jobs[i]->getPid() << endl;
				return 0;
			}
		}
		// jobId is not existed
		cout << "smash error: kill: job-id " << args[2] << "does not exist" << endl;
		return 1;
	}
	/*************************************************/
	else if (!strcmp(cmd, "showpid"))
	{
		cout << "smash pid is " << getpid() << endl;
		return 0;
	}
	/*************************************************/
	else if (!strcmp(cmd, "fg"))
	{
		if (num_arg > 1)
		{
			cout << "smash error: fg: invalid arguments" << endl;
			return 1;
		}

		int jobIdToFg;
		if (num_arg == 0) // means no job id
		{
			if (jobs.size() == 0)
			{
				cout << "‫‪smash‬‬ ‫‪error:‬‬ ‫‪fg:‬‬ ‫‪jobs‬‬ ‫‪list‬‬ ‫‪is‬‬ ‫‪empty‬‬" << endl;
				return 1;
			}
			int maxJobId = jobs[0]->getJobId();
			for (int i = 1; i < jobs.size(); i++)
			{
				if (jobs[i]->getJobId() > maxJobId) maxJobId = jobs[i]->getJobId();
			}
			jobIdToFg = maxJobId;
		}
		else if (num_arg == 1)
		{
			jobIdToFg = atoi(args[1]);
		}

		for (int i = 0; i < jobs.size(); i++)
		{
			if (jobs[i]->getJobId() == jobIdToFg)
			{
				int status = 0;
				cout << jobs[i]->getCmd() << " : " << jobs[i]->getPid() << endl;
				kill(jobs[i]->getPid(), SIGCONT);
				int waitRes = waitpid(jobs[i]->getPid(), &status, WUNTRACED);
				if (waitRes == -1 ) cout << "smash error : waitpid failed" << endl;
				jobs.erase( jobs.begin()+i );
				return 0;
			}
		}
		cout << "smash error: fg: job-id " << args[1] << " does not exist" << endl;
		return 1;


	}
	/*************************************************/
	else if (!strcmp(cmd, "bg"))
	{
		if (num_arg > 1)
		{
			cout << "smash error: fg: invalid arguments" << endl;
			return 1;
		}


		int jobIdToBg;
		int maxJobId = -1;


		if (num_arg == 0)
		{
			if (jobs.size() == 0)
			{
				cout << "‫‪smash‬‬ ‫‪error:‬‬ ‫‪fg:‬‬ ‫‪jobs‬‬ ‫‪list‬‬ ‫‪is‬‬ ‫‪empty‬‬" << endl;
				return 1;
			}

			//find first stopped job
			int firstStoppedIdx;
			for (int i = 0; i < jobs.size(); i++)
			{
				if (jobs[i]->getIsStopped())
				{
					maxJobId = jobs[i]->getJobId();
					firstStoppedIdx = i;
					break;
				}
			}

			if (maxJobId == -1) // no stopped job
			{
				cout << "‫‪smash‬‬ ‫‪error:‬‬ ‫‪bg:‬‬ ‫‪there‬‬ ‫‪are‬‬ ‫‪no‬‬ ‫‪stopped‬‬ ‫‪jobs‬‬ ‫‪to‬‬ ‫‪resume‬‬" << endl;
				return 1;
			}

			for (int i = firstStoppedIdx; i < jobs.size(); i++)
			{
				if (!(jobs[i]->getIsStopped())) continue;
				else if (jobs[i]->getJobId() > maxJobId)
				{
					maxJobId = jobs[i]->getJobId();
				}

			}

			jobIdToBg = maxJobId;
		}


		if (num_arg == 1)
		{
			int jobIdReq = atoi(args[1]);
			bool jobIdReqIsExist = false;
			for (int i = 0; i < jobs.size(); i++)
			{
				if (jobs[i]->getJobId() == jobIdReq)
				{
					if (!(jobs[i]->getIsStopped()))
					{
						cout << "‫‪smash‬‬ ‫‪error:‬‬ ‫‪bg:‬‬ ‫‪job-id‬‬ ‪" << jobIdReq << " ‫‪is‬‬ ‫‪already‬‬ ‫‪running‬‬ ‫‪in‬‬ ‫‪the‬‬ ‫‪background‬‬" << endl;
						return 1;
					}
					jobIdToBg = jobIdReq;
					jobIdReqIsExist = true;
				}
			}
			if (jobIdReqIsExist == false)
			{
				cout << "smash error: bg: job-id " << jobIdReq << " does not exist" << endl;
				return 1;
			}
		}


		for (int i = 0; i < jobs.size(); i++)
		{
			if (jobs[i]->getJobId() == jobIdToBg)
			{
				int status = 0;
				cout << jobs[i]->getCmd() << " : " << jobs[i]->getPid() << endl;
				kill(jobs[i]->getPid(), SIGCONT);
				return 0;
			}
		}

	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{

		if (num_arg == 1 && !strcmp(args[1], "kill"))
		{
			for (int i = 0; i < jobs.size(); i++)
			{
				kill(jobs[i]->getPid(), SIGTERM);
				cout << "[" << jobs[i]->getJobId() << "] " << jobs[i]->getCmd() << "‫‪Sending‬‬ ‫‪SIGTER‬‬M...";
				sleep(5);
				int waitResult = waitpid(jobs[i]->getPid(), NULL, WNOHANG);
				if (waitResult != 0) // means jobs[i] has finished
				{
					cout << "Done." << endl;
				}
				else
				{
					kill(jobs[i]->getPid(), SIGKILL);
					cout << "(‫‪5‬‬ ‫‪sec‬‬ ‫‪passe‬‬d)‫‪Sending‬‬ ‫...‪SIGKILL‬‬ ‫‪Done‬‬." << endl;
				}


			}
			return 0;
		}


		else
		{
			exit(0);
		}

	}

	/*************************************************/
	else if (!strcmp(cmd, "diff"))
	{
		if (num_arg != 2)
		{
			cout << "smash error: diff: invalid arguments";
			return 1;
		}



		//using File database to deal with files.
		FILE* file1 = fopen(args[1], "r");
		FILE* file2 = fopen(args[2], "r");

		char a, b;


		while (fscanf(file1, "%c", &a) != EOF && fscanf(file2, "%c", &b) != EOF)
		{
			if (a != b)
			{
				fclose(file1);
				fclose(file2);
				cout << "1" << endl;
				return 1;//two files aren't equal.
			}
		}

		if (fscanf(file1, "%c", &a) != EOF || fscanf(file2, "%c", &b) != EOF)
		{
			fclose(file1);
			fclose(file2);
			cout << "1" << endl;
			return 1;//two files are not equal.
		}

		fclose(file1);
		fclose(file2);
		cout << "0" << endl;
		return 0;//two files are equal.
	}


	/*************************************************/
	else // external command
	{
		ExeExternal(args, cmdString);
		return 0;
	}
	if (illegal_cmd == TRUE)
	{
		printf("smash error: > \"%s\"\n", cmdString);
		return 1;
	}
	return 0;
}
//**************************************************************************************
// function name: ExeExternal
// Description: executes external command
// Parameters: external command arguments, external command string
// Returns: void
//**************************************************************************************
void ExeExternal(char* args[MAX_ARG], char* cmdString)
{
	int pID;
	switch (pID = fork())
	{
	case -1:
		// Add your code here (error)
		perror("smash error: fork failed");
		return;
	case 0:
		// Child Process
		if (setpgrp() == -1)
		{
			perror("smash error: setpgrp failed");
			return;
		}

		// Add your code here (execute an external command)
		if (execvp(args[0], args) == -1)
		{
			perror("smash error: execv failed");
			return;
		}


	default:
		// Add your code here
		// if pid > 0 its parent procces
		int status;
		if (waitpid(pID, &status, WUNTRACED) == -1)
		{
			perror("smash error: waitpid failed");
			return;
		}
	}
}
//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command -1- if not
//**************************************************************************************
int BgCmd(char* lineSize, vector<job*>& jobs)
{
	char* Command;
	char* delimiters = " \t\n";
	char* args[MAX_ARG];
	int num_arg = 0;
	int maxJobId = 0;
	int bgJobId;
	int bgpId;

	int line_size= getSize(lineSize);
	char* fullCommand = new char[line_size+1];
	strcpy(fullCommand, (const char*)lineSize);
	fullCommand[line_size-1]='\0';

	if (lineSize[strlen(lineSize) - 2] == '&')
	{
		lineSize[strlen(lineSize) - 2] = '\0';
		Command = strtok(lineSize, delimiters);
		args[0] = Command;
		for (int i = 1; i < MAX_ARG; i++)
		{
			args[i] = strtok(NULL, delimiters);
			if (args[i] != NULL) { num_arg++; }
		}
		if (jobs.size() == 0)
		{
			bgJobId = 1;
		}
		else
		{
			for (int i = 0; i < jobs.size(); i++)
			{
				if (jobs[i]->getJobId() > maxJobId)
				{
					maxJobId = (jobs[i]->getJobId());
				}
			}
			bgJobId = maxJobId + 1;
		}
		bgpId = fork();
		if (bgpId == -1)
		{
			perror("smash error: fork failed");
			return -1;
		}
		if (bgpId == 0) /*child process*/
		{
			if (setpgrp() == -1)
			{
				perror("smash error: setpgrp failed");
				return -1;
			}
			if (execvp(args[0], args) == -1)
			{
				perror("smash error: execv failed");
				return -1;
			}
		}
		else
		{
			job* new_job = new job(bgJobId, bgpId, fullCommand, time(NULL), false);
			jobs.push_back(new_job);	
		}
		return 0;
	}
	return -1;
}
//**************************************************************************************
// function name: ExeComp
// Description: executes complicated command
// Parameters: command string
// Returns: 0- if complicated -1- if not
//**************************************************************************************
int ExeComp(char* lineSize)
{
	char ExtCmd[MAX_LINE_SIZE + 2];
	char* args[MAX_ARG];
	if ((strstr(lineSize, "|")) || (strstr(lineSize, "<")) || (strstr(lineSize, ">")) || (strstr(lineSize, "*")) || (strstr(lineSize, "?")) || (strstr(lineSize, ">>")) || (strstr(lineSize, "|&")))
	{
		// Add your code here (execute a complicated command)

		/*
		your code
		*/
	}
	return -1;
}
