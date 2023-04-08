#ifndef HANDLERS_H
#define HANDLERS_H

#include <bits/stdc++.h>
#include "utils.h"

#define IP_OFFSET sizeof(struct ether_header)
#define ICMP_OFFSET (sizeof(struct ether_header) + sizeof(struct iphdr))
#define ARP_OFFSET sizeof(struct ether_header)

#define ICMP_ECHO 8
#define ICMP_ECHOREPLY 0
#define ICMP_TIME_EXCEEDED 11
#define ICMP_DEST_UNREACHABLE 3

#define ARP_REQUEST 1
#define ARP_REPLY 2
#define ARPHRD_ETHER 1

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
