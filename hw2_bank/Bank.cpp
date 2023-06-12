#include <pthread.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <semaphore.h>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <cmath>
#include <random>
#include <algorithm>
#include <fstream>

using namespace std;

// implement the class as monitor , add lock ("wrt_lock")
class Account {
    unsigned  acc_id;
    unsigned  pass;
    unsigned  amount;
    int rd_count ;
    pthread_mutex_t wr_lock;
    pthread_mutex_t rd_lock;

    public:
        Account() = default;
        Account(unsigned acc_id_, unsigned pass_, unsigned amount_);
        ~Account() = default;
        void increase_amount(unsigned amount_);
        int decrease_amount (unsigned amount_);
        unsigned get_amount();
        unsigned get_acc_id() { return acc_id; }
        unsigned get_pass() { return pass; }
};


vector <Account*> accounts_vec;
vector <pthread_t> atm_vec;
vector <char*> files_paths;
vector <bool> finished_atm_vec;
unsigned bank_balance = 0;

// Create an output file stream
ofstream outputFile("log.txt");

pthread_mutex_t wr_to_log;

sem_t open_lock;
sem_t close_lock;

void init_lockers()
{
    sem_init( &open_lock, 0, 1 );
    sem_init( &close_lock, 0, 1 );
}


Account::Account(unsigned acc_id_, unsigned pass_, unsigned amount_)
{
    acc_id = acc_id_;
    pass = pass_;
    amount = amount_;
    rd_count = 0;
    if (pthread_mutex_init(&wr_lock ,NULL) != 0) perror("error initiating wr_locks");
    if (pthread_mutex_init(&rd_lock ,NULL) != 0) perror("error initiating rd_locks");
}

void Account::increase_amount(unsigned amount_)
{
    pthread_mutex_lock (&wr_lock); 
    amount += amount_;
    sleep(1);
    pthread_mutex_unlock (&wr_lock);
}

int Account::decrease_amount(unsigned amount_)
{
    pthread_mutex_lock (&wr_lock); 
    if(amount < amount_)// cant withdrew
    {
        pthread_mutex_unlock (&wr_lock);
        return -1;
    }
    amount -= amount_;
    sleep(1);
    pthread_mutex_unlock (&wr_lock);
    return 0;
}

unsigned Account::get_amount()
{
    unsigned amount_to_return;
    pthread_mutex_lock (&rd_lock); 
    rd_count++;
    if(rd_count == 1)
        pthread_mutex_lock (&wr_lock);
    pthread_mutex_unlock (&rd_lock);
    amount_to_return = amount;
    sleep(1);
    pthread_mutex_lock (&rd_lock); 
    rd_count--;
    if(rd_count == 0)
        pthread_mutex_unlock (&wr_lock);
    pthread_mutex_unlock (&rd_lock);
    return amount_to_return;
}

void open_account(int itm_id, unsigned account, unsigned pass, unsigned init_amount)
{  
    sem_wait(&open_lock);
    for (unsigned int i=0; i<accounts_vec.size(); i++)
    {
        if (accounts_vec[i]->get_acc_id() == account)
        {
            pthread_mutex_lock (&wr_to_log);
            outputFile << "Error " << itm_id << ": Your transaction failed - account with the same id exists" << endl;
            pthread_mutex_unlock (&wr_to_log);
            sleep(1);
            sem_post(&open_lock);
            return;
        }
    }
    //account is not exist
    Account* new_acc = new Account(account, pass, init_amount);
    accounts_vec.push_back(new_acc);    
    pthread_mutex_lock (&wr_to_log);
    outputFile << itm_id << ": New account id is " << account << " with password " << pass << " and initial balance " << init_amount << endl; 
    pthread_mutex_unlock (&wr_to_log);
    sleep(1);
    sem_post(&open_lock);    
}

void deposit (int itm_id, unsigned account, unsigned pass, unsigned amount)
{
    for (unsigned int i=0; i<accounts_vec.size(); i++)
    {
        if (accounts_vec[i]->get_acc_id() == account)
        {
            if (accounts_vec[i]->get_pass() != pass)
            {
                pthread_mutex_lock (&wr_to_log);
                outputFile << "Error " << itm_id << ": Your transaction failed - password for account id " << account << " is incorrect" << endl;
                pthread_mutex_unlock (&wr_to_log);
                sleep(1);
                return;
            }
            // pass is correct
            accounts_vec[i]->increase_amount(amount); // amount lock is in class function
            pthread_mutex_lock (&wr_to_log);
            outputFile << itm_id << ": Account " << account << " new balance is " <<
                accounts_vec[i]->get_amount() << " after " << amount << " $ was deposited" << endl;
            pthread_mutex_unlock (&wr_to_log);
            return;
        }
    }
    // account does not exist
    pthread_mutex_lock (&wr_to_log);
    outputFile << "Error " << itm_id << ": Your transaction failed - account id " << account << " does not exist" << endl;
    pthread_mutex_unlock (&wr_to_log);
    sleep(1);
    return;
}

