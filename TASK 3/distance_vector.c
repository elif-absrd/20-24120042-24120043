#include <stdio.h>
#include "dvrouter.h"

#define LINKCHANGES 1 // Set to 1 for extra credit, 0 for basic assignment

float clocktime = 0.0;

void creatertpkt(struct rtpkt *pkt, int srcid, int destid, int mincost[]) {
    pkt->sourceid = srcid;
    pkt->destid = destid;
    for (int i = 0; i < NUM_NODES; i++) {
        pkt->mincost[i] = mincost[i];
    }
}

void tolayer2(struct rtpkt pkt) {
    printf("Sending packet from %d to %d at time %f: [%d, %d, %d, %d]\n", 
           pkt.sourceid, pkt.destid, clocktime, 
           pkt.mincost[0], pkt.mincost[1], pkt.mincost[2], pkt.mincost[3]);
    // Stub: Real simulator would deliver this packet
}

// Extra credit part: Simulate changes in link cost between nodes during the simulation
void simulate_link_changes() {
    if (LINKCHANGES) {
        // At time 10000, we change the cost between node 0 and 1 from 1 to 20
        if (clocktime == 10000.0) {
            printf("Link change: 0-1 cost from 1 to 20 at time %f\n", clocktime);
            rtlinkhandler0(1, 20); // Inform node 0 about the cost change
            rtlinkhandler1(0, 20); // Inform node 1 about the cost change
        } 
        // At time 20000, we change it back from 20 to 1
        else if (clocktime == 20000.0) {
            printf("Link change: 0-1 cost from 20 to 1 at time %f\n", clocktime);
            rtlinkhandler0(1, 1);  // Reset node 0's link cost to node 1
            rtlinkhandler1(0, 1);  // Reset node 1's link cost to node 0
        }
    }
}

int main() {
    printf("Starting simulation with TRACE=2\n");

    rtinit0();
    rtinit1();
    rtinit2();
    rtinit3();

    struct rtpkt pkt;

    // Trigger update at time 1000: Node 0 receives packet from Node 1
    clocktime = 1000.0;
    creatertpkt(&pkt, 1, 0, (int[]){1, 0, 1, 2});
    rtupdate0(&pkt);

    // Simulate a link cost increase at time 10000
    clocktime = 10000.0;
    simulate_link_changes();

    // Simulate the link cost returning to normal at time 20000
    clocktime = 20000.0;
    simulate_link_changes();

    return 0;
}
