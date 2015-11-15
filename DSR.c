/*
* DSR.c
*
* Created: 9/18/2014 5:30:21 PM
*  Author: Tanmay Sane
*/
#include "DSR.h"
#include "DSR_HAL.h"

//  Declarations
int wait_for_route=0;		//Route wait flag
int cache_length=0;			//Route cache length
int wait_for_ack=0;			//Ack wait flag
uint16_t tick = 0;			//timer declaration
int coord =0;
char Net_ID=0x00;
addr64 self_node_addr; //64 bit address
int Self_Net_add=0;
long data_l=0;
long data_h=0;
int cb;	// callback flag -- set if a callback function for new packets has been specified
void (*cbFunPtr)(void);	// callback function pointer
char send_NetID_data[9]={'0'};//address+ net ID for setup phase
int tanmay_c=0;
int tanmay_c2=0;

//Route Cache Entry
route_r_entry discovered_route;

/*Route Functions */

/*Route Discovery */
route_r_entry route_discovery(DSRpacket pkt_1,char ident_f,int destin)
{
	int destination= destin;
	route_r_entry empty_r;
	empty_r.route_r_len=0;
	pkt_1.initiator=Self_Net_add;
	pkt_1.unique_id=calculate_uniq_id(self_node_addr);
	app_seen_list_at_curr(pkt_1);
	if (ident_f==0xF2)
	{
		pkt_1.expiration_period=0; // Intitialize hop count
	}
	
	
	pkt_1.target=destination;
	updated_TX_DSR_Packet(pkt_1,0xFFFF,ident_f);
	// wait till the route is discovered.
	wait_for_route=1;
	int wait_r_v_count;
	wait_r_v_count=0;
	while(wait_for_route==1)
	{
		wait_init(4);
		if(wait_for_route==0)
		{
			break;
		}
		if(wait_r_v_count==1){return empty_r;}
		wait_r_v_count++;
	}
	return discovered_route;
}

/*Timer functions*/

int max_ticks=0;

void timerEvent()
{
	tick++;
	if(tick>65535){tick=0;}
	if(tick<0){tick=0;}
}

void printTick()
{
	printf("Tick = %ud",tick);
}

/*Unique ID Stack functions*/
int seen_list_len=0;
recent_seen_reqs seen_req;
int search_seen_list(int initiator_node,char uni_reqt_id)
{
	for(int i=0;i<seen_list_len;i++){
		if((seen_req.initi_add[i]==initiator_node)&&(seen_req.reqID[i]==uni_reqt_id))
		{
			#ifdef DEBUG1
			printf("Seen Req at=%x",i);
			#endif
			return 1;
		}
	}
	#ifdef DEBUG1
	printf("\nMatch Not Found for Seen");
	#endif
	return 0;
} //end of search function

//clear
void clear_recent_seen_reqs()
{ for (int i=0;i<=seen_list_len;i++)
	{
		seen_req.initi_add[i]=0;
		seen_req.reqID[i]=0;
	}
}
//append
void app_seen_list_at_curr(DSRpacket rec_pkt)
{
	seen_req.initi_add[seen_list_len]=rec_pkt.initiator;
	seen_req.reqID[seen_list_len]=rec_pkt.unique_id;
	seen_list_len++;
	if(seen_list_len==9)
	{
		clear_recent_seen_reqs();
	}
}

int print_seen_list()
{  printf("\n***Seen List***");
	for(int i=0;i<seen_list_len;i++)
	{
		printf("\n Initiator=%d.ReqID=%d",seen_req.initi_add[i],seen_req.reqID[i]);
	}
	printf("\n**********");
	return 0;
}

/**** Unique ID calculation & assignment to packet ***/
char calculate_uniq_id(addr64 self_address)
{
	/*self_address.sl0*/
	long uniq_id;
	long val=(self_address.sl)<<24;
	uniq_id=(val);
	uniq_id=uniq_id>>24;
	int l_u= (int)uniq_id;
	//l_u=l_u;
	char c_u=(char)l_u;
	l_u=(int)c_u;
	int j;
	srand( (unsigned)l_u);
	j = rand() % 256;
	c_u=(char) j;
	return c_u;
}

