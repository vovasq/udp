//
// Created by Vovas on 27.11.2017.
//

#include "utils.h"


//auto logger = spdlog::rotating_logger_mt("clients_logger", "logthreads", 1024*1024, 1);
//auto mainlog = spdlog::rotating_logger_mt("main_logger", "log", 1024*1024, 1);

std::vector<std::pair<pthread_t, long> > clients;
// id, pair(num of datagrams, socket_addr in)
std::map<int, client> udp_clients;


std::map<int, client> clients_map;
std::map<int, item> items_map;

pthread_mutex_t vector_clients_mutex;
pthread_mutex_t map_clients_mutex;
pthread_mutex_t map_items_mutex;

std::array<std::string, COMMAND_NUM> command_list{{
//        "SEND",
                                                          "kill - delete client by number",
                                                          "killall - shutdown server",
                                                          "ls - showClients the list of connected clients",
                                                          "help - repeat the help list again"}
};

bool isManagerLogged = false;


// Getting a unique 32-bit ID is intuitively simple: the next one.
// Works 4 billion times. Unique for 136 years if you need one a second
int generateUserID() {
    static int seed = 0;
    return ++seed;
//    return ++generator_id;
}

int generateItemID() {
    static int seed = 0;
    return ++seed;
}

bool kill_client(int id) {
    pthread_mutex_lock(&vector_clients_mutex);
//    logger->info("start to kill client # {}", id);
    if (clients.size() < id) {
//        logger->info("no client # {}", id);
        pthread_mutex_unlock(&vector_clients_mutex);
        return false;
    }
    auto it = clients.begin() + (id - 1);
//    std::cout << "it->first = "<< it->first << std::endl;
    if (shutdown(it->second, SHUT_RDWR) == 0)
        std::cout << "success sock off client # " << it->second << std::endl;
//        logger->info("success sock off client # {}", it->second);
    else {
//        logger->error("error sock off client # {} , error = ", it->second, errno);
//        pthread_mutex_unlock(&vector_clients_mutex);
        return false;
    }
    if (close(it->second) == 0)
        std::cout << "kill client:  success sock close client # " << it->second << std::endl;
//        logger->info("success sock close client # {}", it->second);
    else {
        std::cout << "kill client: error sock close client  #" << id << "errno" << errno << std::endl;
//        logger->error("error sock close client  # {}, error = ", id, errno);
//        pthread_mutex_unlock(&vector_clients_mutex);
        return false;
    }
    if (pthread_join(it->first, NULL) == 0)
        std::cout << "kill client: success thread join client # " << it->second << std::endl;
//        logger->info("success thread join client  # {}", it->second);
    else {
        std::cout << "kill client: error thread join client # " << it->second << std::endl;
//        logger->error("error thread join client  # {}", it->second);
        pthread_mutex_unlock(&vector_clients_mutex);
        return false;
    }
    clients.erase(it);
    pthread_mutex_unlock(&vector_clients_mutex);
    return true;

}

void showCommands() {
    std::cout << "List of commands is:" << std::endl;
    for (auto it = command_list.begin();
         it != command_list.end(); it++) {
        std::cout << *it << "\n";
    }

}

void showClients() {
//    pthread_mutex_lock(&vector_clients_mutex);
    if (clients.size() != 0) {
        int i = 1;
        for (auto it : clients) {
//            i++;
//            TODO:
            std::cout << "client id" << i++ << " thread: " //<< it.first
                      << " number of socket: " << it.second << std::endl;
        }
    } else
        std::cout << "no connected clients" << std::endl;
//    pthread_mutex_unlock(&vector_clients_mutex);
}

int readn(int sock, char *buff, int size, int flag) {
    int recv_size = 0;
    while (recv_size < size) {
        char temp_buff[size];
        for (int j = 0; j < size; ++j) {
            temp_buff[j] = 0;
        }
//        TODO:
        int temp_recv = recv(sock, temp_buff, size, 0);
        if (temp_recv <= 0)
            return -1;
        for (int i = 0; i < temp_recv; i++)
            buff[i + recv_size] = temp_buff[i];
        recv_size += temp_recv;
    }
    return recv_size;
}

std::string getCommandFromMsg(const char *charmsg) {
    std::string msg(charmsg);
    return msg.substr(1, 4);
}

std::string create_message(std::string role, const char *command, std::string data) {
    std::string message = "";
//    char buf[30];
//    message += itoa(role, buf, 10);
    message += role;
    message += command;
    message += data;
    message += '\0';
    std::cout << "msg on creation : " << message << std::endl;
    return message;
}

