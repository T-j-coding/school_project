#ifndef RULE_ACCEPTER
#define RULE_ACCEPTER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>

#include "trie.h"
#include "string_matcher.h"

#define RULEPORT 9999

void* startlisten(void *arg);
void rule_process(char*);
#endif
