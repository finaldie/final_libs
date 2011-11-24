//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO...

void	test_strtok()
{
	const char src_str[] = "Content-Type: text/html; charset=utf-8";

	char* str, *tok, *save, *tok1;
	int i;
	for( i = 0, str = (char*)src_str; ; i++, str = NULL ){
		tok = strtok_r(str, " ", &save);
		if( !tok ) break;

		printf("tok=%s\n", tok);

		if( strcmp(tok, "Content-Type:") == 0 ){
			str = NULL;
			tok = strtok_r(str, " ", &save);
			printf("content=%s\n", tok);
			tok1 = strtok_r(tok, "=", &save);
			printf("content1=%s\n", tok1);
			tok = NULL;
			tok1 = strtok_r(tok, "=", &save);
			printf("content2=%s\n", tok1);
			printf("http charset=%s\n", tok1);
			break;
		}
	}

	printf("src:%s\n", src_str);
}

/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/