/*Route cache */
//*************************** Route functions ************************************

int app_route_cache_at_curr(DSRpacket rec_pkt1)
{
	//for cache length=0 this function works as update at a given location
	if(cache_length==0)
	{
		rc_ptr.dest_node[0]=rec_pkt1.target;
		rc_ptr.r_len[0]=rec_pkt1.route_record_length;
		rc_ptr.uni_id[0]=rec_pkt1.unique_id;
		rc_ptr.expir_id[0]=rec_pkt1.expiration_period;
		//printf("\nroute_length=%d",rc_ptr.r_len);
		for(int j=0;j<rc_ptr.r_len[0];j++)
		{
			rc_ptr.cached_route[0][j]=rec_pkt1.data[j];
		}
	}
	else
	{
		rc_ptr.dest_node[cache_length]=rec_pkt1.target;
		rc_ptr.r_len[cache_length]=rec_pkt1.route_record_length;
		rc_ptr.uni_id[cache_length]=rec_pkt1.unique_id;
		rc_ptr.expir_id[cache_length]=rec_pkt1.expiration_period;
		for(int j=0;j<rc_ptr.r_len[cache_length];j++)
		{
			rc_ptr.cached_route[cache_length][j]=rec_pkt1.route_record[j];
		}
	}

	cache_length++;
	#ifdef DEBUG1
	printf("cache_l= %d",cache_length);
	#endif
	return 1;//-1
}

//this function should return pointer of the found array
int search_route_cache(int desired_node)
{
	for(int i=0;i<cache_length;i++){
		if(rc_ptr.dest_node[i]==desired_node)
		{
			#ifdef DEBUG1
			printf("MF@=%x\n",i);
			#endif
			return i;
		}
	}
	#ifdef DEBUG1
	printf("MNF!");
	#endif
	return 1000;
} //end of search function

int print_route_cache()
{  printf("\n      *PRoute Cache*     ");
	
	for(int i=0;i<cache_length;i++){
		//printf("Dest Node:  Route_length           Route             UniID   Exp_id");
		printf("\n");
		printf("Dest_Node=%d ",rc_ptr.dest_node[i]);
		printf("Route_len=%d ",rc_ptr.r_len[i]);
		printf("Uni Id=%d ",rc_ptr.uni_id[i]);
		printf("Exp_Id=%d ",rc_ptr.expir_id[i]);
		printf("Cached Route =");
		for(int j=0;j<rc_ptr.r_len[i];j++)
		{
		printf("%x,",rc_ptr.cached_route[i][j]);}
	}
	printf("\n");
	return 0;
}

int update_route_cache(DSRpacket rec_pkt,int location)
{
	rc_ptr.dest_node[location]=rec_pkt.target;
	rc_ptr.r_len[location]=rec_pkt.route_record_length;
	rc_ptr.uni_id[location]=rec_pkt.unique_id;
	rc_ptr.expir_id[location]=rec_pkt.expiration_period;
	for(int j=0;j<rc_ptr.r_len[location];j++)
	{
		rc_ptr.cached_route[location][j]=rec_pkt.route_record[j];
	}
	#ifdef DEBUG1
	printf("%d,%d,%d,%d",rc_ptr.dest_node[location],rc_ptr.r_len[location],rc_ptr.uni_id[location],rc_ptr.expir_id[location]);
	for(int j=0;j<rc_ptr.r_len[location];j++)
	{		printf("\n[%d][%d]=%d",location,j,rc_ptr.cached_route[location][j]);
	}
	printf("/UDone");
	#endif
	return 0;
}

