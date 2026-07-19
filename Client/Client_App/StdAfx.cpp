// stdafx.cpp — Client with automatic reconnect, all variables initialized at declaration
#include "stdafx.h"

short TCP_OnInit()
{
    short              sRetVal      = SUCCESS;
    int                isockfd      = -1;
    int                n            = 0;
    WSADATA            wsaData      = { 0 };
    struct sockaddr_in sa_servaddr  = { 0 };
    char               buff[1024]   = { 0 };

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed.\n");
        return FAILURE;
    }

    sa_servaddr.sin_family      = AF_INET;
    sa_servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    sa_servaddr.sin_port        = htons(PORT);

    // OUTER LOOP: connection lifecycle — keep (re)connecting forever
    while (1)
    {
        // A fresh socket is required for every connection attempt —
        // you cannot connect() again on a socket that already failed
        // or was closed.
        isockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (isockfd == -1)
        {
            printf("socket creation failed...\n");
            Sleep(2000);
            continue;
        }

        printf("Connecting to server...\n");
        if (connect(isockfd, (SA*)&sa_servaddr, sizeof(sa_servaddr)) != 0)
        {
            printf("connection failed, retrying in 2s...\n");
            closesocket(isockfd);
            Sleep(2000);
            continue;   // back to top of outer loop, try again with a new socket
        }

        printf("connected to the server..\n");

        // INNER LOOP: message exchange — runs while THIS connection is alive
        while (1)
        {
            printf("\r\n Enter the Message : \t");
            memset(buff, 0, sizeof(buff));
            fgets(buff, sizeof(buff), stdin);
            buff[strcspn(buff, "\n")] = 0;

            if (send(isockfd, buff, (int)strlen(buff), 0) == SOCKET_ERROR)
            {
                printf("\r\n Lost connection while sending.\n");
                break;   // drop out to reconnect
            }

            memset(buff, 0, sizeof(buff));
            n = recv(isockfd, buff, sizeof(buff) - 1, 0);
            if (n <= 0)
            {
                printf("\r\n Server disconnected.\n");
                break;   // drop out to reconnect
            }
            buff[n] = '\0';
            printf("\r\n Message from Server: %s \t", buff);
        }

        // Connection is dead — close it and loop back to the top to reconnect
        closesocket(isockfd);
        printf("Attempting to reconnect...\n");
        Sleep(1000);
    }

    // unreachable while the outer loop runs forever, kept for completeness
    WSACleanup();
    return sRetVal;
}