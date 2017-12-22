//
// Created by Vovas on 07.12.2017.
//


#include "utils.h"


pthread_mutex_t stdout_mutex;
pthread_t requestHandler;
int id;
int countDG = -1;
struct sockaddr_in serveraddr;


std::vector<std::string> sentMessages;

std::array<std :: string, COMMAND_NUM> command_list{{
        "exit",
        "rise",
        "getlist",
        "help - repeat the help list again"}
};


// int error_code;
// int error_code_size = sizeof(error_code);
// getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);

int readn(int socket, char *message, size_t length, int flags) {
    int received = 0;
    while (received < length) {
        char buffer[length];
        int size = recv(socket, buffer, length, flags);
        if (size <= 0) return size;
        for (int i = 0; i < size; i++)
            message[i + received] = buffer[i];
        received += size;
    }
    return received;
}

std::string create_message(const char * role, const char * command, std::string data) {
    std::string message = "";
    message += role;
    message += command;
    message += data;
    message += '\0';
    return message;
}

std::vector<std::string> split(std::string s, std::string delimiter) { //Разбивает строку по делиметру
    std::vector<std::string> list;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) { //пока не конец строки
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);
    return list;
}

void createAndSendMsg(int Socket, int userID, const char * role, const char * command, std::string data){

    socklen_t serverlen = sizeof(serveraddr);
    std::string delim = data.size() > 0 ? " " : "";
    ++countDG;
    std::string message = create_message(role, command,
                                         std::to_string(userID) + " " +
                                         std::to_string(countDG) +  delim + data);

    if(sendto(Socket, message.c_str(), message.size(), 0, (struct sockaddr *)&serveraddr, serverlen)
       < message.size()){

        // if(send(Socket, message, strlen(message) + 1, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE)
        std::cout << "error sending message " << message << std::endl;
        return;
    }
    else{
//        std::cout << "SUCCESS sent pack # : " << countDG << " message =  "<< message << std::endl;
        sentMessages.push_back(message);
    }

}

void resendMsg(int Socket, std::string message) {
    socklen_t serverlen = sizeof(serveraddr);
    if(sendto(Socket, message.c_str(), message.size(), 0, (struct sockaddr *)&serveraddr, serverlen)
       < message.size()){

        // if(send(Socket, message, strlen(message) + 1, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE)
        std::cout << "error sending message " << message << std::endl;
        return;
    }
    else{
        std::cout << "SUCCESS sent pack # : " << countDG << " message =  "<< message << std::endl;
        sentMessages.push_back(message);
    }

}

