#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";
	int result;
	envid_t sender_id;
	int perm = 0;

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	
	while(1)
	{
		perm = 0;
		result = ipc_recv(&sender_id, &nsipcbuf, &perm);
		//if(result == NS_REQ_OUTPUT)
		if(perm == 0 || sender_id == 0)
		{
			cprintf("Invalid Request to ipc_recv: %e", result);
		}
		
		if(sender_id != ns_envid)
		{
			cprintf("Expected from env: %x, but received from env: %x", ns_envid, sender_id);
		}
		if(sys_net_transmit(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len))
		cprintf("Could not transmit even after 10 attempts");
	}
}
