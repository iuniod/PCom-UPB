#include "../include/utils.h"

using namespace std;

void print_mac_addr(uint8_t* mac_addr) {
	for (int i = 0; i < 6; i++) {
		cerr << hex << (int) mac_addr[i] << ":";
	}
	cerr << endl;
}

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

	sort(routing_table.begin(), routing_table.end(), [](const struct route_table_entry& a, const struct route_table_entry& b) {
		if (ntohl(a.prefix & a.mask) < ntohl(b.prefix & b.mask)) {
			return true;
		} else if (ntohl(a.prefix & a.mask) > ntohl(b.prefix & b.mask)) {
			return false;
		} else {
			return ntohl(a.mask) > ntohl(b.mask) ? false : true;
		}
	});
}

int is_possible_match(uint32_t ip_addr, struct route_table_entry entry) {
	return (ntohl(ip_addr & entry.mask)) == (ntohl(entry.prefix & entry.mask)) ? 0 : 
			(ntohl(ip_addr & entry.mask) > ntohl(entry.prefix & entry.mask) ? 1 : -1);
}

route_table_entry get_next_hop(uint32_t ip_addr, vector<struct route_table_entry> routing_table) {
	struct route_table_entry entry;
	entry.interface = -1;
	entry.mask = 0;

	int left = 0, right = routing_table.size() - 1;

	while (left <= right) {
		int mid = (left + right) / 2;
		int cmp = is_possible_match(ip_addr, routing_table[mid]);

		if (cmp == 0) {
			entry = routing_table[mid];
			left = mid + 1;
		} else if (cmp == 1) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}

	return entry;
}