//split function as python builts-in
std::vector<std::string> split(std::string s, std::string delimiter) {
    std::vector<std::string> list;
    std::cout << "solit" << std::endl;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);
    std::cout << "solit size = " << list.size() << std::endl;

    return list;
}

bool isLogged(std::string login) {
    for (auto it : clients_map)
        if (it.second.login == login)
            return true;
    return false;

}

void print_clients() {
    if (isManagerLogged)
        std::cout << "Manager is logged in" << std::endl;
    else
        std::cout << "Manager is not logged in" << std::endl;

    if (clients_map.size() == 0) {
        std::cout << "No clients connected" << std::endl;
        return;
    }
    std::cout << "Clients:" << std::endl;
    for (auto it : clients_map)
        std::cout << "id = " << it.first
                  << " login = " << it.second.login
                  << " pswd = " << it.second.password << std::endl;
}

void print_udp_clients() {

    if (isManagerLogged)
        std::cout << "Manager is logged in" << std::endl;
    else
        std::cout << "Manager is not logged in" << std::endl;

    if (udp_clients.size() == 0) {
        std::cout << "No clients connected" << std::endl;
        return;
    }
    std::cout << "Clients:" << std::endl;
    for (auto it : udp_clients)
        std::cout << "client id = " << it.first
                << " num of datagrams  = " << it.second.count_DG
                << " name = " << it.second.login
                << std::endl;

}

void print_items() {
    if (items_map.size() == 0) {
        std::cout << "No items in the list" << std::endl;
        return;
    }
    std::cout << "Items:" << std::endl;
    for (auto it : items_map)
        std::cout << "id = " << it.first
                  << " price =  " << it.second.price
                  << " login = " << it.second.name
                  << " holder =  " << it.second.holder
                  << std::endl;
}

bool createAndSendMsg(int Socket, int userID, sockaddr_in clientaddr, const char * role,
                      const char * command, std::string data){

    socklen_t clientlen = sizeof(clientaddr);
    std::string addUDP = "";
    if(userID >= 0){
        addUDP += std::to_string(userID);
        addUDP += " ";
        addUDP += std::to_string(udp_clients[userID].count_DG);
    } else if( userID == MANAGER_ID){
        addUDP += std::to_string(userID);
        addUDP += " ";
        addUDP += std::to_string(udp_clients[userID].count_DG);
    } else{
        addUDP += std::to_string(NOBODY_ID);
    }

    std::string message = create_message(role, command, addUDP + " " + data);

    if(sendto(Socket, message.c_str(), message.size(), 0, (struct sockaddr *)&clientaddr,
              clientlen) < message.size()){

        std::cout << "error sending message"  << message << std::endl;
        return false;
    } else{

        if(userID != NOBODY_ID){
            std::cout << "countDg++" << std::endl;
            udp_clients[userID].count_DG++;
        } else
            std::cout << "no countDG++" << std::endl;
        std::cout << "SUCCESS send message "  << message << std::endl;
    }
    return true;
}

void broadcastSend(int socket, std::string data, int exceptID){
    for (auto it: udp_clients){
        if(exceptID == BROADCAST_ALL || exceptID != it.first)
            if(!createAndSendMsg(socket, it.first, it.second.client_addr, SERVER, BROADCAST, data))
                std::cout << "ERROR send msg  for client id = " << it.first << std::endl;
    }
}

