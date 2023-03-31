#include "../include/handlers.h"

void icmp_handler(packet pack, uint8_t icmp_type, uint8_t icmp_code) {
	// Check if the packet is echo request
	struct iphdr *ip_hdr = (iphdr *) (pack.payload + IP_OFFSET);
	struct icmphdr *icmp_hdr = (icmphdr *) (pack.payload + ICMP_OFFSET);
	
	/* Check if the packet is echo request and send ICMP echo reply */
	if (icmp_hdr->type == ICMP_ECHO) {
		cerr << "Sending ICMP echo reply" << endl;
		icmp_hdr->type = icmp_type;
		icmp_hdr->code = icmp_code;
		icmp_hdr->checksum = 0;
		icmp_hdr->checksum = htons(checksum((uint16_t *) icmp_hdr, sizeof(struct icmphdr)));
		ip_hdr->check = 0;
		swap(ip_hdr->saddr, ip_hdr->daddr);
		ip_hdr->check = htons(checksum((uint16_t *) ip_hdr, sizeof(struct iphdr)));
		send_to_link(pack.interface, pack.payload, pack.len);
	}
}

void ip_handler(packet pack, vector<struct route_table_entry> &routing_table,
                unordered_map<uint32_t, uint8_t*> &arp_table, queue<struct packet> &packet_queue) {
	struct ether_header *eth_hdr = (ether_header *) pack.payload;
	struct iphdr *ip_hdr = (iphdr *) (pack.payload + IP_OFFSET);
	struct icmphdr *icmp_hdr = (icmphdr *) (pack.payload + ICMP_OFFSET);

	/* Check the checksum, if it's wrong, drop the packet */
	uint16_t checksum_ip = ip_hdr->check;
	if (checksum((uint16_t *) ip_hdr, sizeof(struct iphdr)) != 0) {
		cerr << "Wrong checksum for IP packet" << endl;
		return;
	}
	cerr << "Checksum for IP packet is correct: " << ntohs(checksum_ip) << endl;

	/* Check TTL, and send ICMP time exceeded message*/
	if (ip_hdr->ttl <= 1) {
		cerr << "TTL is less than 2" << endl;
		return;
	}
	cerr << "TTL is greater than 1" << endl;


	/* Update TTL and checksum */
	ip_hdr->ttl--;
	ip_hdr->check = 0;
	ip_hdr->check = htons(checksum((uint16_t *) ip_hdr, sizeof(struct iphdr)));

	/* Check if the packet is for me and send ICMP echo reply */
	uint32_t ip_addr = inet_addr(get_interface_ip(pack.interface));
	if (ip_hdr->daddr == ip_addr ) {
		cerr << "Packet is for me" << endl;
		if (icmp_hdr->type != ICMP_ECHO) {
			cerr << "Sending ICMP echo reply" << endl;
			icmp_handler(pack, ICMP_ECHOREPLY, 0);
		}
		return;
	}
	cerr << "Finding the next hop" << endl;

	/* Find the next hop */
	route_table_entry next_hop = get_next_hop(ip_hdr->daddr, routing_table);
	cerr << "Next hp interface: " << next_hop.interface << endl;
	if (next_hop.interface == -1) {
		cerr << "No route to host" << endl;
		return;
	}

	/* Check if the next hop is in the ARP table */
	if (arp_table.find(next_hop.next_hop) == arp_table.end()) {
		cerr << "Next hop is not in the ARP table" << endl;
		return;
	}

	/* Send the packet to the next hop */
	memcpy(eth_hdr->ether_dhost, arp_table[next_hop.next_hop], 6);
	uint8_t* mac_addr = (uint8_t *) malloc(6);
	get_interface_mac(next_hop.interface, mac_addr);
	memcpy(eth_hdr->ether_shost, mac_addr, 6);

	// Print MAC addresses
	cerr << "Source MAC: ";
	for (int i = 0; i < 6; i++) {
		cerr << hex << (int) eth_hdr->ether_shost[i] << ":";
	}
	cerr << endl;
	cerr << "Destination MAC: ";
	for (int i = 0; i < 6; i++) {
		cerr << hex << (int) eth_hdr->ether_dhost[i] << ":";
	}
	cerr << endl;

	send_to_link(next_hop.interface, pack.payload, pack.len);
}