void withrawal(int itm_id, unsigned account, unsigned pass, unsigned amount)
{
    int dec_suc = -1;
    for (unsigned int i=0; i<accounts_vec.size(); i++)
    {
        if (accounts_vec[i]->get_acc_id() == account)
        {
            if (accounts_vec[i]->get_pass() != pass)
            {
                pthread_mutex_lock (&wr_to_log); 
                outputFile << "Error " << itm_id << ": Your transaction failed - password for account id " << account << " is incorrect" << endl;
                pthread_mutex_unlock (&wr_to_log); 
                sleep(1);
                return;
            }
            dec_suc = accounts_vec[i]->decrease_amount(amount); // amount lock is in class function
            if (dec_suc == -1) // amount to wuthrew > account amount 
            {
                pthread_mutex_lock (&wr_to_log); 
                outputFile << "Error " << itm_id << ": Your transaction failed - account id " << account << " balance is lower than " << amount << endl;
                pthread_mutex_unlock (&wr_to_log); 
                sleep(1);
                return;
            }
            // pass is correct and amount < balance
            pthread_mutex_lock (&wr_to_log); 
            outputFile << itm_id << ": Account " << account << " new balance is " << 
                accounts_vec[i]->get_amount() << " after " << amount << " $ was withdrew" << endl;
            pthread_mutex_unlock (&wr_to_log); 
            return;
        }
    }
    // account does not exist
    pthread_mutex_lock (&wr_to_log); 
    outputFile << "Error " << itm_id << ": Your transaction failed - account id " << account << " does not exist" << endl;
    pthread_mutex_unlock (&wr_to_log); 
    sleep(1);
    return;    
}

void balance_check(int itm_id, unsigned account, unsigned pass)
{
    for (unsigned int i=0; i<accounts_vec.size(); i++)
    {
        if (accounts_vec[i]->get_acc_id() == account)
        {
            if (accounts_vec[i]->get_pass() != pass)
            {
                pthread_mutex_lock (&wr_to_log); 
                outputFile << "Error " << itm_id << ": Your transaction failed - password for account id " << account << " is incorrect" << endl;
                pthread_mutex_lock (&wr_to_log); 
                sleep(1);
                return;
            }
            // pass is correct
            /*
            to read balance 
            + give access for more than one reader
            are implemented in the class 
            */
            accounts_vec[i]->get_amount();
            pthread_mutex_lock (&wr_to_log); 
            outputFile << itm_id << ": Account " << account << " balance is " << accounts_vec[i]->get_amount() << endl;
            pthread_mutex_unlock (&wr_to_log); 
            return;
        }
    }
    // account does not exist
    pthread_mutex_lock (&wr_to_log); 
    outputFile << "Error " << itm_id << ": Your transaction failed - account id " << account << " does not exist" << endl;
    pthread_mutex_unlock (&wr_to_log); 
    sleep(1);
    return;
}

void quit_account (int itm_id, unsigned account , unsigned pass)
{
    sem_wait(&close_lock);
    for (unsigned int i=0; i<accounts_vec.size(); i++)
    {
        if (accounts_vec[i]->get_acc_id() == account)
        {
            if (accounts_vec[i]->get_pass() != pass)
            {
                pthread_mutex_lock (&wr_to_log); 
                outputFile << "Error " << itm_id << ": Your transaction failed - password for account id " << account << " is incorrect" << endl;
                pthread_mutex_unlock (&wr_to_log); 
                sleep(1);
                sem_post(&close_lock);
                return;
            }
            // pass is correct
            // delete account
            accounts_vec.erase(accounts_vec.begin() + i);
            pthread_mutex_lock (&wr_to_log); 
            outputFile << itm_id << ": Account " << account << " is now clodsed. Balance was " << accounts_vec[i]->get_amount() << endl;
            pthread_mutex_unlock (&wr_to_log); 
            sleep(1);
            sem_post(&close_lock);
            return;
        }
    }
    // account does not exist
    pthread_mutex_lock (&wr_to_log); 
    outputFile << "Error " << itm_id << ": Your transaction failed - account id " << account << " does not exist" << endl;
    pthread_mutex_unlock (&wr_to_log); 
    sleep(1);
    sem_post(&close_lock);
    return;
}

