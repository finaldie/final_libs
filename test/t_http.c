//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iconv.h>

#include "tu_inc.h"
#include "../base/libnet.h"
#include "../base/net_core.h"

//TODO...

#define BUFF_SIZE	( 1024 * 1024 * 512 )

typedef struct{
	char	request[200];
	char	response[BUFF_SIZE];
}hconn_t;

void	test_http()
{
	char* http_request = "GET /admin.php HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: Close\r\n\r\n";
	
	int fd = lnet_conn("192.168.60.105", 80, 1);
	if( fd == -1 ){
		printf("connect failed\n");
		exit(1);
	}

	int n = net_send_safe(fd, http_request, strlen(http_request));
	assert(n == strlen(http_request));

	char buf[BUFF_SIZE];
	n = net_recv(fd, buf, BUFF_SIZE);
	printf("http response:%s\n", buf);
}

void	test_host()
{
	char   *ptr, **pptr; 
	struct hostent *hptr; 
	char   str[32]; 
	ptr = "www.baidu.com";

	if((hptr = gethostbyname(ptr)) == NULL) {
		  printf("gethostbyname error for host:%s\n", ptr); 
		  return;
	}
						
	printf("official hostname:%s\n",hptr->h_name);
	for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
		printf(" alias:%s\n",*pptr);
						  
	switch(hptr->h_addrtype) {
		case AF_INET: 
		case AF_INET6: 
			pptr = hptr->h_addr_list; 
			for(; *pptr!=NULL; pptr++) 
				printf("address:%s\n", 
						inet_ntop(hptr->h_addrtype, *pptr, str,sizeof(str))); 
			printf(" first address: %s\n", 
					inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
			break;
		default:
			printf("unknown address type\n");
			break;
	}
}

void	test_host1(int argc, char** argv)
{
	if( argc != 2 ){
		printf("input host, count\n");
		return;
	}

	char* host = argv[0];
	int count = atoi(argv[1]);
	
	int i=0;
	for(; i<count; ++i){
		host_info_t hinfo;

		int st =net_get_host(host, &hinfo);
		if( st ){
			printf("host error st=%d\n", st);
			net_free_host(&hinfo);
			continue;
		}

		printf("alias_count=%d ip_count=%d\n", hinfo.alias_count, hinfo.ip_count);
		printf("offical name = %s\n", hinfo.official_name);
		int i = 0;
		for(; i< hinfo.alias_count; ++i){
			printf("alias name=%s\n", hinfo.alias_names[i]);
		}

		i = 0;
		for(; i< hinfo.ip_count; ++i){
			printf("ip addr=%s\n", hinfo.ips[i]);
		}

		net_free_host(&hinfo);
	}
}

static
int		is_line_end(const char pstr)
{
	return pstr == '\n' ? 1 : 0;
}

// analysis http header line : read until '\n'
static 
int		read_line(const char* pbuf, int start, int len, char* out)
{
	int n = start;
	if ( n > len )
		return 0;

	int i = 0;
	while( !is_line_end(pbuf[n]) && n <= len ){
		out[i++] = pbuf[n++];
	}

	return ++i;
}

static char* test_head = "HTTP1.1 200 OK\r\nGET XXX\r\nX-ASF:PHP\r\n\r\n<asdwerwxxxxx>";

static
int		is_http_head_end(const char* line, int len)
{
	if( len == 0 ) return 1;
	if( len != 2 ) return 0;

	if( line[0] == '\r' && line[1] == '\n' )
		return 1;
	return 0;
}

void	test_http_end()
{
	int s = 0;
	char line[100];
	int len = strlen(test_head) + 1;
	int n = 0;
	do{
		n = read_line(test_head, s, len - s, line);
		printf("read line n = %d, %s\n", n, line);
		s += n;
	}while( !is_http_head_end(line, n) );
}

//static
void	analysis_http_header(const char* pbuf, int len, int* status, char* charset)
{
	char http[50];
	int	s;
	char cset[50];
	char line[100];
	int start = 0;
	int n = read_line(pbuf, start, len, line);
	start += n;
	sscanf(line, "%s %d %s", http, &s, cset);
	printf("read http header(%d):%s %d %s\n", n, http, s, cset);
	*status = s;
	
	n = read_line(pbuf, start, len - start, line);
	start += n;
	printf("readline(len=%d|%d-%d)=%s\n", n, start, len - start, line);
	while( !is_http_head_end(line, n) ){
		char* str, *tok, *save, *tok1;
		int i;
		for( i = 0, str = line; ; i++, str = NULL ){
			tok = strtok_r(str, ":", &save);
			if( !tok ) break;

			printf("tok=%s\n", tok);

			if( strcmp(tok, "Content-Type") == 0 ){
				str = NULL;
				tok = strtok_r(str, ":", &save);
				tok1 = strtok_r(tok, "=", &save);
				tok = NULL;
				tok1 = strtok_r(tok, "=", &save);
				printf("http charset=%s\n", tok1);
				strcpy(charset, tok1);
				return;
			}
			
			break;
		}

		n = read_line(pbuf, start, len - start, line);
		start += n;
		printf("readline(len=%d)=%s\n", n, line);
	}
}

int 	IconV(const char* s, char* scode, char* dcode, char* cv)
{
	const char* src = s;
	const char* src_code = scode;
	const char* des_code = dcode;
	
	iconv_t cd = iconv_open(des_code, src_code); 
	if (cd == (iconv_t)(-1)) {
		printf("iconv openfailed\n");
		return 1; 
	}
	
	size_t src_len = strlen(src); 
	size_t des_len = src_len * 4 + 1;
	char* outbuf = (char*)malloc(des_len);
	char* out = outbuf; 
	size_t ret = iconv(cd, (char**)&src, &src_len, (char**)&out, &des_len); 
	if(ret == (size_t)(-1)) {
		printf("iconv failed:%d-%s\n", errno, strerror(errno));
		return 2;
	}
	else{
		printf("iconv sucessful ret=%lu\n", ret);
		memcpy(cv, outbuf, des_len);
	}

	free(outbuf); 
	iconv_close(cd);
	return 0;
}

#define TMP_SIZE 32768
void	test_host2(int argc, char** argv)
{
	printf("argc = %d\n", argc);
	if( argc != 2 ){
		printf("input host name & URI\n");
		return;
	}

	host_info_t hinfo;
	char* host = argv[0];
	char* uri = argv[1];

	net_get_host(host, &hinfo);
	printf("alias_count=%d ip_count=%d\n", hinfo.alias_count, hinfo.ip_count);
	printf("offical name = %s\n", hinfo.official_name);
	int i = 0;
	for(; i< hinfo.alias_count; ++i){
		printf("alias name=%s\n", hinfo.alias_names[i]);
	}

	i = 0;
	for(; i< hinfo.ip_count; ++i){
		printf("ip addr=%s\n", hinfo.ips[i]);
	}

	char* http_request_fmt = "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n";
	char http_request[200];
	snprintf(http_request, 200, http_request_fmt, uri, host);
	printf("http_request=%s\n", http_request);
	
	int fd = lnet_conn(hinfo.ips[0], 80, 1);
	if( fd == -1 ){
		printf("connect failed\n");
		exit(1);
	}

	int n = net_send_safe(fd, http_request, strlen(http_request));
	assert(n == strlen(http_request));

	char buf[TMP_SIZE];
	char res[BUFF_SIZE];
	int total = 0;
	memset(res, 0, BUFF_SIZE);

	do{
		n = net_recv(fd, buf, TMP_SIZE);
		if( n == 0 ){
			printf("recv complete\n");
			break;
		}
		else if( n < 0 ){
			printf("recv error: %d : %s\n", errno, strerror(errno));
			break;
		}

		memcpy(res + total, buf, n);
		total += n;
	}while(1);
	printf("http response:\n%s\n\n", res);

	int code;
	char charset[10];
	analysis_http_header(res, total, &code, charset);
	printf("http code=%d charset=%s\n", code, charset);

	char* cv = (char*)malloc(strlen(res) * 4 + 1);

	if( IconV(res, "utf-8", "gbk", cv) ){
		printf("iconv error\n");
		exit(1);
	}

	printf("cv:\n%s\n", cv);

	net_free_host(&hinfo);
	close(fd);
}

void	proc_recv(int fd, hconn_t* arg)
{
	char* request = arg->request;
	int n = net_send_safe(fd, request, strlen(request));
	assert(n == strlen(request));

	char buf[TMP_SIZE];
	int total = 0;
	memset(arg->response, 0, BUFF_SIZE);

L:
	do{
		n = net_recv(fd, buf, TMP_SIZE);
		if( n == 0 ){
			printf("recv complete\n");
			break;
		}
		else if( n < 0 ){
			printf("recv error: %d : %s\n", errno, strerror(errno));
			goto L;
		}

		memcpy(arg->response + total, buf, n);
		total += n;
	}while(1);
	printf("http response:\n%s\n\n", arg->response);

	int code;
	char charset[10];
	analysis_http_header(arg->response, total, &code, charset);
	printf("http code=%d charset=%s\n", code, charset);

	/*
	char* cv = (char*)malloc(strlen(res) * 4 + 1);

	if( IconV(res, charset, "gbk", cv) ){
		printf("iconv error\n");
		exit(1);
	}

	printf("cv:\n%s\n", cv);
	*/
	
	free(arg);
	close(fd);
}

void	conn_proc(int fd, int ev, conn_arg_t arg)
{
	printf("conn proc: fd=%d ev=%d\n", fd, ev);
	if( ev == LNET_CONN_SUCESS ){
		hconn_t* conn_arg = (hconn_t*)arg.ptr;
		proc_recv(fd, conn_arg);
	}
	else{
		printf("connect error or timeout\n");
		free(arg.ptr);
	}
}


void	test_host3(int argc, char** argv)
{
	if( argc != 2 ){
		printf("input host name & URI\n");
		return;
	}
	lnet_init();

	host_info_t hinfo;
	char* host = argv[0];
	char* uri = argv[1];

	net_get_host(host, &hinfo);
	printf("alias_count=%d ip_count=%d\n", hinfo.alias_count, hinfo.ip_count);
	printf("offical name = %s\n", hinfo.official_name);
	int i = 0;
	for(; i< hinfo.alias_count; ++i){
		printf("alias name=%s\n", hinfo.alias_names[i]);
	}

	i = 0;
	for(; i< hinfo.ip_count; ++i){
		printf("ip addr=%s\n", hinfo.ips[i]);
	}

	char* http_request_fmt = "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n";
	hconn_t* conn_arg = (hconn_t*)malloc(sizeof(hconn_t));
	snprintf(conn_arg->request, 200, http_request_fmt, uri, host);
	printf("http_request=%s\n", conn_arg->request);

	conn_arg_t arg;
	arg.ptr = conn_arg;
	int fd = lnet_conn_a(hinfo.ips[0], 80, 1, 0, conn_proc, arg);
	if( fd == -1 ){
		printf("conn failed\n");
		exit(1);
	}
	else if( fd > 0 ){
		proc_recv(fd, conn_arg);
	}
	else{
		printf("waitint for connecting...\n");
	}

	net_free_host(&hinfo);
	lnet_dispatch();
}

/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/