//void *run_client(void *param) {
////    SOCKET sock = (SOCKET *)param;
//    bool isThisManager = false;
////    int managerID = 0;
//    client manager;
//    long sock = (long) param;
//    while (true) {
//        char Buffer[MAX_MESSAGE_SIZE];
//        for (int j = 0; j < MAX_MESSAGE_SIZE; ++j) {
//            Buffer[j] = 0;
//        }
////        logger->info("sock number {}", sock);
////        if(recv(sock, Buffer, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == -1)
//        if (readn(sock, Buffer, MAX_MESSAGE_SIZE, 0) == -1) {
//            break;
//        }
//        std::string msg(Buffer);
//        std::cout << msg.length()<<std::endl;
//        if (msg.compare(1, 4, LOGIN) == 0) {
//            std::string pswd;
//            std::string login;
//            std::cout << "len msg = " << msg.length() << std::endl;
//            std::vector<std::string> log_and_pqsswd = split(msg.substr(5, msg.size()), " ");
////            login = log_and_pqsswd.at(log_and_pqsswd.size());
////            pswd = log_and_pqsswd[5];
////            std::cout <<  "Size = " << log_and_pqsswd.size() << std::endl;
//            for(int i = 0; i < log_and_pqsswd.size(); i++)
//                std::cout << log_and_pqsswd[i] << std::endl;
//
//            std::cout << "detected packet contains " << std::endl;
//            std::cout << "login = " << login << " pswd = " << pswd << std::endl;
//            if (msg.compare(0, 1, MANAGER) == 0) {
//                // manager_name checking is actuualy useless: login == manager_name &&
//                if (!isManagerLogged) {
//                    if (manager_password == pswd) {
//                        isManagerLogged = true;
//                        isThisManager = true;
//                        std::string message = create_message(SERVER, ACKNOWLEDGE, MANAGER);
//                        if (send(sock, message.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                            std::cout << "error send" << std::endl;
//                        std::string data = " ";
//                        broadcastSend(sock, data  + START);
//                    } else {
//                        std::string message = create_message(SERVER, ERROR, ERR_MANAGER_WRONG_PSWD);
//                        if (send(sock, message.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                            std::cout << "error send" << message << std::endl;
//
//                    }
//                } else {
//                    std::string message = create_message(SERVER, ERROR, ERR_MANAGER_ALREADY_EXISTS);
//                    if (send(sock, message.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                        std::cout << "error send" << std::endl;
//                }
//            } else if (isLogged(login)) {
//                std::cout << "contains" << std::endl;
//                std::string message = create_message(SERVER, ERROR, ERR_USER_ALREADY_EXISTS);
//                if (send(sock, message.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                    std::cout << "error send" << std::endl;
//                break;
//            } else {
//                client newClient;
//                newClient.login = login;
//                newClient.id = generateUserID();
//                newClient.password = pswd;
//                clients_map.insert(std::pair<int, client>(newClient.id, newClient));
//                std::string data = USER;
//                char buf[30];
//                data += std::to_string(newClient.id);
//                std::string message = create_message(SERVER, ACKNOWLEDGE, data);
//                if (send(sock, message.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                    std::cout << "error send message to user with id =  " << newClient.id << std::endl;
//            }
//        } else if (msg.compare(1, 4, EXIT) == 0 && msg.compare(0, 1, MANAGER) != 0) {
//            std::cout << "User going to logout" << std::endl;
//            std::cout << "Received message = " << msg << std::endl;
//            std::string str_id = msg.substr(5, msg.size() - 5);//str_response.size() - 6);
//            int id = atoi(str_id.c_str());
//            if (clients_map.erase(id) == 1)
//                std::cout << "Successfully remove user with id = " << id << std::endl;
//            else
//                std::cout << "Error remove user with id = " << id << std::endl;
//
//        } else if (msg.compare(1, 4, NEWITEM) == 0) {
////            MANAGER, NEWITEM, item_name + " " + item_price
//            std::cout << "we have a message to create new item" << std::endl;
//            std::vector<std::string> name_and_price = split(msg.substr(5, MAX_MESSAGE_SIZE), " ");
//            item newItem;
//            newItem.id = generateItemID();
//            newItem.name = name_and_price[0];
//            newItem.price = atoi(name_and_price[1].c_str());
//            std::cout << "detected item name " << newItem.name
//                      << " item price = " << newItem.price << std::endl;
//            items_map.insert(std::pair<int, item>(newItem.id, newItem));
//            // TODO: send acknowledge of adding item
//
//        } else if (msg.compare(1, 4, GETLIST) == 0) {
//
//            std::cout << "we have a message to send list of items" << std::endl;
////            SENDLIST
//            char buf[30];
//            std::string data = "";
//            data += std::to_string(items_map.size());
//            std::string message = create_message(SERVER, SENDLIST, data);
////                std::cout << "MSG =  " << message << std::endl;
//            if (send(sock, message.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                std::cout << "error send" << std::endl;
//            int msg_number = 1;
//
//            pthread_mutex_lock(&map_items_mutex);
//            for (auto it = items_map.begin(); it != items_map.end(); it++) {
//                char buf[30];
//                std::string data = "";
//                data += std::to_string(msg_number);
//                data += " ";
//                data += std::to_string(it->first);
//                data += " ";
//                data += it->second.name + " ";
////                char buf1[30];
//                data += std::to_string(it->second.price);
//                data += " ";
//                data += it->second.holder;
//                std::string message = create_message(SERVER, SENDLIST, data);
//                if (send(sock, message.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                    std::cout << "error send" << std::endl;
//                msg_number++;
//            }
//            pthread_mutex_unlock(&map_items_mutex);
//
//        } else if (msg.compare(1, 4, RISE) == 0){
//            std::cout << "we get a request to rise" << std::endl;
//            std::vector<std::string> item_id_and_price = split(msg.substr(5, MAX_MESSAGE_SIZE), " ");
//            int user_id = atoi(item_id_and_price[0].c_str());
//            int item_id = atoi(item_id_and_price[1].c_str());
//            int new_price = atoi(item_id_and_price[2].c_str());
//            std::cout   << "user id = " << user_id
//                        << " item id = " << item_id
//                        << " new price = " << new_price
//                        << std::endl;
//            // check for such an id in items
//            pthread_mutex_lock(&map_items_mutex);
//            if(items_map.find(item_id) != items_map.end()){
//                // check for such an price
//                if(items_map.at(item_id).price < new_price) {
//                    std::string response = create_message(SERVER, ACKNOWLEDGE, " ");
//                    if (send(sock, response.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                        std::cout << "error send" << std::endl;
//                    else{
//                        items_map.at(item_id).price = new_price;
//                        items_map.at(item_id).holder = clients_map.at(user_id).login;
//                        items_map.at(item_id).holder_id = user_id;
//                    }
//                }else{
//                    std::string response = create_message(SERVER, ERROR, ERR_ITEM_PRICE_TOO_LOW);
//                    if (send(sock, response.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                        std::cout << "error send" << std::endl;
//                }
//                pthread_mutex_unlock(&map_items_mutex);
//                // if items holder changes
//                if(items_map[item_id].holder_id != user_id)
//                    continue;
//                char buf[30];
//                std::string data = "";
//                data += clients_map[user_id].login;
//                data += " ";
//                data += std::to_string(new_price);
//                data += " ";
//                data += items_map[item_id].name;
//                data += " ";
//                data += std::to_string(item_id);
//                data += " ";
//                data += std::to_string(user_id);
//                broadcastSend(sock,data);
//            }else{
//                std::string response = create_message(SERVER, ERROR, ERR_ITEM_WRONG_ID);
//                if (send(sock, response.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                    std::cout << "error send" << std::endl;
//                pthread_mutex_unlock(&map_items_mutex);
//            }
//
//        } else if (msg.compare(1, 4, DONE) == 0){
//            std::cout << "we get a request to done" << std::endl;
//            std::vector < std::string > item_split= split(msg.substr(5, MAX_MESSAGE_SIZE), " ");
//            int item_id = atoi(item_split[1].c_str());
////                int new_price = atoi(item_id_and_price[2].c_str());
//            std::cout   << "done  item id = " << item_id
//                        << std::endl;
//            // check for such an id in items
//            pthread_mutex_lock(&map_items_mutex);
//            if(items_map.find(item_id) != items_map.end()) {
//                // check for such an price
//                int  user_id = items_map[item_id].holder_id;
//                char buf[30];
//                std::string data = "";
//                data += clients_map[user_id].login;
//                data += " ";
//                data += std::to_string(items_map[item_id].price);
//                data += " ";
//                data += std::to_string(item_id);
//                data += " ";
//                data += std::to_string(user_id);
//                data += " ";
//                data += APPROVE;
//                if(items_map.erase(item_id) == 1)
//                    std::cout << "item removed" << std::endl;
//                broadcastSend(sock,data);
//                pthread_mutex_unlock(&map_items_mutex);
//            }
////            items_map.insert(std::pair<int, item>(newItem.id, newItem));
//        }
////        else if (msg.compare(1, 4, STOP) == 0){
////
////        }
//
//    }
//    if(isThisManager){
//        std::string data  = " ";
//        broadcastSend(sock, data + STOP);
//        isManagerLogged = false;
//        pthread_mutex_lock(&map_items_mutex);
//        items_map.erase(items_map.begin(), items_map.end());
//        pthread_mutex_unlock(&map_items_mutex);
//    }
////
////    for(auto i: sons){
////        pthread_join(i, NULL);
////    }
//    std::cout << "success sock off client # " << sock << std::endl;
////        logger->info("success sock off client with sock # {}", sock);
//
//    if(shutdown(sock, SHUT_RDWR) == 0)
//    {
//        std::cout << "success sock off client with sock # "<<  sock << std::endl;
//        if(close(sock) == 0)
//        {
//            auto it = clients.cbegin();
//            for(; it !=  clients.cend(); it++)
//            {
//                if(it->second == sock)
//                {
//                    std::cout << "success delete client with sock # "<< sock << std::endl;
//                    break;
//                }
//            }
//            if(it != clients.cend())
//                clients.erase(it);
//            std::cout << "success sock close client with sock # " << sock << std::endl;
//        }
//        else
//        {
//            std::cout << "error sock close client  with sock # " << sock<< std::endl;
//        }
//    }
//    else
//    {
//        if(errno == EBADF)
//            std::cout << "sock is not connected # " <<  sock << std::endl;
//        else
//            std::cout <<"error unreachable sock # " << sock << std :: endl;
//    }
//    std::cout << "we finish " << sock << std::endl;
////    logger->info("we finish {}", sock);
//    pthread_exit(NULL);
//
//}



