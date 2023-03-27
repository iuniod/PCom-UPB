#include <unistd.h>
#include <stdint.h>

#define ETHERTYPE_IP 0x0800
#define ETHERTYPE_ARP 0x0806

#define IP_PROTO_ICMP 1
#define ICMP_ECHO 8
#define ICMP_ECHOREPLY 0
#define ICMP_DEST_UNREACH 3
#define ICMP_TIME_EXCEEDED 11

#define MAX_LEN 1500

#define ARP_TABLE_NAME "arp_table.txt"

typedef struct {
    size_t len;
	char payload[MAX_LEN];
	int interface;
} packet;
