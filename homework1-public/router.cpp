#include "include/handlers.h"

using namespace std;

// IP address is in network order and MAC address is in host order
unordered_map<uint32_t, uint8_t*> arp_table;
vector<struct route_table_entry> routing_table;
queue<struct packet> packet_queue;

void print (uint8_t* mac_addr) {
	for (int i = 0; i < 6; i++) {
		cerr << hex << (int) mac_addr[i] << ":";
	}
	cerr << endl;
}

int main(int argc, char *argv[]) {
	// Do not modify this line
	init(argc - 2, argv + 2);

	// parse_arp_table(arp_table);
	parse_router_table(routing_table, argv[1]);

	while (1) {
        packet pack;

		pack.interface = recv_from_any_link(pack.payload, &(pack.len));
		DIE(pack.interface < 0, "recv_from_any_links");

		struct ether_header* eth_hdr = (struct ether_header*) pack.payload;
		
		/* Check if the packet is for me, otherwise drop it*/
		// uint8_t* mac_broadcast = (uint8_t *) malloc(6); // Broadcast MAC address
		// uint8_t* mac_addr = (uint8_t *) malloc(6); // MAC address of the interface
		// memset(mac_broadcast, 0xFF, 6);
		// get_interface_mac(pack.interface, mac_addr);
		// if (memcmp(eth_hdr->ether_dhost, mac_broadcast, 6) != 0 && memcmp(eth_hdr->ether_dhost, mac_addr, 6) != 0) {
		// 	cerr << "Packet is not for me ";
		// 	print(eth_hdr->ether_dhost);
		// 	cerr << " vs ";
		// 	print(mac_addr);
		// 	cerr << endl;
		// 	free(mac_broadcast);
		// 	free(mac_addr);
		// 	continue;
		// }
		// free(mac_broadcast);
		// free(mac_addr);

		cerr << "Routing table: " << argv[1] << endl;
		switch(ntohs(eth_hdr->ether_type)) {
			case ETHERTYPE_IP: {
				cerr << "----- IP packet received -----" << endl;
				ip_handler(pack, routing_table, arp_table, packet_queue);
				break;
			}case ETHERTYPE_ARP:{
				cerr << "----- ARP packet received -----" << endl;
				arp_handler(pack, routing_table, arp_table, packet_queue);
				break;
			}default:
				break;
		}
	}
}
