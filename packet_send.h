//create by cyj

#ifndef PACKET_SEND_H
#define PACKET_SEND_H

#include <linux/if_packet.h>
#include <stdio.h>
#include <stdint.h>

#define PACKET_TYPE_LINK 1

struct packet_send_handler
{
	 int sd;
	 int type;
	 struct sockaddr_ll device;
};

int packet_send_init(struct packet_send_handler* psh,const char *dev,char *emsg);

int packet_send_link(struct packet_send_handler* psh,const uint8_t *packet,uint32_t len);

int packet_send_close(struct packet_send_handler* psh);

#endif