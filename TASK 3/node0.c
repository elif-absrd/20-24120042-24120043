#include <stdio.h>
#include "dvrouter.h"

struct distance_table dt0;
int mincost0[NUM_NODES] = {0, 1, 3, 7};

extern void tolayer2(struct rtpkt pkt);

void printdt0(struct distance_table *dtptr);

void send_costs_from_0() {
    struct rtpkt pkt;
    creatertpkt(&pkt, 0, 1, mincost0); // To node 1
    tolayer2(pkt);
    creatertpkt(&pkt, 0, 2, mincost0); // To node 2
    tolayer2(pkt);
    creatertpkt(&pkt, 0, 3, mincost0); // To node 3
    tolayer2(pkt);
    printf("Node 0 sent distance vector to nodes 1, 2, 3 at time %f\n", clocktime);
}

void rtinit0() {
    for (int i = 0; i < NUM_NODES; i++)
        for (int j = 0; j < NUM_NODES; j++)
            dt0.cost[i][j] = INFINITY;

    dt0.cost[0][0] = 0;
    dt0.cost[1][1] = 1;
    dt0.cost[2][2] = 3;
    dt0.cost[3][3] = 7;

    for (int i = 0; i < NUM_NODES; i++) {
        mincost0[i] = INFINITY;
        for (int j = 0; j < NUM_NODES; j++)
            if (dt0.cost[i][j] < mincost0[i])
                mincost0[i] = dt0.cost[i][j];
    }

    printf("rtinit0 called at time %f\n", clocktime);
    printdt0(&dt0);
    send_costs_from_0();
}

void rtupdate0(struct rtpkt *rcvdpkt) {
    int src = rcvdpkt->sourceid;
    int updated = 0;

    printf("rtupdate0 called at time %f, received packet from node %d\n", clocktime, src);
    printf("Received mincost: [%d, %d, %d, %d]\n", rcvdpkt->mincost[0], rcvdpkt->mincost[1],
           rcvdpkt->mincost[2], rcvdpkt->mincost[3]);

    for (int i = 0; i < NUM_NODES; i++) {
        int new_cost = dt0.cost[src][src] + rcvdpkt->mincost[i];
        if (new_cost < 0 || new_cost > INFINITY) new_cost = INFINITY;
        if (new_cost < dt0.cost[i][src]) {
            dt0.cost[i][src] = new_cost;
            updated = 1;
        }
    }

    if (updated) {
        for (int i = 0; i < NUM_NODES; i++) {
            mincost0[i] = INFINITY;
            for (int j = 0; j < NUM_NODES; j++)
                if (dt0.cost[i][j] < mincost0[i])
                    mincost0[i] = dt0.cost[i][j];
        }
        printf("Distance table updated:\n");
        printdt0(&dt0);
        send_costs_from_0();
    } else {
        printf("Distance table unchanged:\n");
        printdt0(&dt0);
    }
}

// 🟡 This handler was added for extra credit. It handles dynamic link cost changes for node 0.
void rtlinkhandler0(int linkid, int newcost) {
    if (linkid != 1) return;

    printf("rtlinkhandler0 called at time %f, link to node %d changed to cost %d\n", 
           clocktime, linkid, newcost);

    int old_cost = dt0.cost[1][1];
    dt0.cost[1][1] = newcost;
    int updated = (old_cost != newcost);

    for (int i = 0; i < NUM_NODES; i++) {
        if (i != 1) {
            // Recalculate cost using updated link
            int new_cost = dt0.cost[1][1] + dt0.cost[i][1];
            if (new_cost < 0 || new_cost > INFINITY) new_cost = INFINITY;
            if (new_cost < dt0.cost[i][1]) {
                dt0.cost[i][1] = new_cost;
                updated = 1;
            } else if (old_cost < newcost && dt0.cost[i][1] != INFINITY) {
                // If cost worsens, consider removing the path
                dt0.cost[i][1] = INFINITY;
                updated = 1;
            }
        }
    }

    if (updated) {
        for (int i = 0; i < NUM_NODES; i++) {
            mincost0[i] = INFINITY;
            for (int j = 0; j < NUM_NODES; j++)
                if (dt0.cost[i][j] < mincost0[i])
                    mincost0[i] = dt0.cost[i][j];
        }
        printf("Distance table updated due to link change:\n");
        printdt0(&dt0);
        send_costs_from_0();
    }
}

void printdt0(struct distance_table *dtptr) {
    printf("                via     \n");
    printf("   D0 |    0     1     2     3 \n");
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