void * listener_init(void * param){
    // int Socket = *((int *)param);
    long Socket = (long)param;
    do{
        char response[MAX_MESSAGE_SIZE];
        socklen_t serverlen = sizeof(serveraddr);
        int receive = recvfrom(Socket, response, MAX_MESSAGE_SIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
        if(receive != -1 && receive != 0) {
            std::string response_str = response;
            std::vector<std::string> msgVector = split(response_str.substr(5, MAX_MESSAGE_SIZE), " ");
            int msgNumber = atoi(msgVector[1].c_str());
            if(msgNumber > countDG + 1) {

                if(response_str.compare(1, 4, BROADCAST) != 0)
                    resendMsg(Socket, sentMessages.back());
//                continue;
                break;
            }
            countDG++;
//            std::cout << " current countDg = " << countDG
//                      << " message countDg = " << msgVector[1] << std::endl;

            if (response_str.compare(1, 4, ERROR) == 0){
                std::string errMsg = msgVector[msgVector.size() - 1];
                if(errMsg.compare(ERR_ITEM_PRICE_TOO_LOW) == 0){
                    std::cout << "Price  is to loow for this item" << std::endl;
                } else if(errMsg.compare(ERR_ITEM_WRONG_ID) == 0){
                    std::cout << "There is no item  with this id" << std::endl;
                } else if(errMsg.compare(ERR_ITEM_ALREADY_SOLD) == 0){
                    std::cout << "There is no such an item" << std::endl;
                } else {
                    std::cout << "Uknown error from server: " << response_str.substr(5,3)
                              << std::endl;
                }
//                countDG++;
            } else if(response_str.compare(1, 4, ACKNOWLEDGE) == 0){
                std::cout << "Your price accepted waiting for other users"<< std::endl;
//                countDG++;
            } else if(response_str.compare(1, 4, SENDLIST) == 0){
//                std::cout << "listen handler" << std::endl;
//                std::vector<std::string> msgVector = split(response_str.substr(5, MAX_MESSAGE_SIZE), " ");
                std::string size = msgVector[msgVector.size() - 1];
                int size_of_list = atoi(size.c_str());
                std::cout << "size_of_list = " << size_of_list << std::endl;
                std::cout << " current countDg = " << countDG
                          << " message countDg = " << msgVector[1] << std::endl;
                for(int i = 0; i < size_of_list; i++) {
                    char item_buf[MAX_MESSAGE_SIZE];
                    if(recvfrom(Socket, item_buf, MAX_MESSAGE_SIZE, 0, (struct sockaddr *)&serveraddr, &serverlen) != -1) {
                        std::string str_item_buf = item_buf;
                        std::vector<std::string> item_vector = split(str_item_buf.substr(5, MAX_MESSAGE_SIZE), " ");
                        std::cout   << "msg # " << item_vector[2]
                                    << " id = " << item_vector[3]
                                    << " name =  " << item_vector[4]
                                    << " price = " << item_vector[5]
                                    << " holder =  " << item_vector[6]
                                    << std::endl;
                        countDG++;
                        std::cout << " current countDg = " << countDG
                                  << " message countDg = " << item_vector[1] << std::endl;


                    }else{
                        std::cout << "Error while reading items from server" << std::endl;
                        break;
                    }
                }
            } else if(response_str.compare(1, 4, BROADCAST) == 0){
                if(msgVector[msgVector.size() - 1] == START){
                    std::cout << "Manager is connected we are ready to start" << std::endl;
                } else if(msgVector[msgVector.size() - 1] == STOP){
                    std::cout << "Manager is disconnected we are finish" << std::endl;
                } else if(msgVector[msgVector.size() - 1] == APPROVE){
                    std::cout   << "Client " << msgVector[2]
                                << " gets item " << msgVector[3]
                                << " with price " << msgVector[4]
                                << std::endl;
                } else {
                    std::cout   << "Client " << msgVector[2]
                                << " rises price to " << msgVector[3]
                                << " for item " << msgVector[4]
                                << std::endl;

                }

            }else if(response_str.compare(1, 4, STOP) == 0) {

            } else{
                std::cout << "Uknown response from server"<< std::endl;
                break;
            }
        }else{
             std::cout << "We finish" << std::endl;
            break;
        }
    }while(true);
    pthread_cancel(requestHandler);
}

// void  user_dialogue(int id, int Socket){
void * user_dialogue(void * param){
    long Socket = (long) param;
    do {

        char Buffer[MAX_MESSAGE_SIZE];
        std::string request;
        std::cout << "Input your request:" << std::endl;
        // std::getline(std::cin, request);
        std::cin >> request;
        if(request.compare("exit") == 0){
            std::cout << "You are trying to logging out" << std::endl;
//            std::string message = create_message(USER, EXIT, "");
            createAndSendMsg(Socket, id, USER, EXIT, "");
            shutdown(Socket, SHUT_RDWR);
            close(Socket);
            break;
        }
        else if(request.compare("rise") == 0){
            int  item_id;
            int  new_price;
            std::cout << "Input item id" << std::endl;
            std::cin >> item_id;
            if(std::cin.fail()){
                std::cout << "Error: item id should be a number " << std::endl;
                std::cin.clear();
                continue;
            }

            std::cout << "Input new price for the item" << std::endl;
            std::cin >> new_price;
            if(std::cin.fail()){
                std::cout << "Error: price should be a number" << std::endl;
                std::cin.clear();
                continue;
            }
            std::cout << "Server is proccessing your request..." << std::endl;
            createAndSendMsg(Socket, id, USER, RISE, std::to_string(item_id) + " " + std::to_string(new_price));
        }

        else if(request.compare("getlist") == 0){
            std::string message = create_message(USER, GETLIST, " ");
            std::cout << "Waiting for the server..." << std::endl;

            createAndSendMsg(Socket, id, USER, GETLIST, "");
        }
        else if(request.compare("help") == 0){
            std::cout << "Here it is a list of commands" << std::endl;
        }
        else {
            std::cout << "Uknown request! Retry!" << std :: endl;
        }
    } while (true);
    pthread_exit(NULL);
}

// void manager_dialogue(int Socket){
void * manager_dialogue(void * param){
    long Socket = (long) param;
    do {
        char Buffer[MAX_MESSAGE_SIZE];
        std::string request;
        std::cout << "Input your request:" << std::endl;
        std::cin >> request;

        if(request.compare("exit") == 0){
            std::cout << "You are trying to logging out" << std::endl;

            createAndSendMsg(Socket, id, MANAGER, EXIT,"");

            shutdown(Socket, SHUT_RDWR);
            close(Socket);
            break;
        } else if(request.compare("additem") == 0){

            std::string item_name;
            unsigned int item_price;

            std::cout << "Input item name" << std::endl;
            std::cin >> item_name;
            if(item_name.size() > MAX_IT_NAME_SIZE) {
                std::cout << "Item Name should be not more than symbols"
                          << MAX_IT_NAME_SIZE << std::endl;
                continue;
            }

            std::cout << "Input item begin price" << std::endl;
            std::cin >> item_price;
            if(std::cin.fail()){
                std::cout << "Price should be a number" << std::endl;
                std::cin.clear();
                continue;
            }

            std :: string price_str = std::to_string(item_price);
            if(price_str.size() > MAX_IT_NAME_SIZE){
                std::cout << "Item price should be not more than symbols"
                          << MAX_IT_NAME_SIZE << std::endl;
                continue;
            }
            createAndSendMsg(Socket, id, MANAGER, NEWITEM, item_name + " " + price_str);

        } else if(request.compare("done") == 0){
            int  item_id;
            std::cout << "Input item id" << std::endl;
            std::cin >> item_id;
            if(std::cin.fail()){
                std::cout << "Error: item id should be a number " << std::endl;
                std::cin.clear();
                continue;
            }
            std::cout << "Server is proccessing your request..." << std::endl;
            createAndSendMsg(Socket, id, MANAGER, DONE, std::to_string(item_id));

        }
        else if(request.compare("getlist") == 0){
            std::cout << "Waiting for the server..." << std::endl;
            createAndSendMsg(Socket, id, MANAGER, GETLIST, "");

        } else if(request.compare("stop") == 0){
            std::cout << "Waiting for the server" << std::endl;
        } else if(request.compare("help") == 0){
            std::cout << "Here it is a list of commands" << std::endl;
        } else {
            std::cout << "Uknown request! Retry!" << std :: endl;
        }
    } while (true);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {

    int Socket, portno = DEFAULT_PORT, n;
    socklen_t  serverlen;
//    struct sockaddr_in serveraddr;
    struct hostent *server;
    char hostname[] = "localhost";

    /* socket: create the socket */
    Socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (Socket < 0)
        std::cout << "ERROR opening socket" << std::endl;

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    std::string s;
    long i;
    int tmp_size = 0;
    // logging in
    std::string login;
    std::string password;
    while(login.size() <= 0 && login.size() <= MAX_login_SIZE){
        std::cout << "Input your login:" <<std::endl;
        std::cin >> login;
    }
    while(password.size() <= 0 && password.size() <= MAX_PASSWORD_SIZE){
        std::cout << "Input your password:" << std::endl;
        std::cin >> password;
    }

    std::cout << "Checking on server..." << std::endl;
    // create message and send to the server
    std::string del = " ";
    std::string message;
    if(login.compare(manager_name) == 0){
        std::cout << "You are trying to logging as a manager" << std::endl;
//        message = create_message(MANAGER, LOGIN, login + del + password);
        createAndSendMsg(Socket, NOBODY_ID, MANAGER, LOGIN, login + del + password);
    } else{
//        message = create_message(USER, LOGIN, login + del + password);
        createAndSendMsg(Socket, NOBODY_ID, USER, LOGIN, login + del + password);
    }

//    /* send the message to the server */
//    serverlen = sizeof(serveraddr);
////    sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen)
//    if(sendto(Socket, message.c_str(), message.size(), 0, (struct sockaddr *)&serveraddr, serverlen)
//       < message.size()){
//        // if(send(Socket, message, strlen(message) + 1, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE)
//        std::cout << "error sending message" << std::endl;
//        return 1;
//    }
//    else{
//        std::cout << "countDg++" << std::endl;
//        countDG++;
//    }
    // get message from the server
    char response[MAX_MESSAGE_SIZE];
    // std :: cout << "len of message = " << strlen(message) << std::endl;
//    if(readn(Socket, response, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == MAX_MESSAGE_SIZE) {
    int res = recvfrom(Socket, response, MAX_MESSAGE_SIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
    if( res != -1 && res != 0) {
        std::string str_response = response;
        if(str_response.compare(1,4,ACKNOWLEDGE) == 0){
            countDG++;
            if (pthread_mutex_init(&stdout_mutex, NULL) != 0) {
                std::cout << "mutex init failed" << std::endl;
                pthread_exit(NULL);
            }

            std::vector<std::string> msgVector = split(str_response.substr(5, MAX_MESSAGE_SIZE), " ");
            if(msgVector[msgVector.size() - 1].compare(USER) == 0 ){
                // TODO: get user id  substr
                std::cout << "str size = " << str_response.size() << std::endl;
//                std::string str_id = str_response.substr(6, str_response.size() - 6);//str_response.size() - 6);
                std::vector<std::string> str_id = split(str_response.substr(5, MAX_MESSAGE_SIZE), " ");
                // int id = atoi(str_id.c_str());
                id = atoi(str_id[0].c_str());

                int packet_number = atoi(str_id[1].c_str());
                std::cout << "Pack # "<< packet_number
                          << " you are connected as user with id = "<< id << std::endl;

                if(pthread_create(&requestHandler, NULL, user_dialogue, (void *) (Socket)) == 0)
                    std::cout << "Thread for listen was successfully created" << std::endl;
                else
                    std::cout << "kekkeke" <<std::endl;

//                // pthread_create
            } else{
                std::vector<std::string> str_id = split(str_response.substr(7, MAX_MESSAGE_SIZE), " ");
                // int id = atoi(str_id.c_str());
                id = MANAGER_ID;
                std::cout << "You are connected as MANAGER" << std::endl;
                // TODO:
                if(pthread_create(&requestHandler, NULL, manager_dialogue, (void *) (Socket)) == 0)
                    std::cout << "Thread for listen was successfully created" << std::endl;
                else
                    std::cout << "kekkeke" <<std::endl;
                // manager_dialogue(Socket);
            }
//
            listener_init((void *)Socket);

        }
        else if(str_response.compare(1,4,ERROR) == 0){
            std::cout << "UDP ERROR LOGIN" << std::endl;
            if(str_response.compare(5, 3, ERR_MANAGER_WRONG_PSWD) == 0)
                std::cout << "Wrong password for MANAGER" << std::endl;
            else if(str_response.compare(5, 3, ERR_MANAGER_ALREADY_EXISTS) == 0)
                std::cout << "MANAGER is already logged in" << std::endl;
            else if(str_response.compare(5, 3, ERR_USER_ALREADY_EXISTS) == 0)
                std::cout << "USER is already logged in" << std::endl;
            else
                std::cout << "Uknown error from server" << std::endl;
            countDG--;
        }
        else{
            std::cout << "Uknown error from server" << std::endl;
        }
    }
    else{
        std::cout << "Server wrong response " <<  response << std::endl;
        std::cout <<  "Response len = " << strlen(response) << std ::endl;
        std::cout <<  "RES = " << res<< std ::endl;

    }


    // shutdown(Socket, SHUT_RDWR);
    // close(Socket);
//    if (pthread_join(requestHandler, NULL) == 0)
//         std::cout << "Join is done" << std::endl;

    std::cout << "Bye!" << std::endl;
    return 0;
}
