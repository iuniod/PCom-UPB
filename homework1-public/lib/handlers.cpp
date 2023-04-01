#include "../include/handlers.h"

void arp_request(uint32_t ip_addr, int interface, queue<packet> &packet_queue) {
	// Create the ARP request packet
	packet pack;
	pack.interface = interface;
	pack.len = sizeof(struct ether_header) + sizeof(struct arp_header);

	// Create the Ethernet header
	ether_header *eth_hdr = (ether_header *) pack.payload;
	eth_hdr->ether_type = htons(ETHERTYPE_ARP);
	get_interface_mac(interface, eth_hdr->ether_shost);
	memset(eth_hdr->ether_dhost, 0xFF, 6);

	// Create the ARP header
	arp_header *arp_hdr = (arp_header *) (pack.payload + ARP_OFFSET);
	arp_hdr->htype = htons(1);
	arp_hdr->ptype = htons(ETHERTYPE_IP);
	arp_hdr->hlen = 6;
	arp_hdr->plen = 4;
	arp_hdr->op = htons(ARP_REQUEST);
	get_interface_mac(interface, arp_hdr->sha);
	arp_hdr->spa = inet_addr(get_interface_ip(interface));
	memset(arp_hdr->tha, 0, 6);
	arp_hdr->tpa = ip_addr;

	send_to_link(pack.interface, pack.payload, pack.len);

}

void icmp_handler(packet pack, int icmp_type) {
	// The ICMP header starts right after the IP header
	ether_header *eth_hdr = (ether_header *) pack.payload;
	iphdr *ip_hdr = (iphdr *) (pack.payload + sizeof(struct ether_header));
	icmphdr *icmp_hdr = (icmphdr *) (pack.payload + sizeof(struct ether_header) + sizeof(iphdr));

	// Check if the packet is an echo request
	if (icmp_type == ICMP_ECHOREPLY) {
		ip_hdr->protocol = IPPROTO_ICMP;
		// ip_hdr->tot_len = htons(sizeof(iphdr) + sizeof(struct icmphdr));
		ip_hdr->check = 0;
		ip_hdr->ttl = 64;
		ip_hdr->check = htons(checksum((uint16_t *) ip_hdr, sizeof(struct iphdr)));

		icmp_hdr->type = ICMP_ECHOREPLY;
		icmp_hdr->checksum = 0;
		icmp_hdr->checksum = htons(checksum((uint16_t *) icmp_hdr, sizeof(struct icmphdr)));

		// swap(ip_hdr->saddr, ip_hdr->daddr);
		swap(eth_hdr->ether_shost, eth_hdr->ether_dhost);
		pack.len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr);
	} else {
		// The packet is time exceeded or unreachable
        ip_hdr->protocol = IPPROTO_ICMP;
        ip_hdr->tot_len = htons(sizeof(iphdr) + sizeof(struct icmphdr));
        ip_hdr->check = 0;
        ip_hdr->ttl = 64;
        ip_hdr->check = htons(checksum((uint16_t *) ip_hdr, sizeof(struct iphdr)));

		icmp_hdr->type = icmp_type;
		icmp_hdr->code = 0;
		icmp_hdr->checksum = 0;
		icmp_hdr->checksum = htons(checksum((uint16_t *) icmp_hdr, sizeof(struct icmphdr)));

        pack.len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr);
	}

	// Send the packet
	send_to_link(pack.interface, pack.payload, pack.len);
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
		icmp_handler(pack, ICMP_TIME_EXCEEDED);
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
		if (icmp_hdr->type == ICMP_ECHO) {
			cerr << "Sending ICMP echo reply" << endl;
			icmp_handler(pack, ICMP_ECHOREPLY);
		}
		return;
	}
	cerr << "Finding the next hop" << endl;

	/* Find the next hop */
	route_table_entry next_hop = get_next_hop(ip_hdr->daddr, routing_table);
	cerr << "Next hp interface: " << next_hop.interface << endl;
	if (next_hop.interface == -1) {
		cerr << "No route to host" << endl;
		icmp_handler(pack, ICMP_DEST_UNREACHABLE);
		return;
	}

	/* Check if the next hop is in the ARP table */
	if (arp_table.find(next_hop.next_hop) == arp_table.end()) {
		cerr << "Next hop is not in the ARP table" << endl;
		packet_queue.push(pack);

		/* Send ARP request */
		arp_request(next_hop.next_hop, next_hop.interface, packet_queue);

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

void arp_handler(packet pack, vector<struct route_table_entry> &routing_table,
                 unordered_map<uint32_t, uint8_t*> &arp_table, queue<struct packet> &packet_queue) {
	struct ether_header *eth_hdr = (ether_header *) pack.payload;
	struct arp_header *arp_hdr = (arp_header *) (pack.payload + ARP_OFFSET);

	/* Check if the packet is an ARP request */
	cerr << "Checking if the packet is an ARP request" << endl;
	if (ntohs(arp_hdr->ptype) == ETHERTYPE_IP && ntohs(arp_hdr->op) == ARP_REQUEST) {
		cerr << "ARP request" << endl;
		/* Check if the packet is for me */
		uint32_t ip_addr = inet_addr(get_interface_ip(pack.interface));
		if (arp_hdr->tpa != ip_addr) {
			cerr << "ARP request is not for me " << arp_hdr->tpa << " vs " << ip_addr << endl;
			return;
		}

		/* Send ARP reply */
		arp_hdr->op = htons(ARP_REPLY);
		memcpy(arp_hdr->tha, arp_hdr->sha, 6);
		arp_hdr->tpa = arp_hdr->spa;
		get_interface_mac(pack.interface, arp_hdr->sha);
		arp_hdr->spa = ip_addr;

		/* Send the packet */
		memcpy(eth_hdr->ether_dhost, arp_hdr->tha, 6);
		memcpy(eth_hdr->ether_shost, arp_hdr->sha, 6);

		send_to_link(pack.interface, pack.payload, pack.len);
	} else if (ntohs(arp_hdr->ptype) == ETHERTYPE_IP && ntohs(arp_hdr->op) == ARP_REPLY) {
		cerr << "ARP reply" << endl;
		/* Add the new entry in the ARP table */
		arp_table[arp_hdr->spa] = arp_hdr->sha;

		/* Send the packets from the queue */
		if (!packet_queue.empty()) {
			packet p = packet_queue.front();
			packet_queue.pop();
			ip_handler(p, routing_table, arp_table, packet_queue);
		}
	}
}