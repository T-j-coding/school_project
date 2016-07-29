#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/time.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include <pcap.h>
//#include "nids.h"

#include "hash.h"
#include "util.h"
#include "packet_send.h"
#include "get_data.h"

extern void process_gzip_data(struct data_head*);
//char *dev_up, *dev_down;//read dev from config file
pcap_t *pt_a, *pt_up_stream;

pthread_t pid_up_stream;

struct packet_send_handler net_down,net_up,g_net;
/*
char buff[100][2000];
int  seg_num=0,seg_lens[50];
*/
uint8_t up_router_mac[ETH_ALEN]= {0xff,0xff,0xff,0xff,0xff,0xce};
uint8_t down_router_mac[ETH_ALEN];
uint8_t my_mac[ETH_ALEN];
uint8_t up_mac[ETH_ALEN];
uint8_t down_mac[ETH_ALEN];
uint8_t mac_broadcast[]= {0xff,0xff,0xff,0xff,0xff,0xff};

char errbuf[256];
char dev_up[10], dev_down[10],g_dev[10];

struct data_head data_cache_table[DATA_CACHE_TABLE_SIZE];

int devname_to_mac(char devname[],uint8_t mac[])
{
    int tmp_sd;
    if ((tmp_sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
        printf ("%s","socket() failed to get socket descriptor for using ioctl() ");
        return -1;
    }
    // Use ioctl() to look up interface name and get its MAC address.
    struct ifreq ifr;
    memset (&ifr, 0, sizeof (ifr));
    snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", devname);
    if (ioctl (tmp_sd, SIOCGIFHWADDR, &ifr) < 0) {
        printf ("%s","ioctl() failed to get source MAC address ");
        close(tmp_sd);
        return -1;
    }
    close (tmp_sd);
    memcpy (mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
    return 0;
}

int str_to_mac(char str[], uint8_t mac[])
{
    int i,j;
    int len=strlen(str);
    if(len!=17) return -1;
    for(i=0,j=0; i<6; ++i,j+=3)
    {
        mac[i]=0;
        if(str[j]-'0'>=0&&str[j]-'0'<=9)
        {
            mac[i]+=(str[j]-'0')*16;
        }
        else
        {
            mac[i]+=(str[j]-'a'+10)*16;
        }
        if(str[j+1]-'0'>=0&&str[j+1]-'0'<=9)
        {
            mac[i]+=str[j+1]-'0';
        }
        else
        {
            mac[i]+=str[j+1]-'a'+10;
        }
    }
    return 0;
}

int read_config()
{
    //dev_up = "eth1";
    //dev_down = "eth0";
    FILE *fd=fopen("config.txt", "r");
    if(fd==NULL)
    {
        printf("open config file error!\n");
        return -1;
    }
    char bf[20];
    int rval;
    while(fscanf(fd, "%s",bf)!=EOF)
    {
        if(strcmp(bf, "dev_up")==0)
        {
            if(fscanf(fd, "%s",dev_up)==EOF)
            {
                printf("dev_up read error!\n");
                goto error;
            }
            if(devname_to_mac(dev_up, up_mac)==-1)
            {
                printf("dev_up name error!\n");
                goto error;
            }
        }
        else if(strcmp(bf, "dev_down")==0)
        {
            if(fscanf(fd, "%s",dev_down)==EOF)
            {
                printf("dev_down read error!\n");
                goto error;
            }
            if(devname_to_mac(dev_down, down_mac)==-1)
            {
                printf("dev_down name error!\n");
                goto error;
            }
        }
        else if(strcmp(bf, "up_router_mac")==0)
        {
            //printf("up_router\n");
            char tmp[20];
            if(fscanf(fd, "%s",tmp)==EOF)
            {
                printf("up_router_mac read error!\n");
                goto error;
            }
            //printf("%s\n",tmp);
            if(str_to_mac(tmp, nids_params.up_router_mac)==-1)
            {
                printf("up_router_mac read error!\n");
                goto error;
            }
            //int it=0;
            //for(it=0;it<6;++it) printf("%x",up_router_mac[it]);
        }
        else if(strcmp(bf, "down_router_mac")==0)
        {
            char tmp[20];
            if(fscanf(fd, "%s",tmp)==EOF)
            {
                printf("down_router_mac read error!\n");
                goto error;
            }
            if(str_to_mac(tmp, nids_params.down_router_mac)==-1)
            {
                printf("down_router_mac read error!\n");
                goto error;
            }
        }
        else if(strcmp(bf, "my_mac")==0)
        {
            char tmp[20];
            if(fscanf(fd, "%s",tmp)==EOF)
            {
                printf("my_mac read error!\n");
                goto error;
            }
            if(str_to_mac(tmp, my_mac)==-1)
            {
                printf("my_mac read error!\n");
                goto error;
            }
            memcpy(nids_params.my_mac,my_mac,ETH_ALEN);
        }
        else if(strcmp(bf, "tcp_stream_table_size")==0)
        {
            int rval;
            if((rval=fscanf(fd, "%d",&nids_params.n_tcp_streams))==EOF||rval==0)
            {
                printf("tcp_stream_table_size read error!\n");
                goto error;
            }
        }
        else if(strcmp(bf, "packet_send_time_out")==0)
        {
            int rval;
            if((rval=fscanf(fd, "%d",&nids_params.send_time_out))==EOF||rval==0)
            {
                printf("packet_send_time_out read error!\n");
                goto error;
            }
        }
        else if(strcmp(bf, "packet_queue_limit")==0)
        {
            int rval;
            if((rval=fscanf(fd, "%d",&nids_params.queue_limit))==EOF||rval==0)
            {
                printf("packet_queue_limit read error!\n");
                goto error;
            }
        }
        else if(strcmp(bf, "capture_time_out")==0)
        {
            int rval;
            if((rval=fscanf(fd, "%d",&nids_params.pcap_timeout))==EOF||rval==0)
            {
                printf("capture_time_out read error!\n");
                goto error;
            }
        }
        else if(strcmp(bf, "ip_table_size")==0)
        {
            int rval;
            if((rval=fscanf(fd, "%d",&nids_params.n_hosts))==EOF||rval==0)
            {
                printf("ip_table_size read error!\n");
                goto error;
            }
        }
        else if(strcmp(bf, "gk_method")==0)
        {
            if(fscanf(fd, "%d", &gk_method)==EOF)
            {
                printf("gk_method read error!\n");
                goto error;
            }
            switch (gk_method)
            {
            case 5:
                printf("work at monitoring mode\n");
                break;
            case 2:
                printf("stream block method: reset\n");
                break;
            case 4:
                printf("stream block method: drop\n");
                break;
            default:
                printf("stream block method error\n");
                goto error;
                break;
            }
        }
        else
        {
            printf("read config file error!\n");
            goto error;
        }
    }
    fclose(fd);
    return 0;
error:
    fclose(fd);
    return -1;
}

int mac_equal(uint8_t packet_mac[],uint8_t mac[])
{
    /*
    int i;
    for(i=0;i<ETH_ALEN;++i)
    {
      if(packet_mac[i]!=mac[i])
      {
        return 1;
      }
    }
    return 0;
    */
    return packet_mac[ETH_ALEN-1]==mac[ETH_ALEN-1];
}

int mac_filter(const u_char *packet, int dev)
{
    int ret=0;
    struct ether_header *ehdr=(struct ether_header*)packet;
    switch(dev)
    {
    case 0:
        if(mac_equal(ehdr->ether_shost, up_router_mac))
        {
            ret=0;
            break;
        }
        ret=mac_equal(ehdr->ether_dhost, up_router_mac)
            +mac_equal(ehdr->ether_dhost,mac_broadcast);
        break;
    case 1:
        ret=mac_equal(ehdr->ether_shost, up_router_mac);
        break;
    default:
        break;
    }
    return ret;
}

int g_filter(const u_char *packet, int dev){
	struct ether_header *ehdr=(struct ether_header*)packet;
	//if(packet[13]==0)
	//	printf("%d %d\n",packet[12],packet[13]);
	if (packet[12] != 0x88 || packet[13] != 0xa8){
		return 0;
	}
 	if(mac_equal(ehdr->ether_dhost, my_mac))
        {
        	return 1;
        }
	//printf("not equal\n");
	return 0;
}

void up_stream_handler( u_char *devId, const struct pcap_pkthdr *hdr, const u_char *packet )
{
	    struct ether_header *ehdr=(struct ether_header*)packet;

    //if( ehdr->ether_dhost[ETH_ALEN-1] == 0xce ){
        printf("TO dst: %X:%X:%X:%X:%X:%X, src: %X:%X:%X:%X:%X:%X\n", ehdr->ether_dhost[0],ehdr->ether_dhost[1],
        ehdr->ether_dhost[2],ehdr->ether_dhost[3],ehdr->ether_dhost[4],
		ehdr->ether_dhost[ETH_ALEN-1], ehdr->ether_shost[0],ehdr->ether_shost[1],
        ehdr->ether_shost[2],ehdr->ether_shost[3],ehdr->ether_shost[4],ehdr->ether_shost[ETH_ALEN-1]);
        return;                                    
    //}
    if( ehdr->ether_shost[ETH_ALEN-1] == 0xce )
    {
        //printf("B  src: %X:%X:%X:%X:%X:%X, dst:%X\n", ehdr.ether_shost[0],ehdr.ether_shost[1],
        //ehdr.ether_shost[2],ehdr.ether_shost[3],ehdr.ether_shost[4],
        //ehdr.ether_shost[ETH_ALEN-1], ehdr.ether_dhost[ETH_ALEN-1] );
        int c;
        //while((c = libnet_write_link( net_b, (u_char*)packet, hdr->caplen ))==-1);
        c = packet_send_link( &net_up, (u_char*)packet, hdr->caplen );
        //printf("B write: %d %d\n", c,hdr->caplen );
    }
}

void* up_stream_capture(void *arg)
{
    //dev_b = pcap_lookupdev( errbuf );
    u_char dev_id=1;
    pt_up_stream = pcap_open_live( dev_up, 65535, 1, 1, errbuf );
    if( pt_up_stream == NULL )
    {
        printf("pcap_open_live:%s ", errbuf );
        exit(0);
    }

    pcap_loop( pt_up_stream, -1,/*pcap_handler_interface*/nids_pcap_handler/*up_stream_handler*/, &dev_id);
}

int create_up_stream_capture_thread()
{
    int status;
    status = pthread_create( &pid_up_stream, NULL, up_stream_capture, NULL );
    if ( status != 0 )
    {
        printf( "pthread_create( up_stream_capture ) faile.\n");
        //TODO:log
        goto end;
    }
    //printf("before pthread join\n");
    //pthread_join( pid_up_stream, NULL );
    //printf("after pthread join\n");
    return 0;
end:
    return -1;
}

void ip_frag_handler(struct ip * a_packet, int len)
{
    ;//printf("i am called!\n");
}

//forbid checksum data send by this host
void chksum_ctl_host()
{
    static struct nids_chksum_ctl ctl;

    ctl.netaddr = inet_addr("192.168.13.64");
    ctl.mask = inet_addr("255.255.255.224");
    ctl.action = NIDS_DONT_CHKSUM;
    nids_register_chksum_ctl(&ctl, 1);
}

//forbid all checksum
void forbid_chksum()
{
    static struct nids_chksum_ctl ctl2;

    ctl2.netaddr = 0;
    ctl2.mask = 0;
    ctl2.action = NIDS_DONT_CHKSUM;
    nids_register_chksum_ctl(&ctl2, 1);
}

void packet_send(const uint8_t *packet,uint32_t len,uint32_t dev)
{
    int ret=-1;
    switch(dev)
    {
    case 0:
        ret=packet_send_link(&net_down, packet, len);
        break;
    case 1:
        ret=packet_send_link(&net_up, packet, len);
        break;
    default:
        break;
    }
    if(ret==-1)
    {
        printf("error send packet: ret=-1\n");
        //TODO:log
    }
}

void g_packet_send(const uint8_t *packet,uint32_t len,uint32_t dev)
{
    int ret=-1;
    ret=packet_send_link(&g_net, packet, len);
    if(ret==-1)
    {
        printf("error send packet: ret=-1\n");
        //TODO:log
    }
}
u_int mk_hash_index(struct tuple4 *addr)
{
  u_int hash1=mkhash(addr->saddr, addr->source, addr->daddr, addr->dest);
  u_int hash2=mkhash(addr->daddr, addr->dest, addr->saddr, addr->source);
  return ((hash1 % tcp_stream_table_size)^(hash2 % tcp_stream_table_size))%tcp_stream_table_size;
}
/*
int mk_hash_index(struct tuple4* addr)
{
    uint32_t hash=mkhash(addr->saddr, addr->source, addr->daddr, addr->dest);
    return hash % DATA_CACHE_TABLE_SIZE;
    //return 0;
}
*/
int tuple4_equal(struct tuple4* t1,struct tuple4* t2)
{
    /*
    return (t1->saddr==t2->saddr &&
            t1->daddr==t2->daddr &&
            t1->source==t2->source &&
            t1->dest==t2->dest);
    */
    return !memcmp(t1,t2,sizeof(struct tuple4));
}

//return the exact tcp_stream or the last tcp_stream of the linkedlist
struct data_head* find_stream(struct tcp_stream *a_tcp)
{
    u_int hash_index;
    hash_index=mk_hash_index(&a_tcp->addr);
    struct data_head* p_dh=&data_cache_table[hash_index];
    struct data_head* p_null=NULL;
    while(p_dh->next!=NULL)
    {
        if(p_dh->a_tcp==a_tcp)
        {
            break;
        }
        else if(p_dh->a_tcp==NULL)
        {
            p_null=p_dh;
        }
        p_dh=p_dh->next;
    }
    if(p_dh->a_tcp!=a_tcp&&p_null!=NULL)
    {
        p_dh=p_null;
    }
    return p_dh;
}

void insert_new_data_segment(struct tcp_stream *a_tcp, struct data_segment* ds,int from_client)
{
    //return;
    struct data_head* p_dh=find_stream(a_tcp);
    //return;
    if(p_dh==NULL)
    {
        printf("find stream error!\n");
    }
    else if(p_dh->a_tcp==a_tcp)
    {
        if(from_client)
        {
            if(p_dh->last_up==NULL)
            {
                p_dh->first_up=ds;
                p_dh->last_up=ds;
            }
            else
            {
                p_dh->last_up->next=ds;
                p_dh->last_up=ds;
            }
        }
        else
        {
            if(p_dh->last_down==NULL)
            {
                p_dh->first_down=ds;
                p_dh->last_down=ds;
            }
            else
            {
                p_dh->last_down->next=ds;
                p_dh->last_down=ds;
            }
            process_gzip_data(p_dh);
        }
    }
    else if(p_dh->a_tcp==NULL)
    {
	//p_dh->temp_up = p_dh->first_up;
	//p_dh->temp_up = NULL;
	if(p_dh->dist_buf == NULL)
	    p_dh->dist_buf = (unsigned short*) malloc(0x8000*sizeof(unsigned short));
	if(p_dh->litter_buf == NULL)
	    p_dh->litter_buf = (unsigned char*) malloc(0x8000*sizeof(unsigned char));
	p_dh->dist = 0;
	p_dh->lit = 0;
	p_dh->current_block = NULL;
	p_dh->temp_up = NULL;
	p_dh->offset_up = 0;

        p_dh->a_tcp=a_tcp;
        if(from_client)
        {
            p_dh->first_up=ds;
            p_dh->last_up=ds;
        }
        else
        {
            p_dh->first_down=ds;
            p_dh->last_down=ds;
        }
	
	if(!from_client) {
	   process_gzip_data(p_dh);
	}
    }
    else
    {
        struct data_head* p_dh_new=mknew(struct data_head);
        memset(p_dh_new,0x00,sizeof(struct data_head));
        if(p_dh_new==NULL)
        {
            printf("mem alloc error\n");
            return;
        }
        pthread_mutex_init(&p_dh_new->tcp_stream_lock_up,NULL);
	pthread_mutex_init(&p_dh_new->tcp_stream_lock_down,NULL);
        if(from_client)
        {
            p_dh_new->last_up=ds;
            p_dh_new->first_up=ds;
        }
        else
        {
            p_dh_new->first_down=ds;
            p_dh_new->last_down=ds;
        }
        p_dh_new->a_tcp=a_tcp;
        p_dh->next=p_dh_new;
        //printf("cccccccccccccccccccccccccccccc\n");
        p_dh->dist_buf = (unsigned short*)malloc(0x8000*sizeof(unsigned short));
	p_dh->litter_buf = (unsigned char*)malloc(0x8000*sizeof(unsigned char));
	if(!from_client) {
	   process_gzip_data(p_dh);
	}
    }

    //TODO:notify
}

void free_data_segment(struct data_segment* ds)
{
    free(ds->data);
    free(ds);
}

void free_data_stream(struct tcp_stream *a_tcp)
{
    
    struct data_head* p_dh=find_stream(a_tcp);
    //p_dh->a_tcp=NULL;
    if(p_dh->a_tcp==a_tcp)
    {  
        //printf("before lock...\n");
        pthread_mutex_lock(&p_dh->tcp_stream_lock_up);
	pthread_mutex_lock(&p_dh->tcp_stream_lock_down);
	/*if(pthread_mutex_trylock(&p_dh->tcp_stream_lock_down) != 0){
		printf("\n\n******************************************************************************************\n\n");
		pthread_mutex_unlock(&p_dh->tcp_stream_lock_down);
		pthread_mutex_lock(&p_dh->tcp_stream_lock_down);
		}*/
        //printf("after lock...\n");
        if(a_tcp->user!=NULL)
        {
            a_tcp->user=NULL;
        }
        p_dh->a_tcp=NULL;
        pthread_mutex_unlock(&p_dh->tcp_stream_lock_down);
	pthread_mutex_unlock(&p_dh->tcp_stream_lock_up);
	//printf("unlock\n");
        struct data_segment* p_ds_tmp=p_dh->first_up,*tmp;
        p_dh->last_up=NULL;
        p_dh->last_down=NULL;
        p_dh->first_up=NULL;
        p_dh->offset_down=0;
        p_dh->offset_up=0;
        while(p_ds_tmp!=NULL)
        {
            tmp=p_ds_tmp;
            p_ds_tmp=p_ds_tmp->next;
            free_data_segment(tmp);
        }
        p_ds_tmp=p_dh->first_down;
        p_dh->first_down=NULL;
        while(p_ds_tmp!=NULL)
        {
            tmp=p_ds_tmp;
            p_ds_tmp=p_ds_tmp->next;
            free_data_segment(tmp);
        }
    }
    //TODO:log some error may happen
    
}

int cnt=0;
void tcp_stream_handler (struct tcp_stream *a_tcp, void ** this_time_not_needed)
{
    //sunpy
    //return;

    if (a_tcp->nids_state == NIDS_JUST_EST)
    {
        //printf("connection established:%d %d\n", a_tcp->addr.saddr,a_tcp->addr.daddr);
#ifdef WE_WANT_DATA_RECEIVED_BY_A_CLIENT
        a_tcp->client.collect++;
#endif
#ifdef WE_WANT_DATA_RECEIVED_BY_A_SERVER
        a_tcp->server.collect++;
#endif
#ifdef WE_WANT_URGENT_DATA_RECEIVED_BY_A_SERVER
        a_tcp->server.collect_urg++;
#endif
#ifdef WE_WANT_URGENT_DATA_RECEIVED_BY_A_CLIENT
        a_tcp->client.collect_urg++;
#endif
	u_int hash_index;
	hash_index=mk_hash_index(&a_tcp->addr);
	struct data_head* p_dh=&data_cache_table[hash_index];
        do{
		if(p_dh->a_tcp!=NULL && tuple4_equal(&p_dh->a_tcp->addr, &a_tcp->addr))
		{
			free_data_stream(a_tcp);
			break;
		}
		p_dh=p_dh->next;
	}while(p_dh!=NULL);
	return;
    }//else{return;}
    if (a_tcp->nids_state == NIDS_TIMED_OUT)
    {
        //TODO:maybe log
        free_data_stream(a_tcp);
	printf("NIDS_TIMED_OUT\n");
        return;
    }
    if (a_tcp->nids_state == NIDS_CLOSE)
    {
        //tcp connection closed
        //TODO:maybe log
        free_data_stream(a_tcp);
        //printf("connection closed: %d %d\n",a_tcp->addr.saddr,a_tcp->addr.daddr);
        return;
    }
    if (a_tcp->nids_state == NIDS_RESET)
    {
        //tcp connection reseted
        //TODO:maybe log
        //printf("connection reset: %d %d\n",a_tcp->addr.saddr,a_tcp->addr.daddr);
        free_data_stream(a_tcp);
        return;
    }
    //something
    if (a_tcp->nids_state == NIDS_BLOCKED)
    {
	free_data_stream(a_tcp);
	return;
    }
    if (a_tcp->nids_state == NIDS_DATA)
    {
        // new data has arrived; gotta determine in what direction
        // and if it's urgent or not
	//printf("get data!\n");
        struct half_stream *hlf;

        if (a_tcp->server.count_new_urg)
        {
            // new byte of urgent data has arrived
            // TODO:log to check if urgent data is used
            printf("server:new byte of urgent data has arrived\n");
            return;
        }
        if(a_tcp->client.count_new_urg) {
            // new byte of urgent data has arrived
            // TODO:log to check if urgent data is used
            printf("client:new byte of urgent data has arrived\n");
            return;
        }
        int from_client=1;
        if (a_tcp->client.count_new)
        {
            hlf = &a_tcp->client;
            from_client=0;
        }
        else
        {
            hlf = &a_tcp->server;
        }

        struct data_segment* ds_new=mknew(struct data_segment);
        if(ds_new)
        {
            ds_new->len=hlf->count_new;
            ds_new->next=NULL;
            ds_new->data=(char*)malloc(ds_new->len*sizeof(char));
            if(ds_new->data)
            {
                memcpy(ds_new->data, hlf->data, ds_new->len);
/*
                if(from_client){
		    int i=0;
		    for(i=0;i<ds_new->len;++i){
                        printf("%c",hlf->data[i]);
                    }
                    printf("\n");
                }
*/
                //printf("le: %d\n",ds_new->len);
                insert_new_data_segment(a_tcp, ds_new,from_client);
            }
            else {
                free(ds_new);
                printf("memory alloc error!\n");
            }
        }
        else
        {
            printf("memory alloc error!\n");
        }
    }

    return ;
}

/* single processing function */
void system_destroy( int sig )
{
    pthread_cancel(pid_up_stream);
    pcap_close(pt_up_stream);
    packet_send_close(&net_up);
    packet_send_close(&net_down);
	stop_zcp_loop=1;
    exit(0);
}

int init_packet_send()
{
    net_up.type=PACKET_TYPE_LINK;
    if(packet_send_init( &net_up, dev_down, errbuf )==-1)
    {
        fprintf(stderr, "packet_send_init fail:%s \n", errbuf );
        return -1;
    }

    net_down.type=PACKET_TYPE_LINK;
    if(packet_send_init( &net_down, dev_up, errbuf )==-1)
    {
        fprintf(stderr, "packet_send_init fail:%s ", errbuf );
        packet_send_close(&net_up);
        return -1;
    }
}

int g_init_packet_send()
{
    g_net.type=PACKET_TYPE_LINK;
    if(packet_send_init( &g_net, g_dev, errbuf )==-1)
    {
        fprintf(stderr, "packet_send_init fail:%s \n", errbuf );
        return -1;
    }
}

void init_default()
{
    //strcpy(dev_up,"eth1");// "p2p2");
    //strcpy(dev_down,"eth0");// "p2p1");
    //nids_params.device=dev_down;
    strcpy(g_dev,"eth1");// "p2p2");
    //strcpy(g_dev,"eth3");// "p2p2");
    // sunpy:
    //strcpy(g_dev,"eth3");// "p2p2");
    nids_params.device=g_dev;
    //nids_params.pcap_timeout=2048;
    nids_params.send_time_out=10000;
    nids_params.tcp_flow_timeout=6000;
    nids_params.pkt_queue_num=5;
    nids_params.promisc=0;
    nids_params.n_hosts=1000000;
    nids_params.n_tcp_streams=DATA_CACHE_TABLE_SIZE;//tcp stream table size
    nids_params.queue_limit=500000; //max number packet to deal with, packet over this number will be dorped
    nids_params.pcap_timeout=1;
}

typedef int (*FILTER_FUN_TYPE)();

int system_init()
{
    init_default();
    if(read_config()==-1)
    {
        printf("read_config error!\n");
        exit(0);
    }

    if (!nids_init ())
    {
        fprintf(stderr,"%s\n",nids_errbuf);
        exit(1);
    }

    memset(data_cache_table, 0x00, sizeof(data_cache_table));

    nids_register_tcp ((void*)tcp_stream_handler);
    //nids_register_filter((FILTER_FUN_TYPE)mac_filter);
    nids_register_filter((FILTER_FUN_TYPE)g_filter);
    //nids_register_packet_send((void*)g_packet_send);
    nids_register_reset_callback(tcp_stream_handler);	
/*
    if(g_init_packet_send()==-1)
    {
        pcap_close(pt_up_stream);
        exit(0);
    }
*/
    /* regise signal ctrl+c stop capture */
    signal( SIGINT, system_destroy ) ;

#ifndef ZCP
#ifdef G_DEV
    if(create_up_stream_capture_thread()==-1)
    {
        system_destroy(0);
        return 0;
    }
	printf("create_up_stream_capture_thread ok!\n");
#endif
#endif

    return 0;
}

void system_run()
{
    nids_run ();
}

