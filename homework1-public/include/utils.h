
#ifndef UTILS_H
#define UTILS_H

#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include "protocols.h"
#include "lib.h"

#define ETHERTYPE_IP 0x0800
#define ETHERTYPE_ARP 0x0806

#define MAX_LEN 1500

struct packet {
    size_t len;
	char payload[MAX_LEN];
	int interface;
};

using namespace std;

/**
 * @brief Print the mac address in format xx:xx:xx:xx:xx:xx
 * 
 * @param mac_addr the mac address
 */
void print_mac_addr(uint8_t* mac_addr);

/**
 * @brief Parse the arp table from "arp_table.txt"
 * 
 * @param arp_table the arp table
 */
void parse_arp_table(unordered_map<uint32_t, uint8_t*> &arp_table);

/**
 * @brief Parse the routing table from the file
 * 
 * @param routing_table the routing table
 * @param file_name the file name
 */
void parse_router_table(vector<struct route_table_entry> &routing_table, char* file_name);

/**
 * @brief Get the next hop object
 * 
 * @param ip_addr the ip address of the destination
 * @param routing_table the routing table
 * @return route_table_entry the next hop - the interface will be -1 if the destination is not reachable
 */
route_table_entry get_next_hop(uint32_t ip_addr, vector<struct route_table_entry> routing_table);


#endif // UTILS_H