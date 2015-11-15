/*
* DSR.h
*
* Created: 5/20/2014 5:30:34 PM
*  Author: Tanmay Sane
*/

#include "Xbee.h"
#define F_CPU 16000000UL
#include <util/delay.h>
#ifndef DSR_H_
#define DSR_H_

#define EXP_TIME 5  //5 rd, ca-> 200
#define MAX_COUNT 65535
#define MAX_C_LEN 10 // Maximum No of routes that can be cached
#define MAX_C_HOP 5 // Maximum No of hops per route that can be cached
// Data Structure

typedef struct{
	char identifier;	// packet type (custom for our code)
	int initiator;
	int target;
	char unique_id;
	char expiration_period;
	int route_record_length;
	int dataLen;	// holds length of data array
	int route_record[10];
	char data[20];	// packet data
} DSRpacket;

//Route Cache Entry
typedef struct {
	int route_des_node;
	int route_r_len;
	int route_r_up[10];
	char r_expir_id;
	char uni_pkt_id;
} route_r_entry;


typedef struct {
	
	int dest_node[MAX_C_LEN];
	int r_len[MAX_C_LEN];
	int cached_route[MAX_C_HOP][MAX_C_LEN];
	char expir_id[MAX_C_LEN];
	char uni_id[MAX_C_LEN];
	
} route_cache;
route_cache rc_ptr;

//Unique ID stack

typedef struct
{
	int initi_add[10];
	char reqID[10];
}
recent_seen_reqs;

typedef struct
{
	char route_array[10];
}
route_rec;

// Function Prototypes
void append_route(DSRpacket,int);
DSRpacket append_route_r (DSRpacket ,int);
void updated_TX_DSR_Packet(DSRpacket,int,int);
int char_to_int(char ,char);
long char_to_long(char,char,char,char);
DSRpacket detach_route(DSRpacket );
DSRpacket detach_route_ptr(DSRpacket );
int read_last_entry_on_r_rec(DSRpacket);
int read_last_entry_ptr(DSRpacket);
void app_seen_list_at_curr(DSRpacket);
int Transmit_DSR(int ,char *,int) ; //
char calculate_uniq_id(addr64);
int check_dis_r_validity(route_r_entry );
int status_ack();
void initDSR(void);
void newPkt(void);
void setDSRCB(void (*funptr)(void));
void setCoord(void);
void wait_init(int);
void wait_sec(int);
/*MSB to LSB, enter MSB First */
DSRpacket getDSRpkt(RxPacket);
DSRpacket createDSRpkt(char , int  , int , char , char ,int ,int , int* ,char* );
void printDSR_Packet(DSRpacket);//How to pass value to it;



#endif /* DSR_H_ */