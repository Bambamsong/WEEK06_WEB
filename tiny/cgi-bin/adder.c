/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
    char *buf, *first, *second;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) { // cgiargs값이 buf에 들어감

	    first = strchr(buf, 'first');
        second = strchr(buf, 'second');
	    *first = '\0'; 
        *second = '\0';
	    strcpy(arg1, first+2);
	    strcpy(arg2, second+2);
	    n1 = atoi(arg1); // 처음으로 정수가 나오는곳에서부터 정수로 전환
  	    n2 = atoi(arg2);
    }
    /* Make the response body */
    // content 에 html body를 담는다
    sprintf(content, "QUERY_STRING=");
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", 
	    content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);
  
    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    // 이곳까지 헤더

    printf("%s", content);
    fflush(stdout);

    exit(0);
}
/* $end adder */







