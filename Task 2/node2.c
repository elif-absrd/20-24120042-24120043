#include <stdio.h>
#include "dvrouter.h"

struct distance_table dt2;
int mincost2[NUM_NODES] = {3, 1, 0, 1};

extern void tolayer2(struct rtpkt pkt);

void printdt2(struct distance_table *dtptr);

// This function sends the updated distance vector from node 2 to its direct neighbors
// We added this to modularize sending logic and reuse it after updates
void send_costs_from_2() {
    struct rtpkt pkt;
    creatertpkt(&pkt, 2, 0, mincost2); // To node 0
    tolayer2(pkt);
    creatertpkt(&pkt, 2, 1, mincost2); // To node 1
    tolayer2(pkt);
    creatertpkt(&pkt, 2, 3, mincost2); // To node 3
    tolayer2(pkt);
    printf("Node 2 sent distance vector to nodes 0, 1, 3 at time %f\n", clocktime);
}

void rtinit2() {
    for (int i = 0; i < NUM_NODES; i++)
        for (int j = 0; j < NUM_NODES; j++)
            dt2.cost[i][j] = INFINITY;

    dt2.cost[0][0] = 3;
    dt2.cost[1][1] = 1;
    dt2.cost[2][2] = 0;
    dt2.cost[3][3] = 1;

    for (int i = 0; i < NUM_NODES; i++) {
        mincost2[i] = INFINITY;
        for (int j = 0; j < NUM_NODES; j++)
            if (dt2.cost[i][j] < mincost2[i])
                mincost2[i] = dt2.cost[i][j];
    }

    printf("rtinit2 called at time %f\n", clocktime);
    printdt2(&dt2);
    send_costs_from_2();  // Called after initialization to share initial costs
}

void rtupdate2(struct rtpkt *rcvdpkt) {
    int src = rcvdpkt->sourceid;
    int updated = 0;

    printf("rtupdate2 called at time %f, received packet from node %d\n", clocktime, src);
    printf("Received mincost: [%d, %d, %d, %d]\n", rcvdpkt->mincost[0], rcvdpkt->mincost[1],
           rcvdpkt->mincost[2], rcvdpkt->mincost[3]);

    for (int i = 0; i < NUM_NODES; i++) {
        int new_cost = dt2.cost[src][src] + rcvdpkt->mincost[i];
        if (new_cost < 0 || new_cost > INFINITY) new_cost = INFINITY;
        if (new_cost < dt2.cost[i][src]) {
            dt2.cost[i][src] = new_cost;
            updated = 1;
        }
    }

    if (updated) {
        for (int i = 0; i < NUM_NODES; i++) {
            mincost2[i] = INFINITY;
            for (int j = 0; j < NUM_NODES; j++)
                if (dt2.cost[i][j] < mincost2[i])
                    mincost2[i] = dt2.cost[i][j];
        }
        printf("Distance table updated:\n");
        printdt2(&dt2);
        send_costs_from_2(); // Re-sends updated vector if the table changed
    } else {
        printf("Distance table unchanged:\n");
        printdt2(&dt2);
    }
}

void printdt2(struct distance_table *dtptr) {
    printf("                via     \n");
    printf("   D2 |    0     1     2     3 \n");
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