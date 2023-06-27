#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <vector>
#include <algorithm>
#include <sys/select.h>
#include <fstream>
#include <cstdio> // For remove function
#include <stdio.h>

#define BUFFMAX 516  

using namespace std;

int main(int argc, char *argv[])
{
    int sockfd;
    unsigned short port_num, timeout, max_num_of_resp;
    char buffer[BUFFMAX] = {0};
    struct sockaddr_in my_addr = {0}, client_addr = {0};
    socklen_t cliAddrLen;
    int recvMsgSize; 

    if (argc != 4) {
        fprintf(stderr,"TTFTP_ERROR: illegal arguments\n");
        exit(1);
    }
    port_num = atoi(argv[1]);
    timeout = atoi(argv[2]);
    max_num_of_resp = atoi(argv[3]);
    if (port_num<=0 || timeout<=0 || max_num_of_resp<=0)
    {
        fprintf(stderr,"TTFTP_ERROR: illegal arguments\n");
        exit(1);
    }
    if(port_num<=10000) 
    {
        fprintf(stderr,"TTFTP_ERROR: cant run the server with port %u\n", port_num);
        exit(1);       
    }
    sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); // UDP socket
    if (sockfd < 0) 
    {
        perror("TTFTP_ERROR:open socket failed");
        exit(1);
    }
    memset(&my_addr, 0, sizeof(my_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port_num);
    if (bind(sockfd, (struct sockaddr *)&my_addr,sizeof(my_addr)) < 0)
    {
        perror("TTFTP_ERROR:binding failed");
        exit(1);
    }
    
    char ackBuffer[4];
    ackBuffer[0] = '\x00';
    ackBuffer[1] = '\x00';
    ackBuffer[2] = '\x00';
    ackBuffer[3] = '\x00';
    unsigned int fail_count = 0;
    bool WRQ_flag = false;
    bool last_pkg = true;
    vector<const char*> files_vec;
    std::ofstream file("out.txt", std::ios::app);
    std::vector<std::ofstream> fileVector;
    struct sockaddr_in* curr_client = {0};
    cliAddrLen = sizeof(client_addr);
    if ((recvMsgSize = recvfrom(sockfd, (char *)buffer, BUFFMAX, 0,
                                (struct sockaddr *) &client_addr, &cliAddrLen)) < 0)
    {
        perror("TTFTP_ERROR:recvfrom failed");
        exit(1);
    }
    curr_client = &client_addr;
    unsigned int expected_block_num = 1;

    while (1) /* Run forever */
    { 
        cout << "" << endl;
        cout << "fail_count = " << fail_count << endl;
        std::cout << "start listening on ip = " << inet_ntoa(my_addr.sin_addr) << " and port = " << my_addr.sin_port << endl;
        if(curr_client != &client_addr && last_pkg==false )
        {
            //unexpected packet, new client while handling another
            std::cout << "in  Unexpected packet" << endl;
            char response[4];
            response[0] = '\x00';
            response[1] = '\x05';
            response[2] = '\x00';
            response[3] = '\x04';
            const char* respnse_msg = "Unexpected packet";
            int len = strlen(response) + strlen(respnse_msg) + 1;
            char* result = new char[len];
            strcpy(result, response);
            strcpy(result, respnse_msg);
            if (sendto(sockfd, result, len, 0, (struct sockaddr *) curr_client, sizeof(*curr_client)) != len)
            {
                perror("TTFTP_ERROR:sendto failed");
                exit(1);
            }
        }  
        curr_client = &client_addr;
        if(fail_count >= max_num_of_resp)
        {
            //abndoned file transmission
            if(!files_vec.empty())
            {
                if (remove(files_vec.back()) != 0) {
                    std::perror("TTFTP_ERROR:remove file failed");
                    return 1;
                }
                files_vec.pop_back();
            }
            std::cout << "in Abndoned file transmission" << endl;
            char response[4];
            response[0] = '\x00';
            response[1] = '\x05';
            response[2] = '\x00';
            response[3] = '\x00';
            const char* respnse_msg = "Unexpected packet";
            int len = strlen(response) + strlen(respnse_msg) + 1;
            char* result = new char[len];
            strcpy(result, response);
            strcpy(result, respnse_msg);
            if (sendto(sockfd, result, len, 0, (struct sockaddr *) curr_client, sizeof(*curr_client)) != len)
            {
                perror("TTFTP_ERROR:sendto failed");
                exit(1);
            }
            fail_count = 0;
        }

        printf("Handling client %s\n",inet_ntoa(curr_client->sin_addr));
        unsigned int opcode = (buffer[0]<<8) | buffer [1];
        buffer[recvMsgSize] = '\0';
        std::cout << "recvMsgSize = " << recvMsgSize << endl;
        std::cout << "opcode  = " << opcode << endl;        


        if(opcode == 2)
        { 
            std::cout << "Received WRQ from client" << endl;
            if(last_pkg == false)
            {
                //unexpected packet,same client with another WRQ 
                std::cout << "in  Unexpected packet" << endl;
                char response[4];
                response[0] = '\x00';
                response[1] = '\x05';
                response[2] = '\x00';
                response[3] = '\x04';
                const char* respnse_msg = "Unexpected packet";
                int len = strlen(response) + strlen(respnse_msg) + 1;
                char* result = new char[len];
                strcpy(result, response);
                strcpy(result, respnse_msg);
                if (sendto(sockfd, result, len, 0, (struct sockaddr *) curr_client, sizeof(*curr_client)) != len)
                {
                    perror("TTFTP_ERROR:sendto failed");
                    exit(1);
                }
            }
            WRQ_flag = true;
            const char* filename = buffer + 2;  // Skip the opcode (2 bytes)
            std::cout << "Filename: " << filename << std::endl;
            bool filename_exist = false;
            for(unsigned int i=0; i<files_vec.size(); i++)
            {
                if(strcmp(files_vec[i], filename) == 0) 
                {
                    filename_exist = true;
                    break;
                }
            }

            if(filename_exist)
            {
                //filename is already exist
                cout << "in filename is already exist" << endl;
                char response[4];
                response[0] = '\x00';
                response[1] = '\x05';
                response[2] = '\x00';
                response[3] = '\x06';
                const char* respnse_msg = "File already exists";
                int len = strlen(response) + strlen(respnse_msg) + 1;
                char* result = new char[len];
                strcpy(result, response);
                strcpy(result, respnse_msg);
                if (sendto(sockfd, result, len, 0, (struct sockaddr *) curr_client, sizeof(*curr_client)) != len)
                {
                    perror("TTFTP_ERROR:sendto failed");
                    exit(1);
                } 
            }
            else
            {
                char* new_file = new char[strlen(filename) + 1];
                strcpy(new_file, filename);
                fileVector.push_back(std::ofstream(new_file));
                files_vec.push_back(new_file);
                last_pkg = false;
                const char* mode = filename + strlen(filename) + 1;  // Skip the filename and null terminator
                std::cout << "Mode: " << mode << std::endl;
                ackBuffer[0] = '\x00';
                ackBuffer[1] = '\x04';
                ackBuffer[2] = '\x00';
                ackBuffer[3] = '\x00';
                if (sendto(sockfd, ackBuffer, 4, 0, (struct sockaddr *) curr_client, sizeof(*curr_client)) != 4)
                {
                    perror("TTFTP_ERROR:sendto failed");
                    exit(1);
                }
                expected_block_num = 1;
            }
        }

        else if(opcode == 3)
        {
            cout << "Received DATA from client" << endl;
            if( WRQ_flag == false)
            {
                //uknown user
                cout << "in unknown user" << endl;
                char response[4];
                response[0] = '\x00';
                response[1] = '\x05';
                response[2] = '\x00';
                response[3] = '\x07';
                const char* respnse_msg = "Unknown user";
                int len = strlen(response) + strlen(respnse_msg) + 1;
                char* result = new char[len];
                strcpy(result, response);
                strcpy(result, respnse_msg);
                if (sendto(sockfd, result, len, 0, (struct sockaddr *) curr_client, sizeof(*curr_client)) != len)
                {
                    perror("TTFTP_ERROR:sendto failed");
                    exit(1);
                }                
            }

            unsigned int recieved_pack = (buffer[2]<<8) | buffer[3];
            if(recieved_pack != expected_block_num)
            {
                //bad block number
                cout << "in bad block number" << endl;
                cout << "recieved_pack = " << recieved_pack << endl;
                cout << "expected_block_num = " << expected_block_num << endl;
                if(!files_vec.empty())
                {
                    if (remove(files_vec.back()) != 0) {
                        std::perror("TTFTP_ERROR:remove file failed");
                        return 1;
                    }
                    files_vec.pop_back();
                }

                char response[4];
                response[0] = '\x00';
                response[1] = '\x05';
                response[2] = '\x00';
                response[3] = '\x00';
                const char* respnse_msg = "Bad block number";
                int len = strlen(response) + strlen(respnse_msg) + 1;
                char* result = new char[len];
                strcpy(result, response);
                strcpy(result, respnse_msg);
                if (sendto(sockfd, result, len, 0, (struct sockaddr *) curr_client, sizeof(*curr_client)) != len)
                {
                    perror("TTFTP_ERROR:sendto failed");
                    exit(1);
                }  
            }
            else
            {
                char* data = buffer + 4;
                if( !last_pkg )(fileVector.back()) << data << endl;
                expected_block_num++;
                ackBuffer[0] = '\x00';
                ackBuffer[1] = '\x04';
                ackBuffer[2] = buffer[2];
                ackBuffer[3] = buffer[3];

                if (sendto(sockfd, ackBuffer, 4, 0, (struct sockaddr *) curr_client, sizeof(*curr_client)) != 4)
                {
                    perror("TTFTP_ERROR:sendto failed");
                    exit(1);
                }
                if (recvMsgSize < BUFFMAX) 
                {
                    expected_block_num--;
                    last_pkg = true; 
                }
            }

        }

        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        int selectResult = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        if (selectResult == -1) {
            perror("TTFTP_ERROR: select failed");
            exit(1);
        } else if (selectResult == 0) {
            // Timeout occurred, no packet received
            std::cout << "Timeout occurred. No packet received within the specified timeout." << endl;
            fail_count++;
            if (sendto(sockfd, ackBuffer, 4, 0, (struct sockaddr *) curr_client, sizeof(*curr_client)) != 4)
            {
                perror("TTFTP_ERROR:sendto failed");
                exit(1);
            }   
            continue;
        }
        cliAddrLen = sizeof(client_addr);
        if ((recvMsgSize = recvfrom(sockfd, (char *)buffer, BUFFMAX, 0,
                                    (struct sockaddr *) &client_addr, &cliAddrLen)) < 0)
        {
            perror("TTFTP_ERROR:recvfrom failed");
            exit(1);
        }
    }
    /* NOT REACHED */
    
    return 0;
}