int clear_route_cache(route_cache rc_ptr,int location)
{	rc_ptr.dest_node[location]=0;
	rc_ptr.r_len[location]=0;
	rc_ptr.uni_id[location]=0;
	rc_ptr.expir_id[location]=0;
	for(int j=0;j<rc_ptr.r_len[location];j++)
	{
		rc_ptr.cached_route[location][j]=0;
	}
	return 0;
}

int check_dis_r_validity(route_r_entry route_test)
{
	if(route_test.route_r_len==0)
	{
		return 0; //empty pkt
	}
	return 1; // dis pkt
}

//**************** End of Route Functions ******************************

//**************************** DSR Packet Functions****************************//
/*DSR Packet functions*/

//prep pkt
DSRpacket add_64_to_DSRpkt(int identifr,int d_target)
{
	DSRpacket temp_pkt;
	char buffr_address[8]={};
	/*The address is extracted*/
	long value = self_node_addr.sh;
	buffr_address[0]=(value>>24);
	buffr_address[1]=(value>>16);
	buffr_address[2]=(value>>8);
	buffr_address[3]=value;
	value = self_node_addr.sl;
	buffr_address[4]=(value>>24);
	buffr_address[5]=(value>>16);
	buffr_address[6]=(value>>8);
	buffr_address[7]=value;
	temp_pkt=createDSRpkt(identifr,Self_Net_add,d_target,0,0,0,8,0,buffr_address);
	return temp_pkt;
}

DSRpacket createDSRpkt(char Identifier, int Initiator , int Target, char Unique_id, char Expiration_period,int Route_record_length,int DataLen, int* Route_record,char* Data)
{
	DSRpacket pkt;
	
	int count = 0;
	pkt.identifier=Identifier;
	pkt.initiator=Initiator;
	pkt.target=Target;
	pkt.unique_id=Unique_id;
	pkt.expiration_period=Expiration_period;
	pkt.route_record_length=Route_record_length;
	pkt.dataLen=DataLen;	// holds length of data array
	
	#ifdef DEBUG1
	printf("Identifier = %x ", pkt.identifier);
	printf("Initiator = %d ", pkt.initiator);
	printf("Target = %d ", pkt.target);
	printf("Unique Id = %d ", pkt.unique_id);
	printf("expiration+period = %d ", pkt.expiration_period);
	printf("route_rec_length=%d",pkt.route_record_length);
	printf("data len = %d ", pkt.dataLen);
	#endif
	
	for(count=0;count < DataLen; count++)
	{pkt.data[count] = Data[count];
		#ifdef DEBUG1
		printf("data = %x,%d ", Data[count],count);
		#endif
	}
	for(count=0;count < Route_record_length; count++)
	{pkt.route_record[count] = Route_record[count];
		#ifdef DEBUG1
		printf("route =%d,%x ",count, Route_record[count]);
		#endif
	}
	return pkt;
}


/* This function prints the DSR packet passed to it*/
void printDSR_Packet(DSRpacket pkt )
{
	printf("%d%d%d%x",pkt.identifier,pkt.initiator,pkt.target,pkt.unique_id);
	printf("%d%d%x",pkt.route_record_length,pkt.dataLen,pkt.expiration_period);
	for(int i=0;i<pkt.route_record_length;i++)
	{printf("%d",pkt.route_record[i]);
	}
	for(int i=0;i<pkt.dataLen;i++)
	{printf("%d",pkt.data[i]);
	}
}

DSRpacket append_route_r (DSRpacket r_packet,int net_id_update)
{
	DSRpacket new_pkt;
	new_pkt= r_packet;
	new_pkt.route_record[new_pkt.route_record_length]=net_id_update;
	new_pkt.route_record_length++;
	#ifdef DEBUG1
	printf("\nRoute_len=%d",r_packet.route_record_length);
	for(int i =0; i<r_packet.route_record_length;i++)
	{
		printf("Route Record [%d] = %d\n", i, r_packet.route_record[i]);
	}
	#endif
	return new_pkt;
}

