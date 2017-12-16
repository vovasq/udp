//
// Created by Vovas on 07.12.2017.
//

#ifndef TCP_SERVER_H
#define TCP_SERVER_H



#include <iostream>
#include <map>
#include <vector>
#include <pthread.h>
#include <algorithm>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>



#define DEFAULT_PORT 27015


// roles
#define MANAGER         "M"
#define USER            "U"
#define SERVER          "S"

// comands

#define COMMAND_LEN 4
#define ERROR           "eror"
#define EXIT            "exit"
#define LOGIN           "logi"
#define GETLIST         "getl"
#define RISE            "rise"
#define BROADCAST       "brod"
#define PASS            "pass"
#define SHUTDOWN        "shut"
#define STOP            "stop"
#define DELETEITEM      "deli"
#define NEWITEM         "newi"
#define SENDLIST        "sndl"
#define APPROVE         "aprv"
#define ACKNOWLEDGE     "ackn"
#define DENY            "deny"
#define START           "strt"


// errors

#define ERR_MANAGER_ALREADY_EXISTS  "MAE"
#define ERR_MANAGER_WRONG_PSWD      "WMP"
#define ERR_USER_ALREADY_EXISTS     "UAE"
#define ERR_ITEM_PRICE_TOO_LOW      "TLP"
#define ERR_ITEM_WRONG_ID           "WII"
#define ERR_ITEM_ALREADY_SOLD       "IAS"


const int MAX_MESSAGE_SIZE = 256;
const int MAX_PASSWORD_SIZE = 100;
const int MAX_LOGIN_SIZE = 100;
const int MAX_IT_NAME_SIZE = 100;
const int MAX_IT_PRICE_SIZE = 4;
const int MAX_HOLDER_NAME = 50;
const int BROADCAST_ALL = -1;

const int  COMMAND_NUM = 4;
const int ITEM_LOGIN_LEN = 100;
const int HOLDER_login_LEN = 100;
const char * manager_name  = "manager";
const char *manager_password = "1234";



//TODO: id is stored as a key in a clietns_map so id is useless in struct ???
//TODO: isert Socket to struct of client
struct client {
    std::string login;
    int id;
    std::string password;
};

struct item {
    std::string name;
    std::string holder = "manager"; // default holder
    int id;
    int price;
    // default id =  -1 matches to manager
    int holder_id = -1;
};



#endif //TCP_SERVER_H
