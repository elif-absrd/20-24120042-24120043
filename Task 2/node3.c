#include <stdio.h>
#include "dvrouter.h"

struct distance_table dt3;
int mincost3[NUM_NODES] = {7, INFINITY, 1, 0};

extern void tolayer2(struct rtpkt pkt);

void printdt3(struct distance_table *dtptr);

// This function sends node 3's current minimum cost vector to its directly connected neighbors
// We added this to modularize and reuse sending logic across initializations and updates
void send_costs_from_3() {
    struct rtpkt pkt;
    creatertpkt(&pkt, 3, 0, mincost3); // To node 0
    tolayer2(pkt);
    creatertpkt(&pkt, 3, 2, mincost3); // To node 2
    tolayer2(pkt);
    printf("Node 3 sent distance vector to nodes 0, 2 at time %f\n", clocktime);
}

void rtinit3() {
    for (int i = 0; i < NUM_NODES; i++)
        for (int j = 0; j < NUM_NODES; j++)
            dt3.cost[i][j] = INFINITY;

    dt3.cost[0][0] = 7;
    dt3.cost[2][2] = 1;
    dt3.cost[3][3] = 0;

    for (int i = 0; i < NUM_NODES; i++) {
        mincost3[i] = INFINITY;
        for (int j = 0; j < NUM_NODES; j++)
            if (dt3.cost[i][j] < mincost3[i])
                mincost3[i] = dt3.cost[i][j];
    }

    printf("rtinit3 called at time %f\n", clocktime);
    printdt3(&dt3);
    send_costs_from_3(); // Share initial distances with neighbors
}

void rtupdate3(struct rtpkt *rcvdpkt) {
    int src = rcvdpkt->sourceid;
    int updated = 0;

    printf("rtupdate3 called at time %f, received packet from node %d\n", clocktime, src);
    printf("Received mincost: [%d, %d, %d, %d]\n", rcvdpkt->mincost[0], rcvdpkt->mincost[1],
           rcvdpkt->mincost[2], rcvdpkt->mincost[3]);

    for (int i = 0; i < NUM_NODES; i++) {
        int new_cost = dt3.cost[src][src] + rcvdpkt->mincost[i];
        if (new_cost < 0 || new_cost > INFINITY) new_cost = INFINITY;
        if (new_cost < dt3.cost[i][src]) {
            dt3.cost[i][src] = new_cost;
            updated = 1;
        }
    }

    if (updated) {
        for (int i = 0; i < NUM_NODES; i++) {
            mincost3[i] = INFINITY;
            for (int j = 0; j < NUM_NODES; j++)
                if (dt3.cost[i][j] < mincost3[i])
                    mincost3[i] = dt3.cost[i][j];
        }
        printf("Distance table updated:\n");
        printdt3(&dt3);
        send_costs_from_3(); // Inform neighbors after table update
    } else {
        printf("Distance table unchanged:\n");
        printdt3(&dt3);
    }
}

void printdt3(struct distance_table *dtptr) {
    printf("                via     \n");
    printf("   D3 |    0     1     2     3 \n");
    printf("  ----|-----------------------\n");
    for (int i = 0; i < NUM_NODES; i++) {
        printf("dest %d|", i);
        for (int j = 0; j < NUM_NODES; j++) {
            if (dtptr->cost[i][j] < INFINITY)
                printf("  %3d", dtptr->cost[i][j]);
            else
                printf("  Inf");
        }
        printf("\n");
    }
}
