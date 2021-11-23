#include <QCoreApplication>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <jsoncpp/json/json.h>

#define PORT 8080

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return 0;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        std::cout << "Invalid address or address not supported" << std::endl;
        return 0;
    }

    if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cout << "Connection failed!" << std::endl;
        return 0;
    }
    Json::Value obj;
    obj["command"] = "CONNECT";
    Json::StyledWriter styledWriter; std::string jsonString = styledWriter.write(obj);
    char json[1024] = {0};
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    memset(buffer, 1024, 0);
    recv(sock, buffer, sizeof(buffer), 0);
    if (strncmp(buffer, "{\n", 2)==0)
    {
        Json::Value data; Json::Reader read;
        std::string json(buffer);
        read.parse(json, data);
        if (data["status"] == "CONNECTACK") {
            std::cout << "Connected" << std::endl;
            obj["command"] = "SUBSCRIBE"; obj["topic"] = "/locationA/sensorA";
            jsonString = styledWriter.write(obj);
            char sendBuffer[1024] = {0};
            strcpy(sendBuffer, jsonString.c_str());
            send(sock, sendBuffer, sizeof(sendBuffer), 0);
            while(true) {
                memset(buffer, 1024, 0);
                recv(sock, buffer, sizeof(buffer), 0);
                if (strncmp(buffer, "{\n", 2)==0)
                {
                    read.parse(json, data);
                    std::cout << data["data"] << std::endl;
                }
            }
        }
    }
    else std::cout << "Not JSON\n";
}