void *run_server(void *param) {
    int MainSock; /* socket */
    //    int managerID = 0;
//    bool isManagerLogged = false;
    int managerID = -1;
    struct sockaddr_in manager_addr; /* manager addr */
    int manager_count_DG = 0;
    int portno = DEFAULT_PORT; /* port to listen on */
    socklen_t  clientlen; /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    int optval; /* flag value for setsockopt */
    /*
     * socket: create the parent socket
     */
    MainSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (MainSock < 0)
        std::cout << "Error with socket descriptor" << std::endl;
    /* setsockopt: Handy debugging trick that lets
     * us rerun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    optval = 1;
    setsockopt(MainSock, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval , sizeof(int));

    /*
     * build the server's Internet address
     */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    /*
     * bind: associate the parent socket with a port
     */
    if (bind(MainSock, (struct sockaddr *) &serveraddr,
             sizeof(serveraddr)) < 0)
        std::cout << "ERROR on binding" << std::endl;


    if (pthread_mutex_init(&vector_clients_mutex, NULL) != 0) {
        std::cout << "mutex init failed" << std::endl;
        pthread_exit(NULL);
    }
    if (pthread_mutex_init(&map_clients_mutex, NULL) != 0) {
        std::cout << "mutex init failed" << std::endl;
        pthread_exit(NULL);
    }    if (pthread_mutex_init(&map_items_mutex, NULL) != 0) {
        std::cout << "mutex init failed" << std::endl;
        pthread_exit(NULL);
    }

    clientlen = sizeof(sockaddr_in);
    while (true) {
        struct sockaddr_in clientaddr; /* client addr */
        struct hostent *hostp; /* client host info */
//    char buf[MAX_MESSAGE_SIZE]; /* message buf */
        char *hostaddrp; /* dotted decimal host addr string */

        char Buffer[MAX_MESSAGE_SIZE];
        for (int j = 0; j < MAX_MESSAGE_SIZE; ++j) {
            Buffer[j] = 0;
        }

//        logger->info("sock number {}", sock);
//        if(recv(sock, Buffer, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == -1)
//        if (readn(sock, Buffer, MAX_MESSAGE_SIZE, 0) == -1) {
        int request = recvfrom(MainSock, Buffer, MAX_MESSAGE_SIZE, 0,
                               (struct sockaddr *) &clientaddr, &clientlen) ;
        if ( request == -1) {
            break;
        }
        else{
            std::cout << "request  = "<< request << std::endl;
        }

        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                              sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp == NULL)
            std::cout << "ERROR on gethostbyaddr" << std::endl;
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL)
            std::cout << "ERROR on inet_ntoa"<< std::endl;
        std::cout << "server received: "<<  Buffer << " " << std::endl;

        /*
         * sendto: echo the input back to the client
         */

        std::string msg(Buffer);
        std::cout << msg.length()<<std::endl;

        if (msg.compare(1, 4, LOGIN) == 0) {
            std::string pswd;
            std::string login;
            std::cout << "len msg = " << msg.length() << std::endl;
            std::vector<std::string> log_and_pqsswd = split(msg.substr(5, MAX_MESSAGE_SIZE), " ");
            login = log_and_pqsswd[2];
            pswd = log_and_pqsswd[3];
            std::cout << "detected packet contains " << std::endl;
            std::cout << "login = " << login << " pswd = " << pswd << std::endl;
            if (msg.compare(0, 1, MANAGER) == 0) {
                if (!isManagerLogged) {
                    if (manager_password == pswd) {
                        isManagerLogged = true;

                        manager_addr = clientaddr;
                        client newClient;
                        newClient.login = login;
                        newClient.id = MANAGER_ID;
                        newClient.password = pswd;
                        newClient.client_addr = clientaddr;
                        newClient.count_DG += 1;
                        //  std::cout << "new client id = " << newClient.id << std::endl;
                        clients_map.insert(std::pair<int, client>(newClient.id, newClient));
                        udp_clients.insert(std::pair<int, client>(newClient.id, newClient));

                        createAndSendMsg(MainSock, MANAGER_ID, manager_addr, SERVER, ACKNOWLEDGE, MANAGER);
                        broadcastSend(MainSock,START, MANAGER_ID);
                    } else {
                        createAndSendMsg(MainSock, NOBODY_ID, clientaddr, SERVER, ERROR, ERR_MANAGER_WRONG_PSWD);
                    }
                } else {
                    createAndSendMsg(MainSock, NOBODY_ID, clientaddr, SERVER, ERROR, ERR_MANAGER_ALREADY_EXISTS);
                }

            } else if (isLogged(login)) {
                std::cout << "contains" << std::endl;
                createAndSendMsg(MainSock, NOBODY_ID, clientaddr, SERVER, ERROR, ERR_USER_ALREADY_EXISTS);
//                break;
            } else {
                client newClient;
                newClient.login = login;
                newClient.id = generateUserID();
                newClient.password = pswd;
                newClient.client_addr = clientaddr;
                newClient.count_DG += 1;
                std::cout << "new client id = " << newClient.id << std::endl;
                clients_map.insert(std::pair<int, client>(newClient.id, newClient));
                udp_clients.insert(std::pair<int, client>(newClient.id, newClient));
                std::string data = USER;
                createAndSendMsg(MainSock,newClient.id, newClient.client_addr, USER, ACKNOWLEDGE, data);
            }
        }
        else if (msg.compare(1, 4, EXIT) == 0){ //&& msg.compare(0, 1, MANAGER) != 0) {
            std::cout << "User going to logout" << std::endl;
            std::cout << "Received message = " << msg << std::endl;
            std::vector<std::string> msgVector = split(msg.substr(5, MAX_MESSAGE_SIZE), " ");
            std::string str_id = msgVector[0]; //str_response.size() - 6);
//            for (int i = 0; i < msgVector.size(); i++)
//                std::cout << msgVector[i] << std::endl;
            int id = atoi(str_id.c_str());
            print_udp_clients();
            if (udp_clients.erase(id) == 1) { //clients_map.erase(id) == 1 &&
                std::cout << "Successfully remove user with id = " << id << std::endl;
//                createAndSendMsg(MainSock, id, clientaddr, SERVER, STOP, "");
                if(id == MANAGER_ID){
//                    std::string data  = " ";
                    broadcastSend(MainSock, STOP, BROADCAST_ALL);
                    isManagerLogged = false;
                    udp_clients.erase(id);
//                    manager_addr = 0;
                    pthread_mutex_lock(&map_items_mutex);
                    items_map.erase(items_map.begin(), items_map.end());
                    pthread_mutex_unlock(&map_items_mutex);
                }

            } else
                std::cout << "Error remove user with id = " << id << std::endl;

        }
        else if (msg.compare(1, 4, NEWITEM) == 0) {
//            MANAGER, NEWITEM, item_name + " " + item_price
            std::cout << "we have a message to create new item" << std::endl;
            std::vector<std::string> name_and_price = split(msg.substr(5, MAX_MESSAGE_SIZE), " ");
            item newItem;
            newItem.id = generateItemID();
            newItem.name = name_and_price[2];
            newItem.price = atoi(name_and_price[3].c_str());
            std::cout << "detected item name " << newItem.name
                      << " item price = " << newItem.price << std::endl;
            items_map.insert(std::pair<int, item>(newItem.id, newItem));
            // TODO: send acknowledge of adding item
        } else if (msg.compare(1, 4, RISE) == 0) {
            std::cout << "we get a request to rise" << std::endl;
            std::vector<std::string> item_id_and_price = split(msg.substr(5, MAX_MESSAGE_SIZE), " ");
            int user_id = atoi(item_id_and_price[0].c_str());
            int item_id = atoi(item_id_and_price[2].c_str());
            int new_price = atoi(item_id_and_price[3].c_str());
            std::cout << "user id = " << user_id
                      << " item id = " << item_id
                      << " new price = " << new_price
                      << std::endl;
            // check for such an id in items
            pthread_mutex_lock(&map_items_mutex);
            if (items_map.find(item_id) != items_map.end()) {
                // check for such an price
                if (items_map.at(item_id).price < new_price) {
                    if(!createAndSendMsg(MainSock, user_id, clientaddr, SERVER,ACKNOWLEDGE, "")){
                        std::cout << "error send" << std::endl;
                    }else{
                        items_map.at(item_id).price = new_price;
                        items_map.at(item_id).holder = clients_map.at(user_id).login;
                        items_map.at(item_id).holder_id = user_id;
                    }
                } else {

                    std::string response = create_message(SERVER, ERROR, ERR_ITEM_PRICE_TOO_LOW);
                    if(!createAndSendMsg(MainSock, user_id, clientaddr, SERVER, ERROR, ERR_ITEM_PRICE_TOO_LOW))
                        std::cout << "error send" << std::endl;
                    continue;
                }
                pthread_mutex_unlock(&map_items_mutex);
                // if items holder changes
//                if(items_map[item_id].holder_id != user_id)
//                    continue;

                std::string data = "";
//                data += clients_map[user_id].login;
                data += udp_clients[user_id].login;
                data += " ";
                data += std::to_string(new_price);
                data += " ";
                data += items_map[item_id].name;
                data += " ";
                data += std::to_string(item_id);
//                data += " ";
//                data += std::to_string(user_id);
                broadcastSend(MainSock, data, user_id);
            } else {
                createAndSendMsg(MainSock, user_id, clientaddr, SERVER, ERROR, ERR_ITEM_WRONG_ID);
                pthread_mutex_unlock(&map_items_mutex);
            }
        }else if (msg.compare(1, 4, DONE) == 0){
            std::cout << "we get a request to done" << std::endl;
            std::vector < std::string > item_split= split(msg.substr(5, MAX_MESSAGE_SIZE), " ");
            int item_id = atoi(item_split[2].c_str());
//                int new_price = atoi(item_id_and_price[2].c_str());
            std::cout   << "done  item id = " << item_id
                        << std::endl;
            // check for such an id in items
            pthread_mutex_lock(&map_items_mutex);
            if(items_map.find(item_id) != items_map.end()) {
                // check for such an price
                int user_id = items_map[item_id].holder_id;
                std::string data = "";
                data += udp_clients[user_id].login;
                data += " ";
                data += items_map[item_id].name;
                data += " ";
                data += std::to_string(items_map[item_id].price);
                data += " ";
                data += APPROVE;
                if(items_map.erase(item_id) == 1)
                    std::cout << "item removed" << std::endl;
                broadcastSend(MainSock,data, BROADCAST_ALL);
                pthread_mutex_unlock(&map_items_mutex);
            }
            pthread_mutex_unlock(&map_items_mutex);
            createAndSendMsg(MainSock, MANAGER_ID, clientaddr, SERVER, ERROR, ERR_ITEM_WRONG_ID);

        }
        else if (msg.compare(1, 4, GETLIST) == 0) {

            std::cout << "we have a message to send list of items" << std::endl;
//            SENDLIST
//            char buf[30];
//            int countItems = items_map.size();
            pthread_mutex_lock(&map_items_mutex);
            std::string data = std::to_string(items_map.size());
//            std::string message = create_message(SERVER, SENDLIST, data);
////                std::cout << "MSG =  " << message << std::endl;
//            if (send(sock, message.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                std::cout << "error send" << std::endl;
            std::vector<std::string> msgVector = split(msg.substr(5, MAX_MESSAGE_SIZE), " ");
            int user_id = atoi(msgVector[0].c_str());
            createAndSendMsg(MainSock, user_id, clientaddr, SERVER, SENDLIST, data);

            int msg_number = 1;

            for (auto it = items_map.begin(); it != items_map.end(); it++) {
                std::string data = "";
                data += std::to_string(msg_number);
                data += " ";
                data += std::to_string(it->first);
                data += " ";
                data += it->second.name + " ";
//                char buf1[30];
                data += std::to_string(it->second.price);
                data += " ";
                data += it->second.holder;
                createAndSendMsg(MainSock, user_id, clientaddr, SERVER, SENDLIST, data);
//                std::string message = create_message(SERVER, SENDLIST, data);
//                if (send(sock, message.c_str(), MAX_MESSAGE_SIZE, 0) != MAX_MESSAGE_SIZE)
//                    std::cout << "error send" << std::endl;
                msg_number++;
            }
            pthread_mutex_unlock(&map_items_mutex);


//            items_map.insert(std::pair<int, item>(newItem.id, newItem));
        }
