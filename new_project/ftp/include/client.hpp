#include <arpa/inet.h>
#include <cstdlib>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include <stdio.h>
#include <map>
#include <thread>
#include <random>
#include <string.h>
#include <iostream>
#include <unistd.h>
using namespace std;

constexpr char SERVER_IP[] = "127.0.0.1";
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

class client {
public:
  client();
  ~client();
  void client_connect();
  void client_control();
  int client_data_connectivity();
  void client_read_catelog(int data_fd);
  void client_download_file(int data_fd,vector<string> result);
  void client_upload_file(int data_fd,vector<string> result);
  int analysis_port(string port);
  vector<string> split(string order);
  int client_sock;
  struct sockaddr_in data_addr;
  struct sockaddr_in server_addr;
  char buffer[];
};