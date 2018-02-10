#include <kern/e1000.h>

// LAB 6: Your driver code here Meghana

struct e1000_tx_desc  e1000_txQ[E1000_TX_MAXDESC] __attribute__ ((aligned (16)));
struct packets_tx pkts_tx_buf[E1000_TX_MAXDESC];

struct e1000_rx_desc  e1000_rxQ[E1000_RX_MAXDESC] __attribute__ ((aligned (16)));
struct packets_rx pkts_rx_buf[E1000_RX_MAXDESC];

int pci_attach_enable (struct pci_func *pcif)
{
	pci_func_enable(pcif);
	physaddr_t E1000_BASEADDR = pcif->reg_base[0];
	e1000Base = mmio_map_region(E1000_BASEADDR, pcif->reg_size[0]);
	// cprintf("Hi...%x\n", e1000Base[2]); // 80080783
	
	memset(e1000_txQ, 0, sizeof(struct e1000_tx_desc) * E1000_TX_MAXDESC);
	
	// 
	e1000Base[E1000_TDBAL] = PADDR(e1000_txQ);
	e1000Base[E1000_TDBAH] = 0;
	e1000Base[E1000_TDLEN] = sizeof(struct e1000_tx_desc) * E1000_TX_MAXDESC;
	e1000Base[E1000_TDH] = 0;
	e1000Base[E1000_TDT] = 0;
	e1000Base[E1000_TCTL] = 0;
	
	/*
	 */ 
	e1000Base[E1000_TCTL] |= E1000_TCTL_EN;
	e1000Base[E1000_TCTL] |= E1000_TCTL_PSP;
	//e1000Base[E1000_TCTL] &= ~E1000_TCTL_CT;
	e1000Base[E1000_TCTL] |= (0x10) << 4;
	//e1000Base[E1000_TCTL] &= ~E1000_TCTL_COLD;
	e1000Base[E1000_TCTL] |= (0x40) << 12;
	
	/* Inter Packet Gap
	 * IPGT = A h
	 * IPGR1 = 2/3 of IPGR1
	 * IPGR2 = 6 from manual page: 310
	 */
	
	e1000Base[E1000_TIPG] = 0;
	e1000Base[E1000_TIPG] |= 0xA;
	e1000Base[E1000_TIPG] |= (0x4) << 10;
	e1000Base[E1000_TIPG] |= (0xC) << 20;
	
	for(int i=0; i < E1000_TX_MAXDESC; i++)
	{
		e1000_txQ[i].buffer_addr = PADDR(pkts_tx_buf[i].buffer); // set packet buffer address to Descriptor
		e1000_txQ[i].lower.flags.cmd = 0x8; // set RS bit of CMD
		e1000_txQ[i].upper.fields.status = 0x1; // set DD bit of status
	}
	
	memset(e1000_rxQ, 0, sizeof(struct e1000_rx_desc) * E1000_RX_MAXDESC);
	
	e1000Base[E1000_RA+1] |= E1000_RAH_AV;
	e1000Base[E1000_RA] |= 0x52540012;
	e1000Base[E1000_RA+1] |= 0x3456;
	e1000Base[E1000_RDH] = 0;
	e1000Base[E1000_RDT] = 0;
	e1000Base[E1000_RDBAL] = PADDR(e1000_rxQ);
	e1000Base[E1000_RDBAH] = 0;
	e1000Base[E1000_RDLEN] = sizeof(struct e1000_rx_desc) * E1000_RX_MAXDESC;
	e1000Base[E1000_RCTL] = 0;
	
	
	// Receive Initialization
	/*e1000Base[E1000_IMS] |= E1000_IMS_LSC;
	e1000Base[E1000_IMS] |= E1000_IMS_RXSEQ
	e1000Base[E1000_IMS] |= E1000_IMS_RXT0;
	e1000Base[E1000_IMS] |= E1000_IMS_RXO
	*/
	
	// IMS for receiver
	e1000Base[E1000_RCTL] |= E1000_RCTL_LPE;
	e1000Base[E1000_RCTL] |= E1000_RCTL_LBM_NO;
	e1000Base[E1000_RCTL] |= E1000_RCTL_BAM;
	e1000Base[E1000_RCTL] |= E1000_RCTL_SZ_2048;
	e1000Base[E1000_RCTL] |= E1000_RCTL_SECRC;
	
	for(int i=0; i < E1000_RX_MAXDESC; i++)
	{
		e1000_rxQ[i].buffer_addr = PADDR(pkts_rx_buf[i].buffer); // set packet buffer address to Descriptor
		//e1000_rxQ[i].lower.flags.cmd = 0x8; // set RS bit of CMD
		e1000_rxQ[i].status = 0x1; // set DD bit of status
	}
	
	return 0;
}

int transmit_enable(char *pkt_inputData, size_t size)
{
	uint32_t tail_next = e1000Base[E1000_TDT];
	//cprintf("hi1..%d %d\n", e1000Base[E1000_TDT], (e1000_txQ[tail_next].upper.fields.status & 0x1));
	if((e1000_txQ[tail_next].upper.fields.status & 0x1) == 1)
	{
		memmove((void *) &pkts_tx_buf[tail_next], pkt_inputData, size);
		e1000_txQ[tail_next].upper.fields.status &= 0xFE;
		e1000_txQ[tail_next].lower.flags.length = size;
		e1000_txQ[tail_next].lower.flags.cmd |= 0x00000008 | 0x00000001;
		//cprintf("hi..%x %x\n", e1000_txQ[tail_next].lower.flags.length, e1000_txQ[tail_next].upper.fields.status);
		e1000Base[E1000_TDT] = (e1000Base[E1000_TDT] + 1) % E1000_TX_MAXDESC;
		//cprintf("hi2.. %d\n", e1000Base[E1000_TDT]);
		
		return 0;
	}
	panic("Hey! the queue is full!!");
	return 0;
}
