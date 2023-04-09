#ifndef HANDLERS_H
#define HANDLERS_H

#include <bits/stdc++.h>
#include "utils.h"

using namespace std;

/**
 * @brief Handles IP packets
 * 
 * @param pack The packet to be handled
 * @param routing_table The routing table
 * @param arp_table The ARP table
 * @param packet_queue The queue of packets to be sent
 */
void ip_handler(packet pack, vector<struct route_table_entry> routing_table,
                unordered_map<uint32_t, uint8_t*> &arp_table, queue<struct packet> &packet_queue);

/**
 * @brief Handles ARP packets
 * 
 * @param pack The packet to be handled
 * @param routing_table The routing table
 * @param arp_table The ARP table
 * @param packet_queue The queue of packets to be sent
 */
void arp_handler(packet pack, vector<struct route_table_entry> routing_table,
                 unordered_map<uint32_t, uint8_t*> &arp_table, queue<struct packet> &packet_queue);

#endif // HANDLERS_H
