#include "../include/icmp.h"
#include "../include/lib.h"
#include "../include/protocols.h"
#include <bits/stdc++.h>
#include <arpa/inet.h>

using namespace std;

void icmp_handler(packet pack, int icmp_type) {
	// The ICMP header starts right after the IP header
	ether_header *eth_hdr = (ether_header *) pack.payload;
	iphdr *ip_hdr = (iphdr *) (pack.payload + sizeof(struct ether_header));
	icmphdr *icmp_hdr = (icmphdr *) (pack.payload + sizeof(struct ether_header) + sizeof(iphdr));

	// Check if the packet is an echo request
	if (icmp_type == ICMP_ECHO) {
		icmp_hdr->type = ICMP_ECHOREPLY;

		swap(ip_hdr->saddr, ip_hdr->daddr);
	} else {
		// The packet is time exceeded or unreachable
        ip_hdr->protocol = IPPROTO_ICMP;
        ip_hdr->tot_len = htons(sizeof(iphdr) + sizeof(struct icmphdr));
        ip_hdr->check = 0;
        ip_hdr->ttl = 64;
        ip_hdr->check = checksum((uint16_t *) ip_hdr, sizeof(struct iphdr));

		icmp_hdr->type = icmp_type;
		icmp_hdr->code = 0;
		icmp_hdr->checksum = 0;
		icmp_hdr->checksum = checksum((uint16_t *) icmp_hdr, sizeof(struct icmphdr));

        pack.len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr);
	}

	// Send the packet
	send_to_link(pack.interface, pack.payload, pack.len);
}
