#include "include/lib.h"
#include "include/protocols.h"
#include "include/icmp.h"
#include "include/ip.h"
#include "include/arp.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>

using namespace std;

// IP address is in network order and MAC address is in host order
unordered_map<uint32_t, uint8_t*> arp_table;
vector<struct route_table_entry> routing_table;
queue<struct packet> packet_queue;
int n = 0;

uint8_t get_mac_from_arp_table(uint32_t ip_addr, uint8_t *mac_addr) {
	// Check if the IP address is in the arp table
	cerr << "Looking for " << inet_ntoa(*(in_addr *) &ip_addr) << endl;
	if (arp_table.find(ip_addr) != arp_table.end()) {
		// The IP address is in the arp table
		// Copy the MAC address from the arp table
		memcpy(mac_addr, arp_table[ip_addr], 6);
		return 1;
	}

	// The IP address is not in the arp table
	return 0;
}

// MAC address in hex format
string get_mac_addess(uint8_t *mac) {
	string mac_address = "";
	
	for (int i = 0; i < 6; i++) {
		char buf[3];
		sprintf(buf, "%02x", mac[i]);
		mac_address += buf;
		if (i != 5) {
			mac_address += ":";
		}
	}

	return mac_address;
}

int main(int argc, char *argv[]) {
	// Do not modify this line
	init(argc - 2, argv + 2);

    // Parse the arp table into an unordered map
    parse_arp_table(ARP_TABLE_NAME, arp_table);

	// Read the router's configuration file
	read_router_config(argv[1], routing_table);

	while (1) {
        packet pack;
		uint8_t* mac_broadcast = (uint8_t *) malloc(6);
		memset(mac_broadcast, 0xFF, 6);

		pack.interface = recv_from_any_link(pack.payload, &(pack.len));
		DIE(pack.interface < 0, "recv_from_any_links");

		struct ether_header *eth_hdr = (struct ether_header *) pack.payload;

		// Check if the packet is for me or is a broadcast packet and if not, drop it
		uint8_t *pack_mac = (uint8_t *) malloc(6);
		get_interface_mac(pack.interface, pack_mac);

		if (memcmp(eth_hdr->ether_dhost, pack_mac, 6) != 0 && memcmp(eth_hdr->ether_dhost, mac_broadcast, 6) != 0) {
			cerr << "Packet not for me" << endl;
			break;
		}

		cerr << "Received packet with length " << pack.len << " on interface " << pack.interface << endl;

		switch(ntohs(eth_hdr->ether_type)) {
			case ETHERTYPE_IP: {
				// IP header starts right after the ethernet header
				iphdr *ip_hdr = (iphdr *) (pack.payload + sizeof(struct ether_header));
				// IP address of the interface on which the packet was received
				uint32_t ip_addr = inet_addr(get_interface_ip(pack.interface));

				// Check if the packet is for me and the protocol is ICMP
				cerr << "IP packet received" << endl;
				if (ip_hdr->daddr == ip_addr) {
					if (ip_hdr->protocol == IP_PROTO_ICMP) {
						icmp_handler(pack, ICMP_ECHO);
					}

					break;
				}
				cerr << "IP packet not for me" << endl;

				// Verify the checksum
				// uint16_t chk_sum = ip_hdr->check;
				// ip_hdr->check = 0;
				// if (chk_sum != checksum((uint16_t *) ip_hdr, sizeof(iphdr))) {
				// 	break;
				// }
				cerr << "Checksum is ok" << endl;

				// Decrement the TTL
				if (ip_hdr->ttl <= 1) {
					icmp_handler(pack, ICMP_TIME_EXCEEDED);
					break;
				}
				cerr << "TTL is ok" << endl;

				// Find the next hop
				route_table_entry next_hop = find_next_hop(ip_hdr->daddr, routing_table);
				cerr << "Next hop for " << inet_ntoa(*(in_addr *) &ip_hdr->daddr) << " is on interface: " << next_hop.interface << endl;
				if (next_hop.interface == -1) {
					icmp_handler(pack, ICMP_DEST_UNREACH);
					break;
				}

				// Update the checksum
				ip_hdr->check = 0;
				ip_hdr->ttl--;
				ip_hdr->check = checksum((uint16_t *) ip_hdr, sizeof(iphdr));
				cerr << "Checksum updated" << endl;

				// Find the new MAC address
				get_interface_mac(next_hop.interface, eth_hdr->ether_shost); // Set the source MAC address
				cerr << "Source MAC address is " << get_mac_addess(eth_hdr->ether_shost) << endl;

				uint8_t *next_hop_mac = (uint8_t *) malloc(6);
				if (get_mac_from_arp_table(next_hop.next_hop, next_hop_mac)) {
					// The MAC address is in the arp table
					memcpy(eth_hdr->ether_dhost, next_hop_mac, sizeof(next_hop_mac));
					cerr << "Destination MAC address is " << get_mac_addess(eth_hdr->ether_dhost) << endl;
					send_to_link(next_hop.interface, pack.payload, pack.len);
					cerr << "Packet sent" << endl;
				} else {
					// The MAC address is not in the arp table
					// Send an ARP request
					// send_arp_request(next_hop.interface, next_hop.next_hop);
				}

				
				break;
			}case ETHERTYPE_ARP:{
				struct arp_header *arp_hdr = (struct arp_header *)(pack.payload + sizeof(struct ether_header));

				if (arp_hdr->op == htons(ARPOP_REQUEST)) {	// arp request

					uint8_t *mac = (uint8_t *) malloc(6);
					get_interface_mac(pack.interface, mac);

					memcpy(eth_hdr->ether_dhost, eth_hdr->ether_shost, 6);
					memcpy(eth_hdr->ether_shost, mac, 6);
					
					
					// change arp_hdr for reply
					arp_hdr->op = htons(ARPOP_REPLY);
					
					arp_hdr->tpa = arp_hdr->spa;
					arp_hdr->spa = inet_addr(get_interface_ip(pack.interface));
					

					memcpy(arp_hdr->sha, mac, 6);
					memcpy(arp_hdr->tha, eth_hdr->ether_dhost, 6);

					send_to_link(pack.interface, pack.payload, pack.len);
				}
				break;
			}default:
				break;
		}
	}
}
