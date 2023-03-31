#include "../include/utils.h"


using namespace std;

void parse_arp_table(unordered_map<uint32_t, uint8_t*> &arp_table) {
	ifstream file("arp_table.txt");
	DIE(file.fail(), "Failed to open arp_table.txt");

	string line;
	while (getline(file, line)) {
		istringstream iss(line);
		string ip, mac;
		iss >> ip >> mac;

		uint32_t ip_addr = inet_addr(ip.c_str());
		uint8_t* mac_addr = (uint8_t *) malloc(6);
		hwaddr_aton(mac.c_str(), mac_addr);

		arp_table[ip_addr] = mac_addr;
	}

	file.close();
}

void parse_router_table(vector<struct route_table_entry> &routing_table, char* file_name) {
	ifstream file(file_name);
	DIE(file.fail(), "Failed to open %s", file_name);

	string line;
	while (getline(file, line)) {
		istringstream iss(line);
		string prefix, next_hop, mask, interface;
		iss >> prefix >> next_hop >> mask >> interface;

		struct route_table_entry entry;
		entry.prefix = inet_addr(prefix.c_str());
		entry.next_hop = inet_addr(next_hop.c_str());
		entry.mask = inet_addr(mask.c_str());
		entry.interface = atoi(interface.c_str());

		routing_table.push_back(entry);
	}

	file.close();

	// Sort the routing table by prefix ascending, and if the prefix is the same, sort by mask descending
	sort(routing_table.begin(), routing_table.end(), [](const struct route_table_entry& a, const struct route_table_entry& b) {
		if (a.prefix == b.prefix) {
			return a.mask > b.mask;
		}
		return a.prefix < b.prefix;
	});
}

bool is_possible_match(uint32_t ip_addr, struct route_table_entry entry) {
	return (ip_addr & entry.mask) == entry.prefix;
}

route_table_entry get_next_hop(uint32_t ip_addr, vector<struct route_table_entry> &routing_table) {
	struct route_table_entry entry;
	entry.interface = -1;

	for (auto it = routing_table.begin(); it != routing_table.end(); it++) {
		if (is_possible_match(ip_addr, *it)) {
			if (ntohl(it->mask) > ntohl(entry.mask)) {
				entry = *it;
			}
		}
	}

	return entry;
}


