#ifndef DVROUTER_H
#define DVROUTER_H

#define NUM_NODES 4
#define INFINITY 999

// Packet structure used to send distance vector updates
struct rtpkt {
    int sourceid;
    int destid;
    int mincost[NUM_NODES];
};

// Distance table used by each node to keep track of known costs
struct distance_table {
    int cost[NUM_NODES][NUM_NODES];
};

// Core utility function declarations
void creatertpkt(struct rtpkt *pkt, int srcid, int destid, int mincost[]);
void tolayer2(struct rtpkt pkt);
void printdt0(struct distance_table *dtptr);
void printdt1(struct distance_table *dtptr);
void printdt2(struct distance_table *dtptr);
void printdt3(struct distance_table *dtptr);

// Node-specific functions
void rtinit0(void);
void rtinit1(void);
void rtinit2(void);
void rtinit3(void);
void rtupdate0(struct rtpkt *rcvdpkt);
void rtupdate1(struct rtpkt *rcvdpkt);
void rtupdate2(struct rtpkt *rcvdpkt);
void rtupdate3(struct rtpkt *rcvdpkt);

// 🟡 These two handlers were added as part of the extra credit requirement
// They allow simulation of changing link costs at runtime
void rtlinkhandler0(int linkid, int newcost);
void rtlinkhandler1(int linkid, int newcost);

// 🟡 This was also added as part of extra credit to simulate time-based link changes
extern float clocktime;

#endif
