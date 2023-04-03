#include "include/handlers.h"

using namespace std;

unordered_map<uint32_t, uint8_t*> arp_table;
vector<struct route_table_entry> routing_table;
queue<struct packet> packet_queue;

int main(int argc, char *argv[]) {
	// Do not modify this line
	init(argc - 2, argv + 2);

	/* Uncomment the next line if you want to see a static arp table, named arp_table.txt */
	// parse_arp_table(arp_table);
	parse_router_table(routing_table, argv[1]);

	while (1) {
        packet pack;

		pack.interface = recv_from_any_link(pack.payload, &(pack.len));
		DIE(pack.interface < 0, "recv_from_any_links");

		struct ether_header* eth_hdr = (struct ether_header*) pack.payload;

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
				cerr << "----- Unknown packet received -----" << endl;
				break;
		}
	}
}
