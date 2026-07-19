// stdafx.cpp — Server that keeps accepting new clients, all variables initialized at declaration
#include "stdafx.h"

short TCP_OnInit()
{
    short              sRetVal      = SUCCESS;
    int                isockfd      = -1;
    int                client_fd    = -1;
    int                n            = 0;
    WSADATA            wsaData      = { 0 };
    struct sockaddr_in sa_servaddr  = { 0 };
    struct sockaddr_in sa_clientaddr = { 0 };
    int                client_len   = sizeof(sa_clientaddr);
    char               buff[1024]   = { 0 };

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed.\n");
        return FAILURE;
    }

    isockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (isockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    printf("Socket successfully created..\n");

    sa_servaddr.sin_family      = AF_INET;
    sa_servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    sa_servaddr.sin_port        = htons(PORT);

    if (bind(isockfd, (SA*)&sa_servaddr, sizeof(sa_servaddr)) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    printf("Socket successfully binded..\n");

    // listen() is one-time setup — NOT inside the loop
    if (listen(isockfd, 5) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    printf("Server listening..\n");

    // OUTER LOOP: keep accepting new clients forever, even after one leaves.
    // This is what lets a client that closed and reopened reconnect.
    while (1)
    {
        client_len = sizeof(sa_clientaddr);   // reset each time — accept() overwrites it
        printf("Waiting for a client to connect...\n");
        client_fd = accept(isockfd, (SA*)&sa_clientaddr, &client_len);
        if (client_fd < 0)
        {
            printf("accept failed...\n");
            continue;   // the listening socket itself is still fine, just retry
        }
        printf("Client connected..\n");

        // INNER LOOP: chat with this one client until it disconnects
        while (1)
        {
            memset(buff, 0, sizeof(buff));
            n = recv(client_fd, buff, sizeof(buff) - 1, 0);
            if (n <= 0)
            {
                printf("Client disconnected.\n");
                break;   // fall back out to accept() for the next client
            }
            buff[n] = '\0';
            printf("\r\n Message from Client %s \t", buff);

            printf("\r\n Enter the Message : \t");
            memset(buff, 0, sizeof(buff));
            fgets(buff, sizeof(buff), stdin);
            buff[strcspn(buff, "\n")] = 0;
            send(client_fd, buff, (int)strlen(buff), 0);
        }

        closesocket(client_fd);
        // loop back to accept() — server is ready for the next connection
    }

    closesocket(isockfd);
    WSACleanup();
    return sRetVal;
}