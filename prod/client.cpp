//
// Created by Vovas on 07.12.2017.
//


#include "utils.h"

pthread_mutex_t stdout_mutex;
pthread_t broadcast_listener;

int id;

std::array<std :: string, COMMAND_NUM> command_list{{
                                                            "exit",
                                                            "rise",
                                                            "getlist",
                                                            "help - repeat the help list again"}
};

// std::array<std :: string, COMMAND_NUM> command_list_manager{{
//         "exit",
//         "additem",
//         "stop",
//         "getlist",
//         "help - repeat the help list again"}
// };



//TODO:create thread which one will hande if server is closed
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

void * listener_init(void * param){

    // int Socket = *((int *)param);
    long Socket = (long)param;
    do{
        char response[MAX_MESSAGE_SIZE];
        if(readn(Socket, response, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == MAX_MESSAGE_SIZE) {
            std::string response_str = response;
            // std::vector<std::string> item_vector = split(str_item_buf.substr(5, MAX_MESSAGE_SIZE), " ");
            if (response_str.compare(1, 4, ERROR) == 0){

                if(response_str.compare(5, 3, ERR_ITEM_PRICE_TOO_LOW) == 0){
                    std::cout << "Price  is to loow for this item" << std::endl;
                } else if(response_str.compare(5, 3, ERR_ITEM_WRONG_ID) == 0){
                    std::cout << "There is no item  with this id" << std::endl;
                } else if(response_str.compare(5, 3, ERR_ITEM_ALREADY_SOLD) == 0){
                    std::cout << "There is no such an item" << std::endl;
                } else {
                    std::cout << "Uknown error from server: " << response_str.substr(5,3)
                              << std::endl;
                }
            } else if(response_str.compare(1, 4, ACKNOWLEDGE) == 0){
                std::cout << "Your price accepted waiting for other users"<< std::endl;
            } else if(response_str.compare(1, 4, SENDLIST) == 0){
                std::cout << "listen handler" << std::endl;
                std::string size = response_str.substr(5, response_str.size());
                int size_of_list = atoi(size.c_str());
                std::cout << "size_of_list = " << size_of_list << std::endl;
                for(int i = 0; i < size_of_list; i++)
                {
                    char item_buf[MAX_MESSAGE_SIZE];
                    if(readn(Socket, item_buf, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == MAX_MESSAGE_SIZE) {
                        std::string str_item_buf = item_buf;
                        std::vector<std::string> item_vector = split(str_item_buf.substr(5, MAX_MESSAGE_SIZE), " ");
                        std::cout   << "msg # " << item_vector[0]
                                    << " id = " << item_vector[1]
                                    << " name =  " << item_vector[2]
                                    << " price = " << item_vector[3]
                                    << " holder =  " << item_vector[4]
                                    << std::endl;
                    }else{
                        std::cout << "Error while reading items from server" << std::endl;
                        break;
                    }
                }
            }else if(response_str.compare(1, 4, BROADCAST) == 0){
                std::vector<std::string> msg_vector = split(response_str.substr(5, MAX_MESSAGE_SIZE), " ");
                if(msg_vector[msg_vector.size() - 1] == START){
                    std::cout << "Manager is connected we are ready to start" << std::endl;
                } else if(msg_vector[msg_vector.size() - 1] == STOP){
                    std::cout << "Manager is disconnected we are finish" << std::endl;
                } else if(msg_vector[msg_vector.size() - 1] == APPROVE){
                    std::cout   << "Client " << msg_vector[0]
                                << " gets item " << msg_vector[2]
                                << " with price " << msg_vector[1]
                                << std::endl;
                } else {
                    std::cout   << "Client " << msg_vector[0]
                                << " rises price to " << msg_vector[1]
                                << " for item " << msg_vector[2]
                                << std::endl;

                }

            }else{
                std::cout << "Uknown response from server"<< std::endl;
            }
        }else{
            // std::cout << "Problems with connection" << std::endl;
            break;
        }
        // get list response

    }while(true);
    pthread_cancel(broadcast_listener);

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
            std::string message = create_message(USER, EXIT, std::to_string(id));
            if(send(Socket, message.c_str(), MAX_MESSAGE_SIZE, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE){
                std::cout << "error sending message" << std::endl;
                // return 1;
            }
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
            std::string message = create_message(USER, RISE, std::to_string(id) + " "
                                                             + std::to_string(item_id) + " " + std::to_string(new_price));

            if(send(Socket, message.c_str(), MAX_MESSAGE_SIZE, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE){
                std::cout << "error sending message" << std::endl;
                // return 1;
            }
            // char response[MAX_MESSAGE_SIZE];
            // if(readn(Socket, response, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == MAX_MESSAGE_SIZE) {
            //     std::string response_str = response;
            //     // std::vector<std::string> item_vector = split(str_item_buf.substr(5, MAX_MESSAGE_SIZE), " ");
            //     if (response_str.compare(1, 4, ERROR) == 0){

            //         if(response_str.compare(5, 3, ERR_ITEM_PRICE_TOO_LOW) == 0){
            //             std::cout << "Price " << new_price << " is to loow for this item" << std::endl;
            //         } else if(response_str.compare(5, 3, ERR_ITEM_WRONG_ID) == 0){
            //             std::cout << "There is no item with id  = " << item_id << std::endl;
            //         } else if(response_str.compare(5, 3, ERR_ITEM_ALREADY_SOLD) == 0){
            //             std::cout << "There is no such an item" << std::endl;
            //         } else {
            //             std::cout << "Uknown error from server: " << response_str.substr(5,3)
            //                       << std::endl; 
            //         }
            //     }
            //     else if(response_str.compare(1, 4, ACKNOWLEDGE) == 0){
            //         std::cout << "Your price accepted waiting for other users"<< std::endl; 
            //     }
            //     else{
            //         std::cout << "Uknown response from server"<< std::endl; 
            //     }
            // }


            // std::getline(std::cin, lot_number);
            // send request for rise with this number 
        }
        else if(request.compare("getlist") == 0){
            std::string message = create_message(USER, GETLIST, " ");
            std::cout << "Waiting for the server..." << std::endl;
            if(send(Socket, message.c_str(), MAX_MESSAGE_SIZE, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE){
                std::cout << "error sending message" << std::endl;
                // return 1;
            }
            // char response[MAX_MESSAGE_SIZE];
            // if(readn(Socket, response, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == MAX_MESSAGE_SIZE) {
            //     std::string str_response = response;
            //     std::string size = str_response.substr(5, str_response.size());
            //     int size_of_list = atoi(size.c_str());
            //     std::cout << "size_of_list = " << size_of_list << std::endl;
            //     for(int i = 0; i < size_of_list; i++)
            //     {
            //         char item_buf[MAX_MESSAGE_SIZE];
            //         if(readn(Socket, item_buf, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == MAX_MESSAGE_SIZE) {
            //             std::string str_item_buf = item_buf;
            //             std::vector<std::string> item_vector = split(str_item_buf.substr(5, MAX_MESSAGE_SIZE), " ");

            //             std::cout   << "msg # " << item_vector[0]
            //                         << " id = " << item_vector[1]
            //                         << " name =  " << item_vector[2]
            //                         << " price = " << item_vector[3]
            //                         << " holder =  " << item_vector[4]
            //                         << std::endl;
            //         }else{
            //             std::cout << "Error while reading items from server" << std::endl;
            //         }
            //     }
            // else{
            //     std::cout << "Error with message from server" <<std::endl;
            // }
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
            std::string message = create_message(MANAGER, EXIT,"");
            if(send(Socket, message.c_str(), MAX_MESSAGE_SIZE, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE){
                std::cout << "error sending message" << std::endl;
                // return 1;
            }
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
            // std::string data;
            // data = item_name;
            // data += " ";
            // data += item_price;
            std::string message = create_message(MANAGER, NEWITEM, item_name + " " + price_str);
            if(send(Socket, message.c_str(), MAX_MESSAGE_SIZE, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE){
                std::cout << "error sending message" << std::endl;
                // return 1;
            }
            // send request for rise with this number 
        } else if(request.compare("getlist") == 0){
            std::string message = create_message(MANAGER, GETLIST, " ");
            std::cout << "Waiting for the server..." << std::endl;
            if(send(Socket, message.c_str(), MAX_MESSAGE_SIZE, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE){
                std::cout << "error sending message" << std::endl;
                // return 1;
            }
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

    long Socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(DEFAULT_PORT);
    SockAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

//    Use this for Virtual Box
//    SockAddr.sin_addr.s_addr =  inet_addr("10.0.2.2");

    connect(Socket, (struct sockaddr *) (&SockAddr), sizeof(SockAddr));
    std::string s;
    long i;
    int tmp_size = 0;
    // logging in
    std::string login;
    std::string password;
    while(login.size() <= 0 && login.size() <= MAX_LOGIN_SIZE){
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
        message = create_message(MANAGER, LOGIN, login + del + password);
    } else{
        message = create_message(USER, LOGIN, login + del + password);
    }
    // std::cout << data << std::endl;
    // std::cout << message << std::endl;
    if(send(Socket, message.c_str(), MAX_MESSAGE_SIZE, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE){
        // if(send(Socket, message, strlen(message) + 1, MSG_NOSIGNAL) < MAX_MESSAGE_SIZE)
        std::cout << "error sending message" << std::endl;
        std::cout << "error = " << errno << std::endl;
        return 1;
    }
    // get message from the server
    char response[MAX_MESSAGE_SIZE];
    // std:: cout << "len of message = " << strlen(message) << std::endl;
    if(readn(Socket, response, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == MAX_MESSAGE_SIZE) {
        std::string str_response = response;
        if(str_response.compare(1,4,ACKNOWLEDGE) == 0){
            if (pthread_mutex_init(&stdout_mutex, NULL) != 0) {
                std::cout << "mutex init failed" << std::endl;
                pthread_exit(NULL);
            }
            // std::cout << "pererd tuta" << std::endl;              
            // std::cout << "tuta" << std::endl;              
            if(str_response.compare(5, 1, USER) == 0){
                // TODO: get user id  substr 
                std::cout << "str size = " << str_response.size() << std::endl;
                // size_t  l = str_response.size() - 6;
                std::string str_id= str_response.substr(6, str_response.size() - 6);//str_response.size() - 6);
                // int id = atoi(str_id.c_str());                
                id = atoi(str_id.c_str());
                std::cout << "You are connected as user with id = "<< id << std::endl;
                if(pthread_create(&broadcast_listener, NULL, user_dialogue, (void *) (Socket)) == 0)
                    std::cout << "Thread for listen was successfully created" << std::endl;
                else
                    std::cout << "kekkeke" <<std::endl;

                // pthread_create
            }else{

                std::cout << "You are connected as MANAGER" << std::endl;
                // TODO:
                if(pthread_create(&broadcast_listener, NULL, manager_dialogue, (void *) (Socket)) == 0)
                    std::cout << "Thread for listen was successfully created" << std::endl;
                else
                    std::cout << "kekkeke" <<std::endl;
                // manager_dialogue(Socket);
            }
            listener_init((void *)Socket);

        }
        else if(str_response.compare(1,4,ERROR) == 0){

            if(str_response.compare(5, 3, ERR_MANAGER_WRONG_PSWD) == 0)
                std::cout << "Wrong password for MANAGER" << std::endl;
            else if(str_response.compare(5, 3, ERR_MANAGER_ALREADY_EXISTS) == 0)
                std::cout << "MANAGER is already logged in" << std::endl;
            else if(str_response.compare(5, 3, ERR_USER_ALREADY_EXISTS) == 0)
                std::cout << "USER is already logged in" << std::endl;
            else
                std::cout << "Uknown error from server" << std::endl;
        }
        else{
            std::cout << "Uknown error from server" << std::endl;
        }
    }
    else{
        std::cout << "Server wrong response" <<  response << std::endl;
    }


    // shutdown(Socket, SHUT_RDWR);
    // close(Socket); 
    // if (pthread_join(broadcast_listener, NULL) == 0)
    //     std::cout << "Join is done" << std::endl;

    std::cout << "Bye!" << std::endl;
    return 0;
}
