#include "../include/handlers.h"

void arp_request(uint32_t ip_addr, int interface, int old_interface) {
	/* Create the ARP request packet */
	packet pack;
	pack.interface = interface;
	pack.len = ARP_LEN;

	/* Create the Ethernet header */
	ether_header *eth_hdr = (ether_header *) pack.payload;
	eth_hdr->ether_type = htons(ETHERTYPE_ARP);
	get_interface_mac(old_interface, eth_hdr->ether_shost);
	memset(eth_hdr->ether_dhost, 0xFF, 6);

	/* Create the ARP header */
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

	/* Send the packet */
	send_to_link(pack.interface, pack.payload, pack.len);

}

void icmp_handler(packet pack, int icmp_type) {
	ether_header *eth_hdr = (ether_header *) pack.payload;
	iphdr *ip_hdr = (iphdr *) (pack.payload + IP_OFFSET);
	icmphdr *icmp_hdr = (icmphdr *) (pack.payload + ICMP_OFFSET);

		eth_hdr->ether_type = htons(ETHERTYPE_IP);
		memcpy(eth_hdr->ether_dhost, eth_hdr->ether_shost, 6);
		get_interface_mac(pack.interface, eth_hdr->ether_shost);

		swap(ip_hdr->saddr, ip_hdr->daddr);
		ip_hdr->frag_off = 0;
		ip_hdr->tos = 0;
		ip_hdr->version = 4;
		ip_hdr->ihl = 5;
		ip_hdr->id = 1;
		ip_hdr->protocol = 1;
		ip_hdr->ttl = 64;
		ip_hdr->tot_len = htons(IP_LEN);
		ip_hdr->check = 0;
		ip_hdr->check = htons(checksum((uint16_t *) ip_hdr, ICMP_SIZE));

		icmp_hdr->code = 0;
		icmp_hdr->type = icmp_type;
		icmp_hdr->checksum = 0;
		icmp_hdr->checksum = htons(checksum((uint16_t *) icmp_hdr, ICMP_SIZE));

		pack.len = ICMP_LEN;
	
	/* Send the packet */
	send_to_link(pack.interface, pack.payload, pack.len);
}

void ip_handler(packet pack, vector<struct route_table_entry> routing_table,
                unordered_map<uint32_t, uint8_t*> &arp_table, queue<struct packet> &packet_queue) {
	struct ether_header *eth_hdr = (ether_header *) pack.payload;
	struct iphdr *ip_hdr = (iphdr *) (pack.payload + IP_OFFSET);
	struct icmphdr *icmp_hdr = (icmphdr *) (pack.payload + ICMP_OFFSET);

	/* Check the checksum, if it's wrong, drop the packet */
	uint16_t checksum_ip = ip_hdr->check;
	if (checksum((uint16_t *) ip_hdr, IP_SIZE)) {
		cerr << "Wrong checksum for IP packet" << endl;
		return;
	}

	/* Check if the packet is for me and send ICMP echo reply */
	uint32_t ip_addr = inet_addr(get_interface_ip(pack.interface));
	if (ip_hdr->daddr == ip_addr) {
		cerr << "Packet is ICMP REQUEST" << endl;
		icmp_handler(pack, ICMP_ECHOREPLY);
		return;
	}

	/* Check TTL, and send ICMP time exceeded message*/
	if (ip_hdr->ttl <= 1) {
		cerr << "TTL is less than 2" << endl;
		icmp_handler(pack, ICMP_TIME_EXCEEDED);
		return;
	}

	/* Find the next hop */
	cerr << "Finding the next hop for: " << inet_ntoa(*(in_addr *) &ip_hdr->daddr) << endl;
	route_table_entry next_hop = get_next_hop(ip_hdr->daddr, routing_table);
	cerr << "Next hp interface: " << next_hop.interface << endl;
	if (next_hop.interface == -1) {
		cerr << "No route to host" << endl;
		icmp_handler(pack, ICMP_DEST_UNREACHABLE);
		return;
	}


	/* Update TTL and checksum */
	ip_hdr->ttl--;
	ip_hdr->check = 0;
	ip_hdr->check = htons(checksum((uint16_t *) ip_hdr, IP_SIZE));

	/* Check if the next hop is in the ARP table */
	if (arp_table.find(next_hop.next_hop) == arp_table.end()) {
		cerr << "Next hop is not in the ARP table" << endl;

		/* Send ARP request */
		packet_queue.push(pack);
		arp_request(next_hop.next_hop, next_hop.interface, pack.interface);

		return;
	}

	/* Send the packet to the next hop */
	memcpy(eth_hdr->ether_dhost, arp_table[next_hop.next_hop], 6);
	get_interface_mac(pack.interface, eth_hdr->ether_shost);

	/* Print MAC addresses */
	cerr << "Source MAC: ";
	print_mac_addr(eth_hdr->ether_shost);

	cerr << "Destination MAC: ";
	print_mac_addr(eth_hdr->ether_dhost);

	/* Send the packet */
	send_to_link(next_hop.interface, pack.payload, pack.len);
}

void arp_handler(packet pack, vector<struct route_table_entry> routing_table,
                 unordered_map<uint32_t, uint8_t*> &arp_table, queue<struct packet> &packet_queue) {
	struct ether_header *eth_hdr = (ether_header *) pack.payload;
	struct arp_header *arp_hdr = (arp_header *) (pack.payload + ARP_OFFSET);

	/* Check if the packet is an ARP request */
	cerr << "Checking if the packet type" << endl;
	int arp_type = ntohs(arp_hdr->op);

	switch (ntohs(arp_hdr->op)) {
		case ARP_REQUEST: {
			cerr << "ARP request" << endl;

			/* Add the new entry in the ARP table */
			arp_table[arp_hdr->spa] = (uint8_t *) malloc(6);
			memcpy(arp_table[arp_hdr->spa], arp_hdr->sha, 6);

			/* Send ARP reply */
			arp_hdr->op = htons(ARP_REPLY);

			uint32_t temp = arp_hdr->spa;
			arp_hdr->spa = arp_hdr->tpa;
			arp_hdr->tpa = temp;

			memcpy(arp_hdr->tha, arp_hdr->sha, 6);
			get_interface_mac(pack.interface, arp_hdr->sha);

			get_interface_mac(pack.interface, eth_hdr->ether_shost);
			memcpy(eth_hdr->ether_dhost, arp_hdr->tha, 6);

			/* Send the packet */
			send_to_link(pack.interface, pack.payload, pack.len);

			break;
		} case ARP_REPLY: {
			cerr << "ARP reply" << endl;
			/* Add the new entry in the ARP table */
			arp_table[arp_hdr->spa] = (uint8_t *) malloc(6); 
			memcpy(arp_table[arp_hdr->spa], arp_hdr->sha, 6);
	
			/* Send the packets from the queue */
			int size = packet_queue.size();
			for (int i = 0; i < size; i++) {
				packet p = packet_queue.front();
				packet_queue.pop();
				ip_handler(p, routing_table, arp_table, packet_queue);
			}

			break;
		} default:
			cerr << "Unknown ARP type" << endl;
			break;
	}
}