////        else if (msg.compare(1, 4, STOP) == 0){
////
////        }

    }
//    pthread_mutex_lock(&vector_clients_mutex);
//    for (auto it: clients) {
//        if (shutdown(it.second, SHUT_RDWR) == 0)
//            std::cout << "success sock shutdown " << it.second << std::endl;
//        else {
//            std::cout << "error shutdown sock " << it.second << std::endl;
//        }
//        if (close(it.second) == 0)
//            std::cout << "success sock close # " << it.second << std::endl;
//        else
//            std::cout << "error close sock # " << it.second << std::endl;
//
//        if (pthread_join(it.first, NULL) == 0)
//            std::cout << "success join client # " << it.second << std::endl;
//        else
//            std::cout << "error join client # " << it.second << std::endl;
//    }
//    clients.clear();
//    pthread_mutex_unlock(&vector_clients_mutex);
    std::cout << "server clients thread is done" << std::endl;
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    pthread_t server_init;

//    long MainSock = socket(AF_INET, SOCK_STREAM, 0);
//    struct sockaddr_in SockAddr;
//    SockAddr.sin_family = AF_INET;
//    SockAddr.sin_port = htons(DEFAULT_PORT);
//    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//
//    if(bind(MainSock, (struct sockaddr *) (&SockAddr), sizeof(SockAddr)) == 0)
//        std::cout << "Server is bind" << std ::endl;
//    else
//        std::cout << "Server bind failed"<< std ::endl;
//    if(listen(MainSock, SOMAXCONN) == 0)
//        std::cout << "Server is ready"<< std ::endl;
//    else
//        std::cout <<"Server does not listen" << std::endl;

    if(pthread_create(&server_init, NULL, run_server, NULL) == 0)
        std::cout << "Server is started" << std::endl;
    else
        std::cout << "Error: can not create server_init thread" << std::endl;
    showCommands();
    while (true) {
        std::string cmd_string;
        std::cout << "Enter your command:" << std::endl;

//        std::getline(std::cin, cmd_string);
        std::cin >> cmd_string;
        if (cmd_string.compare("help") == 0) {
            showCommands();
        } else if (cmd_string.compare("kill") == 0) {
            int client_to_del;
            if (clients.size() > 0) {
//                while(true){
                std::cout << "Input number of client:" << std::endl;
                std::cin >> client_to_del;
                if (client_to_del <= 0)
                    std::cout << "WRONG INPUT: client number can not be negative or zero!" << std::endl;
                else if (client_to_del > clients.size())
                    std::cout << "WRONG INPUT: " << clients.size() << " clients are connected" << std::endl;
                else {
                    if (kill_client(client_to_del) == true) {
                        std::cout << "client removed succesfully" << std::endl;
                        continue;
                    } else
                        std::cout << "error remove client see logthread.txt" << std::endl;
                    break;
                }
            } else
                std::cout << "no clients to remove" << std::endl;

        } else if (cmd_string.compare("killall") == 0) {
            std::cout << "The server is turning off...\n";
//            if(shutdown(MainSock, NULL) == 0)
//                std::cout << "shutdown main sock\n";
//            else
//            {
//                printf("shutdown failed with error: %d\n", WSAGetLastError());
// //                closesocket(MainSock);
// //                return 1;
//            }

            // nu takoe
            pthread_cancel(server_init);

//            if(shutdown(MainSock, 2) == 0)
//                std::cout<<"shutdown main sock\n";
//            else
//                std::cout<<"main sock vipil dva stakana\n";
//
//            if (close(MainSock) == 0)
//                std::cout << "close main sock\n";
//            else
//                std::cout << "error close main sock" << std::endl;

            break;
        } else if (cmd_string.compare("ls") == 0) {
            std::cout << "All the clients connected to the server are:" << std::endl;
//            showClients();
//            print_clients();
            print_udp_clients();
            print_items();
        } else
            std::cout << "No such a command:  " << cmd_string << "  ! Retry!" << std::endl;

    }

    if (pthread_join(server_init, NULL) == 0)
        std::cout << "Join is done # " << "\nBye!\n";

    return 0;
}