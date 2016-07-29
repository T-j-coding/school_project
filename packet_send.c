#include "packet_send.h"

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include <net/ethernet.h>

int packet_send_init(struct packet_send_handler* psh,const char *dev,char *emsg)
{
	  memset(&psh->device, 0x00, sizeof(psh->device));
  	if ((psh->device.sll_ifindex = if_nametoindex (dev)) == 0) {
    	sprintf (emsg,"%s","if_nametoindex() failed to obtain interface index ");
    	return -1;
  	}
  	// Submit request for a socket descriptor to look up interface.
  	int tmp_sd;
  	if ((tmp_sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
    	sprintf (emsg,"%s","socket() failed to get socket descriptor for using ioctl() ");
    	return -1;
  	}

  	// Use ioctl() to look up interface name and get its MAC address.
  	struct ifreq ifr;
  	memset (&ifr, 0, sizeof (ifr));
  	snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", dev);
  	if (ioctl (tmp_sd, SIOCGIFHWADDR, &ifr) < 0) {
    	sprintf (emsg,"%s","ioctl() failed to get source MAC address ");
    	return -1;
 	  }
 	  close (tmp_sd);

  	// Copy source MAC address.
  	psh->device.sll_family = AF_PACKET; 
  	memcpy (psh->device.sll_addr, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
  	psh->device.sll_halen = htons (6);

  	if ((psh->sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
    	sprintf (emsg,"%s","socket() failed ");
    	return -1;
  	}
}

int packet_send_link(struct packet_send_handler* psh,const uint8_t *packet,uint32_t len)
{
	int bytes;
        struct ether_header *ehdr=(struct ether_header*)packet;
	uint8_t tmp_mac[ETH_ALEN];
	memcpy(tmp_mac,ehdr->ether_shost,ETH_ALEN);
	memcpy(ehdr->ether_shost,ehdr->ether_dhost,ETH_ALEN);
	memcpy(ehdr->ether_dhost,tmp_mac,ETH_ALEN);
	if((bytes = sendto (psh->sd, packet, len, 0, (struct sockaddr *) &psh->device, sizeof (psh->device)))<0 || bytes < len)
	{
		printf("send: %d\n",bytes);
		return -1;
	}
        //printf("send: %d\n", bytes);
	return bytes;
}

int packet_send_close(struct packet_send_handler* psh)
{
	if(close(psh->sd)==0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}