DSRpacket detach_route(DSRpacket r_packet)
{
	r_packet.route_record[(r_packet.route_record_length)-1]='\0';
	r_packet.route_record_length=(r_packet.route_record_length)-1;
	return r_packet;
}

DSRpacket detach_route_ptr(DSRpacket r_packet)
{
	//r_packet.route_record[(r_packet.route_record_length)-1]='\0';
	r_packet.expiration_period=(r_packet.expiration_period)-1;  // 3--> 2
	return r_packet;
}

void updated_TX_DSR_Packet(DSRpacket r_packet1,int netid_dest_node,int indentifier)
{
	char buff[7+(r_packet1.route_record_length*2)+r_packet1.dataLen ];	//temporary buffer for Tx. [fixed + variable-ROUTE_length +variable-data_length]
	// Transmit DSR packet Identifier
	buff[0] = indentifier;//r_packet1.identifier;
	buff[1] = r_packet1.initiator;
	buff[2] = r_packet1.target;
	buff[3] = r_packet1.unique_id;
	buff[4] = r_packet1.expiration_period; // QUery: Dont know if we are gonna send this as this is intrinsic to the mote
	buff[5] = r_packet1.route_record_length;
	buff[6] = r_packet1.dataLen;
	int j = 0;
	for(int i=0;i<r_packet1.route_record_length;i++ )
	{
		buff[7+j] = (r_packet1.route_record[i]>>8);
		buff[7+j+1] = r_packet1.route_record[i];
		j = j + 2;
	}
	for(int i=0;i<r_packet1.dataLen;i++ )
	buff[7+(r_packet1.route_record_length*2)+i] = r_packet1.data[i];
	TX_Request_16bit(0x00,netid_dest_node,0x01,buff, 7+r_packet1.dataLen+(r_packet1.route_record_length*2));
}

//this is the network id
int read_last_entry_on_r_rec(DSRpacket r_packet2)
{
	return r_packet2.route_record[(r_packet2.route_record_length)-1];
}
//This is the network id
int read_last_entry_ptr(DSRpacket r_packet2)
{
	return r_packet2.route_record[(r_packet2.expiration_period)-1];
}

DSRpacket getDSRpkt(RxPacket rx_packet)
{
	int count;
	DSRpacket rxPkt;
	for(count=0;count<rx_packet.data_len;count++)
	{
		if(count == 0)
		rxPkt.identifier = rx_packet.data[count];
		else if(count == 1)
		rxPkt.initiator = rx_packet.data[count];
		else if(count == 2)
		rxPkt.target = rx_packet.data[count];
		else if(count == 3)
		rxPkt.unique_id = rx_packet.data[count];
		else if(count == 4)
		rxPkt.expiration_period = rx_packet.data[count];
		else if(count == 5)
		rxPkt.route_record_length = rx_packet.data[count];
		else if(count == 6)
		{
			rxPkt.dataLen = rx_packet.data[count];
		}
		else if(count == 7)
		{
			int j=0;
			for(count=7;count < 7+(rxPkt.route_record_length*2); count=count+2)
			{
				rxPkt.route_record[j] = char_to_int(rx_packet.data[count],rx_packet.data[count+1]) ;
				j=j+1;
			}
			
			if(count == 7+ (rxPkt.route_record_length*2))
			{
				for(count=7+(rxPkt.route_record_length*2);count < 7+(rxPkt.route_record_length*2)+rxPkt.dataLen; count++)
				{
					rxPkt.data[count-(7+(rxPkt.route_record_length*2))] = rx_packet.data[count];
					#ifdef DEBUG1
					printf("rxPkt.data[%d]=%x,%d",count-(7+(rxPkt.route_record_length*2)),rx_packet.data[count],count);
					#endif
				}
			}
		}
	} //end of for loop
	return rxPkt;
}

void initDSR()
{
	//int xb;
	//xb=XbeeSetup(); // configure xbee settings
	initTimer();
	cb = 0;
	setNewPacketCB(newPkt);
	//Get 64 bit address.
	self_node_addr=get64bitAddress();
	coord=0;
	set16bitAddress(2);
	_delay_ms(250);
	Self_Net_add=get16bitAddress();
	#ifdef DEBUG1
	printf("\t%d",Self_Net_add);
	#endif
}

