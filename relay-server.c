#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifndef __BOOLEAN
#define __BOOLEAN

#define true 1
#define false 0

#endif
#define BUFSIZE 65536
#define CLIENT_LIMIT 2
int port = 0;

struct Client_Info
{
	int cli_sd[CLIENT_LIMIT];
	int cli_index;
};
struct Client_Info *cli_info;
void usage(void)
{
	printf("syntax : echo-server <port>\n");
	printf("sample : echo-client 1234\n");
}
void init(struct Client_Info *p)
{
	int i;
	p->cli_index = 0;
	for (i = 0; i < CLIENT_LIMIT; i++)
	{
		p->cli_sd[i] = 0;
	}
}
int parse(int argc, char *argv[])
{
	if (argc != 2)
		return false;

	port = atoi(argv[1]);
	return ((port != 0) ? true : false);
}

int main(int argc, char *argv[])
{
	if (parse(argc, argv) == false)
	{
		usage();
		exit(1);
	}
	struct Client_Info *cli_info = (struct Client_Info *)malloc(sizeof(struct Client_Info *));

	char buf[BUFSIZE];
	char *sendbuf = "READY: You are a sender.";
	char *recvbuf = "READY: You are a receiver.";
	int sendbuf_len = 0;
	int recvbuf_len = 0;
	for (int i = 0; i < BUFSIZE; i++)
	{
		sendbuf_len++;
		if (sendbuf[i] == '\0')
			break;
	}
	for (int i = 0; i < BUFSIZE; i++)
	{
		recvbuf_len++;
		if (recvbuf[i] == '\0')
			break;
	}
	while (true)
	{
		init(cli_info);
		int sd = socket(AF_INET, SOCK_STREAM, 0);
		if (sd == -1)
		{
			perror("socket");
			exit(1);
		}

		int optval = 1;
		int res1 = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		if (res1 == -1)
		{
			perror("setsockopt");
			exit(1);
		}

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(port);

		ssize_t res2 = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
		if (res2 == -1)
		{
			perror("bind");
			exit(1);
		}
		int res3 = listen(sd, 5);
		if (res3 == -1)
		{
			perror("listen");
			exit(1);
		}

		struct sockaddr_in send_addr;
		struct sockaddr_in recv_addr;
		socklen_t send_len = sizeof(send_addr);
		socklen_t recv_len = sizeof(recv_addr);
		cli_info->cli_sd[cli_info->cli_index++] = accept(sd, (struct sockaddr *)&send_addr, &send_len);
		cli_info->cli_sd[cli_info->cli_index++] = accept(sd, (struct sockaddr *)&recv_addr, &recv_len);

		close(sd);

		send(cli_info->cli_sd[0], sendbuf, sendbuf_len, 0);
		send(cli_info->cli_sd[1], recvbuf, recvbuf_len, 0);

		size_t res = -1;

		res = recv(cli_info->cli_sd[0], buf, BUFSIZE - 1, 0);
		if (res == -1 || res == 0)
		{
			perror("recv");
			while (res)
			{
				res = recv(cli_info->cli_sd[0], buf, BUFSIZE - 1, 0);
			}
		}
		else
		{
			close(cli_info->cli_sd[0]);
		}

		buf[res] = '\0';
		res = send(cli_info->cli_sd[1], buf, res, 0);
		if (res == -1 || res == 0)
		{
			perror("send");
			while (res)
			{
				res = send(cli_info->cli_sd[1], buf, res, 0);
			}
		}
		else
		{
			close(cli_info->cli_sd[1]);
		}
	}
	free(cli_info);
	return 0;
}
