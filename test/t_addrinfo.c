//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tu_inc.h"

//TODO...

#define ADDR 	"www.baidu.com"

void test_getaddr(int argc, char** argv)
{
	if( argc < 1 ){
		printf("need host\n");
		exit(1);
	}

	char* host = argv[0];
	printf("try get host:%s addr\n", host);
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s; 
	//int ret;
	//socklen_t peer_addr_len = sizeof(struct sockaddr);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* stream socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(host, NULL, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return;
	}

	//char host[NI_MAXHOST], service[NI_MAXSERV];
	char ip[INET6_ADDRSTRLEN];
	int i = 1;
	for (rp = result; rp != NULL; rp = rp->ai_next, i++) {
		//ret = getnameinfo(rp->ai_addr, peer_addr_len, host, NI_MAXHOST,
		//					service, NI_MAXSERV, NI_NUMERICSERV);

		//if( ret == 0 ){
			//printf("host=%s service=%s\n", host, service);
			printf("host=%s\n", inet_ntop(AF_INET, &((struct sockaddr_in*)rp->ai_addr)->sin_addr, ip, INET6_ADDRSTRLEN));
		//}
	}

	freeaddrinfo(result);
}

void test_local()
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* stream socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo("localhost", NULL, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return;
	}

	char ip[INET6_ADDRSTRLEN];
	int i = 1;
	for (rp = result; rp != NULL; rp = rp->ai_next, i++) {
		printf("host=%s\n", inet_ntop(AF_INET, &((struct sockaddr_in*)rp->ai_addr)->sin_addr, ip, INET6_ADDRSTRLEN));
	}

	freeaddrinfo(result);
}

/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/