void newPkt()
{   printf("*");
	
	RxPacket packet = getPacket(); // RxPacket is the XBee packet
	if(packet.data[0] == 0xF2 || packet.data[0] == 0xF3 || packet.data[0] == 0xF6||packet.data[0] == 0xF7)
	{
		// New Packet code here:
		DSRpacket pkt = getDSRpkt(packet);
		switch(pkt.identifier)
		{
			case 0xF2:
			printf("RDP");
			// Unamed node
			if((Self_Net_add==0) && (coord==0)) { printf("\nI cant help in routing");	}
			////Named Node
			else{
				//Check if heard
				int heard_recent_request=search_seen_list(pkt.initiator,pkt.unique_id);
				if((heard_recent_request==1)||(pkt.initiator==Self_Net_add)||(pkt.expiration_period>9))  //heard or hop count excedded
				{
					if(Self_Net_add!=pkt.initiator)
					{      printf("heard"); 			}
				}
				else    // Not heard
				{	app_seen_list_at_curr(pkt); //Seen List Update
					pkt.expiration_period=pkt.expiration_period+1; //increment hop count
					DSRpacket tx_Req_pkt;
					tx_Req_pkt=pkt;
					tx_Req_pkt=append_route_r(pkt,Self_Net_add); // Append existing route
					
					if(tx_Req_pkt.target==Self_Net_add) //target
					{	printf("T_f");
						////Create Route_Reply_Pkt
						tx_Req_pkt.dataLen=9+(tx_Req_pkt.route_record_length*2); //8+ net id + current route length
						tx_Req_pkt.data[8]=0xFF;
						int j = 9;
						//Route saved in data section
						for(int i=0;i<tx_Req_pkt.route_record_length;i++)
						{
							tx_Req_pkt.data[j]=(tx_Req_pkt.route_record[i]>>8);
							tx_Req_pkt.data[j+1] = (tx_Req_pkt.route_record[i]);
							j = j+2;
						}
						////Remove E, i.e existing node
						tx_Req_pkt=detach_route(tx_Req_pkt);
						
						if(tx_Req_pkt.route_record_length ==0)
						{
							updated_TX_DSR_Packet(tx_Req_pkt,tx_Req_pkt.initiator,0xF3);//Send reply pkt
						}
						if(tx_Req_pkt.route_record_length !=0)
						{
							int last_entry_r_rec= read_last_entry_on_r_rec(tx_Req_pkt); //Read D
							printf("\nlast_entry_r_rec=%x",last_entry_r_rec);
							tx_Req_pkt=detach_route(tx_Req_pkt);//Remove D
							updated_TX_DSR_Packet(tx_Req_pkt,last_entry_r_rec,0xF3);//Send reply pkt
						}
					}
					else
					{
						if(Self_Net_add==3)
						{
							_delay_ms(8000);
							_delay_ms(2000);
						}
						printf("P_n"); //Appending has been done already
						printf("Forward pkt");
						updated_TX_DSR_Packet(tx_Req_pkt,0xFFFF,0xF2);
					} //pathnode
				}
			}
			break;
			case 0xF3:
			#ifdef DEGUG1
			printf("Route Reply PKT");
			#endif
			if((Self_Net_add==0)&& (coord==0)) // Unamed node
			{	printf("\nI cant help in routing");
			}
			else{
				//Extract address
				data_h=char_to_long(pkt.data[0],pkt.data[1],pkt.data[2],pkt.data[3]);
				data_l=char_to_long(pkt.data[4],pkt.data[5],pkt.data[6],pkt.data[7]);
				//Match
				if((Self_Net_add==pkt.initiator)||((data_h==self_node_addr.sh)&&(data_l==self_node_addr.sl)))
				{	discovered_route.route_r_len=(pkt.dataLen-9)/2; //Verified
					//Read Route from Data section & extract it.
					int j=0;
					for(int i=0;i<(discovered_route.route_r_len);i++)// 0-7 address, 8 - net id, 9-[datalen] -routes
					{	discovered_route.route_r_up[i]=char_to_int(pkt.data[9+j],pkt.data[10+j]);
						j=j+2;
					}
					discovered_route.r_expir_id=pkt.expiration_period;
					discovered_route.uni_pkt_id=pkt.unique_id;
					discovered_route.route_des_node=pkt.target;
					#ifdef DEBUG1
					printf("\n Initiator Found");
					printf(" discovered_route.route_r_len=%d", discovered_route.route_r_len);
					for(int i=0;i<(discovered_route.route_r_len);i++)// 0-7 address, 8 - net id, 9-[datalen] -routes
					{
						printf("\nd_r[%d]=%d",i,discovered_route.route_r_up[i]);
					}
					printf("\n");
					#endif
					//copy  discovered_route to r_d_pkt
					DSRpacket r_d_pkt;
					r_d_pkt.route_record_length=discovered_route.route_r_len;
					for(int i=0;i<r_d_pkt.route_record_length;i++)// 0-7 address, 8 - NET ID, 9-[dataLen] -routes
					{		r_d_pkt.data[i]=discovered_route.route_r_up[i];
						#ifdef DEBUG1
						printf("\n r_d_pkt.data[%d]=%d",i,r_d_pkt.data[i]);
						#endif
					}
					r_d_pkt.unique_id= discovered_route.uni_pkt_id;
					r_d_pkt.target=discovered_route.route_des_node;
					r_d_pkt.expiration_period=discovered_route.r_expir_id;
					
					int search_res=search_route_cache(pkt.target);
					if(search_res<1000)   //entry found
					{	update_route_cache(r_d_pkt,search_res);
					}
					else//cached entry not found
					{app_route_cache_at_curr(r_d_pkt);
					}
					wait_for_route=0;
					_delay_ms(1);
				}
				//No match found,all nodes here are the path nodes
				else
				{   //Path Node
					DSRpacket temp_1;
					temp_1=pkt;
					int searched_destination=pkt.target;
					int route_exist=search_route_cache(searched_destination);
					if(route_exist>=0)
					{	update_route_cache(temp_1,route_exist);
					}
					else
					{  //pkt received at D
						app_route_cache_at_curr(temp_1);  //saved route in cacahe table
					}
					if(temp_1.route_record_length==0)
					{	printf("\nRL0");
						updated_TX_DSR_Packet(pkt,temp_1.initiator,0xF3);
					}
					else
					{	int read_entry_l=read_last_entry_on_r_rec(pkt); //Normal approach Before this step pkt has ABC,
						pkt=detach_route(pkt);                    //packet should now go to C-detach C
						updated_TX_DSR_Packet(pkt,read_entry_l,0xF3); //TX updated pkt
					}
				}
			}
			break;
			
			case 0xF6: //Message packet
			if((Self_Net_add==0) && (coord==0)) { printf("\nI cant help in routing");	}
			//Named Node
			else
			{	if(Self_Net_add==pkt.target)
				{
					printf("T_F");
					//Extract Pkt
					DSRpacket rep_pkt;
					//Fill Route--Invert route
					rep_pkt=pkt;
					rep_pkt.initiator=pkt.initiator;
					rep_pkt.target=pkt.target;
					rep_pkt.route_record_length=pkt.route_record_length;
					rep_pkt.unique_id=pkt.unique_id;
					//invert route
					for (int i=0; i<pkt.route_record_length;i++)   // 401 104
					{	rep_pkt.route_record[i]=pkt.route_record[(pkt.route_record_length-1)-i];
						printf("[%d]=%d",i,rep_pkt.route_record[i]);
					}
					//rep_pkt has 1,0,4
					rep_pkt.expiration_period=pkt.route_record_length;// re-initialize 3 , now pointing to 4
					rep_pkt=detach_route_ptr(rep_pkt); // detach existing target node  //Remove 4 // curr_value-->2
					printf("exp=%d",rep_pkt.expiration_period);
					
					if(rep_pkt.expiration_period==0)
					{
						updated_TX_DSR_Packet(rep_pkt,rep_pkt.initiator,0xF7);
					}
					if(rep_pkt.expiration_period!=0)
					{
						int l_ent=read_last_entry_ptr(rep_pkt);
						printf("Se=%d",l_ent);
						rep_pkt=detach_route_ptr(rep_pkt);  //	 curr_value -->1 send to 0
						updated_TX_DSR_Packet(rep_pkt,l_ent,0xF7);
					}
				}
				else
				{
					printf("T_N _F %d",pkt.route_record[(pkt.expiration_period)-1]);
					if(pkt.expiration_period==0)
					{
						updated_TX_DSR_Packet(pkt,pkt.target,0xF6); // exp transmitted is zero
					}
					else
					{   int l_ent=read_last_entry_ptr(pkt); //Read 0
						pkt=detach_route_ptr(pkt); // current value 0
						printf("Se=%d",l_ent);
						updated_TX_DSR_Packet(pkt,l_ent,0xF6); //send to 0
					}
				}
			}
			break;
			case 0XF7:
			if((Self_Net_add==0) && (coord==0)) { printf("\nI cant help in routing");	}
			//Named Node
			else
			{
				if(Self_Net_add==pkt.initiator)
				{    _delay_ms(1);
					printf("I_F");
					wait_for_ack=0;
					//while(1){printf("I_F");}
				}
				else
				{
					printf("Target N _F");
					if(pkt.expiration_period==0)
					{	updated_TX_DSR_Packet(pkt,pkt.initiator,0xF7);
					}
					else
					{	int l_ent=read_last_entry_ptr(pkt);
						pkt=detach_route_ptr(pkt);
						printf("Se=%d",l_ent);
						updated_TX_DSR_Packet(pkt,l_ent,0xF7); //TX updated pkt
					}
				}
			}
			break;
			default:
			break;
		}  //end switch case
	}
} // End functions

