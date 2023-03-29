#ifndef IP_H
#define IP_H

#include <bits/stdc++.h>
#include <vector>
#include "lib.h"

using namespace std;

void read_router_config(char *filename, vector<struct route_table_entry> &route_table);

route_table_entry find_next_hop(uint32_t ip_addr, vector<struct route_table_entry> &route_table);
#endif // IP_H