int get_account_idx(unsigned account)
{
    for (unsigned int i=0; i<accounts_vec.size(); i++)
    {
        if (accounts_vec[i]->get_acc_id() == account)
        {
            return i;
        } 
    }
    return -1; // account is not exist
}

void transfer(int itm_id, unsigned account, unsigned pass, unsigned target_acc, unsigned amount)
{
    int dec_suc = -1;
    int acc_idx = get_account_idx(account);
    int target_idx = get_account_idx(target_acc);
    if (acc_idx == -1)
    {
        pthread_mutex_lock (&wr_to_log); 
        outputFile << "Error " << itm_id << ": Your transaction failed - account id " << account << " does not exist" << endl;
        pthread_mutex_unlock (&wr_to_log); 
        sleep(1);
        return;
    }
    else if (target_idx == -1) // account is exist , target is not 
    {
        pthread_mutex_lock (&wr_to_log); 
        outputFile << "Error " << itm_id << ": Your transaction failed - account id " << target_acc << " does not exist" << endl;
        pthread_mutex_unlock (&wr_to_log); 
        sleep(1);
        return;
    }
    else if (accounts_vec[acc_idx]->get_pass() != pass)
    {
        pthread_mutex_lock (&wr_to_log); 
        outputFile << "Error " << itm_id << ": Your transaction failed - password for account id " << account << " is incorrect" << endl;
        pthread_mutex_unlock (&wr_to_log); 
        sleep(1);
        return;   
    }
    dec_suc = accounts_vec[acc_idx]->decrease_amount(amount);
    if (dec_suc == -1) // amount lock is in class fumction
    {
        pthread_mutex_lock (&wr_to_log); 
        outputFile << "Error " << itm_id << ": Your transaction failed - account id " << account << " balance is lower than " << amount << endl;
        pthread_mutex_unlock (&wr_to_log); 
        sleep(1);
        return;
    }
    else 
    {
        // make amount transfer
        accounts_vec[target_idx]->increase_amount(amount);
        pthread_mutex_lock (&wr_to_log); 
        outputFile << itm_id << ": Transfer " << amount << " from account " << account <<
            " to account " << target_acc << " new account balance is " << 
            accounts_vec[acc_idx]->get_amount() << " new target account balance is " <<
            accounts_vec[target_idx]->get_amount() << endl;
        pthread_mutex_unlock (&wr_to_log); 
    }
}

void parse_line(string line, int itm_id)
{
    istringstream iss(line); // Create an input string stream
    vector<string> tokens; // Vector to store the tokens
    string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    string op = tokens[0];
    if (op == "O") open_account(itm_id, stoul(tokens[1]), stoul(tokens[2]), stoul(tokens[3]));
    else if (op == "D") deposit(itm_id, stoul(tokens[1]), stoul(tokens[2]), stoul(tokens[3]));
    else if (op == "W")  withrawal(itm_id, stoul(tokens[1]), stoul(tokens[2]), stoul(tokens[3]));
    else if (op == "B") balance_check(itm_id, stoul(tokens[1]), stoul(tokens[2]));
    else if (op == "Q") quit_account (itm_id, stoul(tokens[1]), stoul(tokens[2]));
    else if (op == "T") transfer (itm_id, stoul(tokens[1]), stoul(tokens[2]), stoul(tokens[3]), stoul(tokens[4]));
    else perror("invalid action"); 
}

        
void* parse_file(void* file_path)
{
    int itm_id;
    for(unsigned int i=0; i<files_paths.size(); i++)
    {
        if(files_paths[i] == (char*)file_path) 
        {
            itm_id = i+1;
            break;
        }
    }
    string file = (char*)file_path;

    ifstream inputFile(file); // Open file for reading

    if (inputFile.is_open()) {
        // File opened successfully
        // Read data from the file
        string line;
        while (getline(inputFile, line)) {
            parse_line(line, itm_id);
            //pthread_mutex_lock (&wr_to_log);
            //outputFile << line << endl;
            //pthread_mutex_unlock (&wr_to_log);
            //atm sleep for 100 mili seconds
            usleep(100000);
        }
        // Close the file
        inputFile.close();
    } else {
        // Failed to open the file
        cerr << "Bank error: illegal arguments" << endl;
        exit(1);
    }
    return file_path;
}