/* setDSRCB()
*
* Application code for setting new packet callback functions using DSR
*/
void setDSRCB(void (*funptr)(void))
{
	cb = 1;
	cbFunPtr = funptr;
}


void setCoord()
{
	/* check for other coords (implement later) */
	coord = 1;
	#ifdef DEBUG1
	printf("\nI am the co-od\n");
	#endif
}

void wait_init(int wait_time)  // No of sec
{
	for(int i=0;i<wait_time;i++)
	{
		for(int i=0;i<1000;i++) //1 sec
		{
			_delay_ms(10);
		}
	}
}

int char_to_int(char a51 ,char a61)   //j+1,j
{
	int a8= (int)(a61);
	int a9= (int)((long)(a51));
	int a10= (int)((a9<<8)|(a8));
	return a10;
}

long char_to_long(char a41,char a51 ,char a61,char a71)
{
	long a8= (long)(((long)(a71))|((long)(a61)<<8));
	long a9= (long)(((long)(a51))|((long)(a41)<<8));
	long a10= (long)((a9<<16)|(a8));
	return a10;
}

void wait_sec(int wait_time)
{
	int scale;
	scale=100;
	int pre_scale;
	pre_scale=2;
	wait_time=pre_scale*(scale*wait_time);
	for (int i=0;i<wait_time;i++)
	{
		_delay_ms(10);
	}
}

