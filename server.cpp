#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

void split_ops(const string& s, vector<string>& v);
string asyncRPC(vector<string>& ops);
void serving(int sock);

const int PORT = 8080;

int main(int argc, char *argv[]) {
  if(argc != 1) {
    cerr << "Usage: " << argv[0] << " SRC DEST\n";
    return 1;
  }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "Failed socket" << endl;
        exit(1);
    }

    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(server_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Failed to bind" << endl;
        exit(1);
    }

    listen(sockfd, 5);

    char serveraddr_buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_addr.sin_addr, serveraddr_buf, INET_ADDRSTRLEN);
    cout << "Server Address: " << serveraddr_buf << endl;

    int clientfd = 0;
    while (1) {
        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
        if (clientfd < 0) {
            cerr << "error" << endl;
            exit(1);
        }

        serving(clientfd);
    }

  return 0;
}

void serving(int sock) {
    char recv_buff[4096] = {0};
    vector<string> ops;

    recv(sock, recv_buff, sizeof(recv_buff), 0);
    string recv_str(recv_buff);
    cout << "Message from client: " << recv_str << endl;


    string msg_send;
    split_ops(recv_str, ops);

    if(ops[0] == "asyncRPC") {
        msg_send = asyncRPC(ops);
    } else {
        cerr << "Bad op!" << endl;
    }

    send(sock, msg_send.c_str(), msg_send.size(), 0);
}


void split_ops(const string& s, vector<string>& v) {
    string::size_type pos1, pos2;
    pos2 = s.find(" ");
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + 1;
        pos2 = s.find(" ", pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

string asyncRPC(vector<string>& ops) {
    string s = "";
    if(ops[1] == "calculate_pi") {
        int m = 1, f = 1;
        double pi = 0, cur = 1.0/m;
        do {
            pi += cur*f*4;
            f = -f;
            m += 2;
            cur = 1.0/m;
        } while(fabs(cur) >= 0.0000001);
        s += "pi=" + to_string(pi);
    } else if(ops[1] == "add") {
        int a = atoi(ops[2].c_str()) + atoi(ops[3].c_str());
        s += "i+j=" + to_string(a);
    } else if(ops[1] == "sort") {
        int n = ops.size()-2;
        int arr[n];
        for (int i=2; i<n+2; i++) {
            arr[i-2] = atoi(ops[i].c_str());
        }
        sort(arr, arr+n);
        s += "Sorted array:";
        for (int i=0; i<n; i++) {
            s += " " + to_string(arr[i]);
        }
    } else if(ops[1] == "matrix_multiply") {
        int p = atoi(ops[2].c_str());
        int q = atoi(ops[3].c_str());
        int r = atoi(ops[4].c_str());
        vector<vector<double>> matrixA;
        vector<vector<double>> matrixB;
        for (int i=0; i<p; i++) {
            vector<double> v;
            for (int j=0; j<q; j++) {
                v.push_back(atof(ops[5+q*i+j].c_str()));
            }
            matrixA.push_back(v);
        }
        int c = 5+p*q;
        for (int i=0; i<q; i++) {
            vector<double> v;
            for (int j=0; j<r; j++) {
                v.push_back(atof(ops[c+r*i+j].c_str()));
            }
            matrixB.push_back(v);
        }

        double matrixC[p][r] = {0};
        for (int i=0; i<p; i++) {
            for (int j=0; j<r; j++) {
                for (int k=0; k<q; k++) {
                    matrixC[i][j] += matrixA[i][k]*matrixB[k][j];
                }
            }
        }
        s += "Matrix C:\n";
        for (int i=0; i<p; i++) {
            for (int j=0; j<r; j++) {
                s += to_string(matrixC[i][j]) + " ";
            }
            s += "\n";
        }
    } else {
        s += "BAD Operation\n";
    }
    return s;
}