bool look_for_unfinished_atm()
{
    //return true if there is unfinished atms
    for (unsigned int i=0; i<finished_atm_vec.size(); i++)
    {
        if (finished_atm_vec[i] == false) return true;
    }
    return false;
}

void* backup_fees(void * ptr)
{
    int fee_percent;
    int calced_fee;
    while(look_for_unfinished_atm())
    {
        // Initialize the random number generator
        std::random_device rd;
        std::mt19937 gen(rd());

        // Define the range for random numbers
        int min = 1;
        int max = 5;

        // Define the distribution
        std::uniform_int_distribution<int> dist(min, max);

        // Generate a random number
        fee_percent = dist(gen);

        for (unsigned int i=0; i<accounts_vec.size(); i++)
        {
            calced_fee = round((accounts_vec[i]->get_amount()) * (double)fee_percent/100);
            pthread_mutex_lock (&wr_to_log);
            outputFile << "Bank: commissions of " << fee_percent << " % were charged, the bank gained " <<
                calced_fee << " $ from account " << accounts_vec[i]->get_acc_id() << endl;
            bank_balance += calced_fee;
            pthread_mutex_unlock (&wr_to_log);
            accounts_vec[i]->decrease_amount(calced_fee);
        }
        sleep(3); // sleep for 3 secomds
    }
    return ptr;
}

bool comparedAccountById(Account* account_a, Account* account_b)
{
    return (account_a->get_acc_id() < account_b->get_acc_id());
}

void* print_status(void * ptr)
{
    printf("\033[2J");
    while(look_for_unfinished_atm())
    {
        printf("\033[1;1H");
        cout << "current Bank Status" << endl;
        sort(accounts_vec.begin(), accounts_vec.end(), comparedAccountById);
        for(unsigned int i=0; i<accounts_vec.size(); i++)
        {
            cout << "Account " << accounts_vec[i]->get_acc_id() << ": Balance - " <<
                accounts_vec[i]->get_amount() << " $, Account Password - " << 
                accounts_vec[i]->get_pass() << endl; 
        }
        cout << "The Bank has " << bank_balance << " $" << endl;
        sleep(0.5); 
    }
    return ptr;
}

int main (int argc, char *argv[])
{
    if (argc <= 1) 
    {
        cerr << "Bank error: illegal arguments" << endl;
        return 1;
    }

    if (pthread_mutex_init(&wr_to_log ,NULL) != 0) perror("error initiating rd_locks");
    // Check if the file was opened successfully
    if (!outputFile) {
        cerr << "Bank error: ofstream failed" << endl;
        return 1;
    }

    
    init_lockers();
    for (int i=1; i<argc; i++)
    {
        pthread_t new_thread;
        atm_vec.push_back(new_thread);
        files_paths.push_back(argv[i]);
        finished_atm_vec.push_back(false);
        if (pthread_create(&atm_vec[i-1], NULL, parse_file, (void*) argv[i]))
        {
            // cant create thread
            cerr << "Bank error: pthread_create failed" << endl;
            return 1;
        }
        
    }
    
    //create thread for the bank for backup fees
    pthread_t Bank_thread;
    if (pthread_create(&Bank_thread, NULL, backup_fees, NULL))
    {
        // cant create thread
        cerr << "Bank error: pthread_create failed" << endl;
        return 1;
    }

    //create thread for the bank to print accounts status
    pthread_t Bank_print_status;
    if (pthread_create(&Bank_print_status, NULL, print_status, NULL))
    {
        // cant create thread
        cerr << "Bank error: pthread_create failed" << endl;
        return 1;
    }

    for (int i = 0; i < argc-1; i++) {
        //mark atm as finished
        
        if (pthread_join(atm_vec[i], NULL))
        {
            // cant join thread
            cerr << "Bank error: pthread_join failed" << endl;
            return 1;
        }
        finished_atm_vec[i] = true;
    }
    if (pthread_join(Bank_thread, NULL))
    {
        // cant join thread
        cerr << "Bank error: pthread_join failed" << endl;
        return 1;
    }
    if (pthread_join(Bank_print_status, NULL))
    {
        // cant join thread
        cerr << "Bank error: pthread_join failed" << endl;
        return 1;
    }
}
