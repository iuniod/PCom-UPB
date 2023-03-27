#include "include/lib.h"
#include "include/protocols.h"
#include "include/utils.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>

using namespace std;

// IP address is in network order and MAC address is in host order
unordered_map<uint32_t, uint8_t*> arp_table;
vector<struct route_table_entry> route_table;
queue<packet> packet_queue;

void parse_arp_table(char *filename) {
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
		// Parse the line
        istringstream iss(line);
        string ip, mac;
        iss >> ip >> mac;

		// Convert the ip and mac addresses
        uint32_t ip_addr = inet_addr(ip.c_str());
        
		uint8_t *mac_addr = (uint8_t *) malloc(6);
        hwaddr_aton(mac.c_str(), mac_addr);

		// Add the ip and mac addresses to the arp table
		arp_table[ip_addr] = mac_addr;
    }

	file.close();
}

void read_router_config(char *filename) {
	struct route_table_entry entry;
	vector<struct route_table_entry> route_table;
	ifstream file(filename);
	string line;

	while (getline(file, line)) {
		// Parse the line
		istringstream iss(line);
		string ip, mask, next_hop, interface;
		iss >> ip >> mask >> next_hop >> interface;

		// Convert the strings
		entry.prefix = inet_addr(ip.c_str());
		entry.mask = inet_addr(mask.c_str());
		entry.next_hop = inet_addr(next_hop.c_str());
		entry.interface = atoi(interface.c_str());

		// Add the entry to the routing table
		route_table.push_back(entry);
	}

	sort(route_table.begin(), route_table.end(), [](const struct route_table_entry &a, const struct route_table_entry &b) {
		if (a.prefix == b.prefix) {
			return a.mask < b.mask;
		}

		return a.prefix < b.prefix;
	});

	file.close();
}

void debug_parse_arp_table() {
	// Print the arp table
	// MAC addresses are printed in the format XX:XX:XX:XX:XX:XX
	// IP addresses are printed in the format X.X.X.X
	cout << "ARP table:" << endl;
	for (auto it = arp_table.begin(); it != arp_table.end(); it++) {
		uint32_t ip_addr = it->first;
		uint8_t *mac_addr = it->second;

		printf("%s ", inet_ntoa(*(in_addr *) &ip_addr));
		printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	}
}

void icmp_handler(packet pack) {
	// The ICMP header starts right after the IP header
	iphdr *ip_hdr = (iphdr *) (pack.payload + sizeof(struct ether_header));
	icmphdr *icmp_hdr = (icmphdr *) (pack.payload + sizeof(struct ether_header) + sizeof(iphdr));

	// Check if the packet is an echo request
	if (icmp_hdr->type == ICMP_ECHO) {
		// Change the type of the packet to echo reply
		icmp_hdr->type = ICMP_ECHOREPLY;

		// Swap the source and destination MAC addresses
		swap(ip_hdr->saddr, ip_hdr->daddr);

		// Send the packet
		send_to_link(pack.interface, pack.payload, pack.len);
	}
}

void sent_reply_icmp_packet(packet pack, int icmp_type, int i) {
	packet reply_pack;

	struct ether_header* reply_eth_hdr;
	iphdr* reply_ip_hdr;
	struct icmphdr* reply_icmp_hdr;
	void *payload;

	memcpy(reply_eth_hdr->ether_dhost, dha, 6);
	memcpy(reply_eth_hdr->ether_shost, sha, 6);
	reply_eth_hdr->ether_type = htons(ETHERTYPE_IP);
	/* No options */
	reply_ip_hdr->version = 4;
	reply_ip_hdr->ihl = 5;
	reply_ip_hdr->tos = 0;
	reply_ip_hdr->protocol = IPPROTO_ICMP;
	reply_ip_hdr->tot_len = htons(sizeof(iphdr) + sizeof(struct icmphdr));
	reply_ip_hdr->id = htons(1);
	reply_ip_hdr->frag_off = 0;
	reply_ip_hdr->ttl = 64;
	reply_ip_hdr->check = 0;
	reply_ip_hdr->daddr = daddr;
	reply_ip_hdr->saddr = saddr;
	reply_ip_hdr->check = checksum((uint16_t *) reply_ip_hdr, sizeof(struct iphdr));

	reply_icmp_hdr->type = icmp_type;
	reply_icmp_hdr->code = 0,
	reply_icmp_hdr->checksum = 0,
	
	reply_icmp_hdr.checksum = checksum((uint16_t *) reply_icmp_hdr, sizeof(struct icmphdr));

	payload = reply_pack.payload;
	memcpy(payload, &reply_eth_hdr, sizeof(struct ether_header));
	payload += sizeof(struct ether_header);
	memcpy(payload, &ip_hdr, sizeof(struct iphdr));
	payload += sizeof(struct iphdr);
	memcpy(payload, &icmp_hdr, sizeof(struct icmphdr));
	reply_pack.len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr);

	send_packet(interface, &packet);
}

