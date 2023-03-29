#include "../include/arp.h"

#include "../include/lib.h"
#include <arpa/inet.h>

using namespace std;

void parse_arp_table(char *filename, unordered_map<uint32_t, uint8_t*>  &arp_table) {
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

void debug_parse_arp_table(unordered_map<uint32_t, uint8_t*>  &arp_table) {
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
