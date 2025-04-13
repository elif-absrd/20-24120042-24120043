#include <stdio.h>
#include "dvrouter.h"

struct distance_table dt1;
int mincost1[NUM_NODES] = {1, 0, 1, INFINITY};

extern void tolayer2(struct rtpkt pkt);

void printdt1(struct distance_table *dtptr);

void send_costs_from_1() {
    struct rtpkt pkt;
    creatertpkt(&pkt, 1, 0, mincost1); // To node 0
    tolayer2(pkt);
    creatertpkt(&pkt, 1, 2, mincost1); // To node 2
    tolayer2(pkt);
    printf("Node 1 sent distance vector to nodes 0, 2 at time %f\n", clocktime);
}

void rtinit1() {
    for (int i = 0; i < NUM_NODES; i++)
        for (int j = 0; j < NUM_NODES; j++)
            dt1.cost[i][j] = INFINITY;

    dt1.cost[0][0] = 1;
    dt1.cost[1][1] = 0;
    dt1.cost[2][2] = 1;

    for (int i = 0; i < NUM_NODES; i++) {
        mincost1[i] = INFINITY;
        for (int j = 0; j < NUM_NODES; j++)
            if (dt1.cost[i][j] < mincost1[i])
                mincost1[i] = dt1.cost[i][j];
    }

    printf("rtinit1 called at time %f\n", clocktime);
    printdt1(&dt1);
    send_costs_from_1();
}

void rtupdate1(struct rtpkt *rcvdpkt) {
    int src = rcvdpkt->sourceid;
    int updated = 0;

    printf("rtupdate1 called at time %f, received packet from node %d\n", clocktime, src);
    printf("Received mincost: [%d, %d, %d, %d]\n", rcvdpkt->mincost[0], rcvdpkt->mincost[1],
           rcvdpkt->mincost[2], rcvdpkt->mincost[3]);

    for (int i = 0; i < NUM_NODES; i++) {
        int new_cost = dt1.cost[src][src] + rcvdpkt->mincost[i];
        if (new_cost < 0 || new_cost > INFINITY) new_cost = INFINITY;
        if (new_cost < dt1.cost[i][src]) {
            dt1.cost[i][src] = new_cost;
            updated = 1;
        }
    }

    if (updated) {
        for (int i = 0; i < NUM_NODES; i++) {
            mincost1[i] = INFINITY;
            for (int j = 0; j < NUM_NODES; j++)
                if (dt1.cost[i][j] < mincost1[i])
                    mincost1[i] = dt1.cost[i][j];
        }
        printf("Distance table updated:\n");
        printdt1(&dt1);
        send_costs_from_1();
    } else {
        printf("Distance table unchanged:\n");
        printdt1(&dt1);
    }
}

// 🟡 This function was added to handle link cost changes dynamically — part of extra credit.
void rtlinkhandler1(int linkid, int newcost) {
    if (linkid != 0) return; // Node 1 only tracks cost changes with node 0

    printf("rtlinkhandler1 called at time %f, link to node %d changed to cost %d\n", 
           clocktime, linkid, newcost);

    int old_cost = dt1.cost[0][0];
    dt1.cost[0][0] = newcost;
    int updated = (old_cost != newcost);

    // Recalculate route costs if the link change affects overall paths
    for (int i = 0; i < NUM_NODES; i++) {
        if (i != 0) {
            int new_cost = dt1.cost[0][0] + dt1.cost[i][0];
            if (new_cost < 0 || new_cost > INFINITY) new_cost = INFINITY;
            if (new_cost < dt1.cost[i][0]) {
                dt1.cost[i][0] = new_cost;
                updated = 1;
            } else if (old_cost < newcost && dt1.cost[i][0] != INFINITY) {
                dt1.cost[i][0] = INFINITY;
                updated = 1;
            }
        }
    }

    // If changes happened, update mincost and send the new vector
    if (updated) {
        for (int i = 0; i < NUM_NODES; i++) {
            mincost1[i] = INFINITY;
            for (int j = 0; j < NUM_NODES; j++)
                if (dt1.cost[i][j] < mincost1[i])
                    mincost1[i] = dt1.cost[i][j];
        }
        printf("Distance table updated due to link change:\n");
        printdt1(&dt1);
        send_costs_from_1();
    }
}

void printdt1(struct distance_table *dtptr) {
    printf("                via     \n");
    printf("   D1 |    0     1     2     3 \n");
    printf("  ----|-----------------------\n");
    for (int i = 0; i < NUM_NODES; i++) {
        printf("dest %d|", i);
        for (int j = 0; j < NUM_NODES; j++) {
            if (dtptr->cost[i][j] < INFINITY) // Fixed typo
                printf("  %3d", dtptr->cost[i][j]);
            else
                printf("  Inf");
        }
        printf("\n");
    }
}