route_table_entry find_next_hop(uint32_t ip_addr) {
	// Find the longest prefix match
	// Store the longest prefix match in the variable "entry"
	route_table_entry best_entry;
	best_entry.prefix = 0;
	best_entry.mask = 0;
	best_entry.next_hop = 0;
	best_entry.interface = 0;

	// Binary search
	int left = 0;
	int right = route_table.size() - 1;
	while (left <= right) {
		int mid = (left + right) / 2;
		route_table_entry entry = route_table[mid];

		if ((ip_addr & entry.mask) == entry.prefix) {
			// Found a match
			if (entry.mask > best_entry.mask) {
				// Found a better match
				best_entry = entry;
			}

			// Continue searching for a better match
			left = mid + 1;
		} else if ((ip_addr & entry.mask) < entry.prefix) {
			// Continue searching in the left half
			right = mid - 1;
		} else {
			// Continue searching in the right half
			left = mid + 1;
		}
	}

	if (best_entry.prefix == 0) {
		// No match found
		best_entry.interface = -1;
	}

	return best_entry;
}

uint8_t get_mac_from_arp_table(uint32_t ip_addr, uint8_t *mac_addr) {
	// Check if the IP address is in the arp table
	if (arp_table.find(ip_addr) != arp_table.end()) {
		// The IP address is in the arp table
		// Copy the MAC address from the arp table
		memcpy(mac_addr, arp_table[ip_addr], 6);
		return 1;
	}

	// The IP address is not in the arp table
	return 0;
}

int main(int argc, char *argv[]) {
	// Do not modify this line
	init(argc - 2, argv + 2);

    // Parse the arp table into an unordered map
    parse_arp_table(ARP_TABLE_NAME);

	// Read the router's configuration file
	read_router_config(argv[1]);

	int i = 0;

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
		
		if (memcmp(eth_hdr->ether_dhost, pack_mac, 6) && memcmp(eth_hdr->ether_shost, mac_broadcast, 6)) {
			break;
		}

		/*Note that packets received are in network order,
		any header field which has more than 1 byte will need to be conerted to
		host order. For example, ntohs(eth_hdr->ether_type). The oposite is needed when
		sending a packet on the link,*/

		switch(ntohs(eth_hdr->ether_type)) {
			case ETHERTYPE_IP: {
				// IP header starts right after the ethernet header
				iphdr *ip_hdr = (iphdr *) (pack.payload + sizeof(struct ether_header));
				// IP address of the interface on which the packet was received
				uint32_t ip_addr = inet_addr(get_interface_ip(pack.interface));

				// Check if the packet is for me and the protocol is ICMP
				if (ip_hdr->daddr == ip_addr && ip_hdr->protocol == IP_PROTO_ICMP) {
					icmp_handler(pack);
					break;
				}

				// Forward the packet
				// Verify the checksum
				uint16_t chk_sum = ip_hdr->check;
				ip_hdr->check = 0;
				if (chk_sum != checksum((uint16_t *) ip_hdr, sizeof(iphdr))) {
					break;
				}

				// Decrement the TTL
				if (ip_hdr->ttl <= 1) {
					sent_reply_icmp_packet(pack, ICMP_TIME_EXCEEDED, i);
					break;
				}
				ip_hdr->ttl--;

				// Find the next hop
				route_table_entry next_hop = find_next_hop(ip_hdr->daddr);
				if (next_hop.interface == -1) {
					sent_reply_icmp_packet(pack, ICMP_DEST_UNREACH, i);
					break;
				}

				// Update the checksum
				ip_hdr->check = 0;
				ip_hdr->check = checksum((uint16_t *) ip_hdr, sizeof(iphdr));

				// Find the new MAC address
				uint8_t *next_hop_mac = (uint8_t *) malloc(6);
				i++;
				if (get_mac_from_arp_table(next_hop.next_hop, next_hop_mac)) {
					// The MAC address is in the arp table
					// Send the packet
					
					// memcpy(eth_hdr->ether_dhost, next_hop_mac, 6);
					// memcpy(eth_hdr->ether_shost, pack_mac, 6);

					send_to_link(next_hop.interface, pack.payload, pack.len);
				} else {
					// The MAC address is not in the arp table
					// Send an ARP request
					// send_arp_request(next_hop.interface, next_hop.next_hop);
				}

				
				break;
			}case ETHERTYPE_ARP:{
				break;
			}default:
				break;
		}
	}
}