// Ack wait & reporting
int status_ack()
{
	wait_for_ack=1;
	int timeout;
	timeout=0;
	while(wait_for_ack==1)
	{
		wait_init(4);
		if(wait_for_ack==0)
		{
			break;
		}
		if(timeout==1){return 0;}
		timeout++;
	}
	return 1; //ACK Success
}
//Transmit DSR
int Transmit_DSR(int Target,char *msg1,int msg1_size)
{
	int ms_len=0;
	ms_len=msg1_size;
	DSRpacket f_pkt;
	//Create packet for tx using F6
	f_pkt=createDSRpkt(0xF6,Self_Net_add,Target,0,0,0,0,0,0);
	f_pkt.dataLen=ms_len;
	route_r_entry route_obtained;
	//Pre Cached Route
	rc_ptr.r_len[0]=2;
	rc_ptr.cached_route[0][0]=1;
	rc_ptr.cached_route[0][1]=0;
	rc_ptr.expir_id[0]=10;
	rc_ptr.dest_node[0]=0;
	cache_length++;
	int s_1=search_route_cache(Target);
	
	if(s_1<1000)
	{
		// Read
		if(tick<rc_ptr.expir_id[s_1])
		{  if (((tick+MAX_COUNT)-rc_ptr.expir_id[s_1])>EXP_TIME)
			{ // Greater, RD
				printf("RD1");
				wait_for_route=1;
				wait_init(2);
				DSRpacket rd_pkt1 =add_64_to_DSRpkt(0xF2,Target); //prep pkt with address
				route_obtained=route_discovery(rd_pkt1,0xF2,Target);
				int route_t=check_dis_r_validity(route_obtained);
				if(route_t==0)
				{
					return 2;
				}
			}
			else {
				//Cached
				printf("Ca1");
				route_obtained.route_r_len=rc_ptr.r_len[s_1];
				for(int i=0;i<route_obtained.route_r_len;i++)
				{ route_obtained.route_r_up[i]=rc_ptr.cached_route[s_1][i];
				}
			}
		}
		else
		{
			if (((tick)-rc_ptr.expir_id[s_1])>EXP_TIME)
			{ // Greater, RD
				printf("RD2");
				wait_for_route=1;
				wait_init(2);
				DSRpacket rd_pkt1 =add_64_to_DSRpkt(0xF2,Target); //prep pkt with address
				route_obtained=route_discovery(rd_pkt1,0xF2,Target);
				int route_t=check_dis_r_validity(route_obtained);
				if(route_t==0)
				{
					return 2;
				}
			}
			else
			{	//Cached
				printf("Ca2");
				route_obtained.route_r_len=rc_ptr.r_len[s_1];
				for(int i=0;i<route_obtained.route_r_len;i++)
				{ route_obtained.route_r_up[i]=rc_ptr.cached_route[s_1][i];
				}
			}
		}	// else
	} // entry not valid

	else{
		wait_for_route=1;
		wait_init(2);
		DSRpacket rd_pkt1 =add_64_to_DSRpkt(0xF2,Target); //prep pkt with address
		route_obtained=route_discovery(rd_pkt1,0xF2,Target);
		int route_t=check_dis_r_validity(route_obtained);
		if(route_t==0)
		{
			return 2;
		}
	}
	// Contention Window
	wait_sec(20);

	//Fill Route
	for(int i=((route_obtained.route_r_len)-1);i>=0;i--)
	{ f_pkt=append_route_r(f_pkt,route_obtained.route_r_up[i]);
	}
	//Fill Route Length
	f_pkt.route_record_length=route_obtained.route_r_len; //3
	//Fill Data
	for(int i=0;i<f_pkt.dataLen;i++)
	{
		f_pkt.data[i]=msg1[i];
	}
	// Initialize r_pointer
	f_pkt.expiration_period=f_pkt.route_record_length; //(3-1) Current pointer value 2
	int l_ent=read_last_entry_ptr(f_pkt);  //401 read r(2)--> 1
	f_pkt=detach_route_ptr(f_pkt); // current value --> 1
	updated_TX_DSR_Packet(f_pkt,l_ent,0xF6);
	wait_for_ack=1;
	int ack_status=0;
	ack_status=status_ack();
	if(ack_status==0)
	{
		return 3;
	}
	
	return 1;
}
