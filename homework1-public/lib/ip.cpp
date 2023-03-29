#include "../include/ip.h"

#include "../include/lib.h"
#include <arpa/inet.h>

using namespace std;

void read_router_config(char *filename, vector<struct route_table_entry> &route_table) {
	cerr << "Reading router config from " << filename << endl;
	struct route_table_entry entry;
	ifstream file(filename);
	string line;

	while (getline(file, line)) {
		// Parse the line
		istringstream iss(line);
		string ip, mask, next_hop, interface;
		iss >> ip >> next_hop >> mask >> interface;

		// Convert the strings
		entry.prefix = inet_addr(ip.c_str());
		entry.next_hop = inet_addr(next_hop.c_str());
		entry.mask = inet_addr(mask.c_str());
		entry.interface = atoi(interface.c_str());

		// Add the entry to the routing table
		route_table.push_back(entry);
	}

	cerr << "Routing table:" << endl;

	file.close();
}

route_table_entry find_next_hop(uint32_t ip_addr, vector<struct route_table_entry> &route_table) {
	// Find the longest prefix match
	// Store the longest prefix match in the variable "entry"
	route_table_entry best_entry;
	best_entry.prefix = 0;
	best_entry.mask = 0;
	best_entry.next_hop = 0;
	best_entry.interface = -1;

    for (auto it = route_table.begin(); it != route_table.end(); it++) {
        if ((ip_addr & it->mask) == it->prefix) {
            if (ntohl(it->mask) > ntohl(best_entry.mask)) {
                best_entry = *it;
            }
        }
    }

	return best_entry;
}

