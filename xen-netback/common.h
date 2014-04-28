/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation; or, when distributed
 * separately from the Linux kernel or incorporated into other
 * software packages, subject to the following license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this source file (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __XEN_NETBACK__COMMON_H__
#define __XEN_NETBACK__COMMON_H__

#define pr_fmt(fmt) KBUILD_MODNAME ":%s: " fmt, __func__

#include <linux/kthread.h>
#include <linux/if_vlan.h>
#include <linux/udp.h>

#include <net/tcp.h>

#include <xen/events.h>
#include <xen/interface/memory.h>

#include <asm/xen/hypercall.h>
#include <asm/xen/page.h>
//----the above is brought from netback.c
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/io.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/wait.h>
#include <linux/sched.h>

#include <xen/interface/io/netif.h>
#include <xen/interface/grant_table.h>
#include <xen/grant_table.h>
#include <xen/xenbus.h>

#include "igbvf.h"
#include <linux/pagemap.h> 
#define MASSERT(x) if(!(x)){\
                      printk(KERN_ALERT "DNPMEM nb !!!!!!!!!!!!!! ASSERT Failure @%s:%d func=%s\n",__FILE__,__LINE__,__func__);\
                      }

#define ENTER() printk(KERN_INFO "DNPMEM nb %s:%d func=%s | Entry Point \n",__FILE__,__LINE__,__func__)
#define EXIT() printk(KERN_INFO "DNPMEM nb %s:%d func=%s | Exit Point \n",__FILE__,__LINE__,__func__)
#define FLOW() printk(KERN_INFO "DNPMEM nb %s:%d func=%s | Flow Point \n",__FILE__,__LINE__,__func__)
#define PRN(x) printk(KERN_INFO "DNPMEM nb %s:%d func=%s | %s \n",__FILE__,__LINE__,__func__, x)           
/*
 * To avoid confusion, we define XEN_NETBK_LEGACY_SLOTS_MAX indicating
 * the maximum slots a valid packet can use. Now this value is defined
 * to be XEN_NETIF_NR_SLOTS_MIN, which is supposed to be supported by
 * all backend.
 */
#define XEN_NETBK_LEGACY_SLOTS_MAX XEN_NETIF_NR_SLOTS_MIN

typedef unsigned int pending_ring_idx_t;
#define INVALID_PENDING_RING_IDX (~0U)

struct pending_tx_info {
	struct xen_netif_tx_request req; /* coalesced tx request */
	struct xenvif *vif;
	pending_ring_idx_t head; /* head != INVALID_PENDING_RING_IDX
				  * if it is head of one or more tx
				  * reqs
				  */
};

struct netbk_rx_meta {
	int id;
	int size;
	int gso_size;
};

#define MAX_PENDING_REQS 256

// dnptwo  <<<<<<<<<<
#define DNP_MAX_NR_PAGE 256  //outstanding always 1024 to be kept
#define DNP_THRESHOLD  50   //how many buffer the dom0 must hold
#define DNP_SKB_SIZE 0
//#define FREELOC(x,y) ((x) > (y) ? (DNP_MAX_NR_PAGE - (x)+(y)): ((y) - (x))) //leader,follower ALWAYS
#define INCR(x,y) (((x) + (y)) & (DNP_MAX_NR_PAGE -1))
#define DNP_SKB_BUF_REQ(len) (((len) + PAGE_SIZE -1) / PAGE_SIZE )

//keep upper, follower .
unsigned int freeloc(unsigned int x, unsigned int y, bool flag);
// dnptwo  >>>>>>>>>>

/* Discriminate from any valid pending_idx value. */
#define INVALID_PENDING_IDX 0xFFFF

#define MAX_BUFFER_OFFSET PAGE_SIZE

#define XEN_NETIF_TX_RING_SIZE __CONST_RING_SIZE(xen_netif_tx, PAGE_SIZE)
#define XEN_NETIF_RX_RING_SIZE __CONST_RING_SIZE(xen_netif_rx, PAGE_SIZE)
/* extra field used in struct page */
union page_ext {
	struct {
#if BITS_PER_LONG < 64
#define IDX_WIDTH   8
#define GROUP_WIDTH (BITS_PER_LONG - IDX_WIDTH)
		unsigned int group:GROUP_WIDTH;
		unsigned int idx:IDX_WIDTH;
#else
		unsigned int group, idx;
#endif
	} e;
	void *mapping;
};
struct xen_netbk;
struct xen_netbk {
	wait_queue_head_t wq;
	struct task_struct *task;

	struct sk_buff_head rx_queue;
	struct sk_buff_head tx_queue;

	struct timer_list net_timer;

	struct page *mmap_pages[MAX_PENDING_REQS];

	pending_ring_idx_t pending_prod;
	pending_ring_idx_t pending_cons;
	struct list_head net_schedule_list;

	/* Protect the net_schedule_list in netif. */
	spinlock_t net_schedule_list_lock;

	atomic_t netfront_count;

	struct pending_tx_info pending_tx_info[MAX_PENDING_REQS];
	/* Coalescing tx requests before copying makes number of grant
	 * copy ops greater or equal to number of slots required. In
	 * worst case a tx request consumes 2 gnttab_copy.
	 */
	struct gnttab_copy tx_copy_ops[2*MAX_PENDING_REQS];

	u16 pending_ring[MAX_PENDING_REQS];

	/*
	 * Given MAX_BUFFER_OFFSET of 4096 the worst case is that each
	 * head/fragment page uses 2 copy operations because it
	 * straddles two buffers in the frontend.
	 */
	struct gnttab_copy grant_copy_op[2*XEN_NETIF_RX_RING_SIZE];
	struct netbk_rx_meta meta[2*XEN_NETIF_RX_RING_SIZE];

};
struct xenvif {
	/* Unique identifier for this interface. */
	domid_t          domid;
	unsigned int     handle;

	/* Reference to netback processing backend. */
	struct xen_netbk *netbk;

	u8               fe_dev_addr[6];

	/* Physical parameters of the comms window. */
	unsigned int     irq;

	/* List of frontends to notify after a batch of frames sent. */
	struct list_head notify_list;

	/* The shared rings and indexes. */
	struct xen_netif_tx_back_ring tx;
	struct xen_netif_rx_back_ring rx;

	/* Frontend feature information. */
	u8 can_sg:1;
	u8 gso:1;
	u8 gso_prefix:1;
	u8 csum:1;

	/* Internal feature information. */
	u8 can_queue:1;	    /* can queue packets for receiver? */

	/*
	 * Allow xenvif_start_xmit() to peek ahead in the rx request
	 * ring.  This is a prediction of what rx_req_cons will be
	 * once all queued skbs are put on the ring.
	 */
	RING_IDX rx_req_cons_peek;

	/* Transmit shaping: allow 'credit_bytes' every 'credit_usec'. */
	unsigned long   credit_bytes;
	unsigned long   credit_usec;
	unsigned long   remaining_credit;
	struct timer_list credit_timeout;

	/* Statistics */
	unsigned long rx_gso_checksum_fixup;

	/* Miscellaneous private stuff. */
	struct list_head schedule_list;
	atomic_t         refcnt;
	struct net_device *dev;

	wait_queue_head_t waiting_to_free;
        
#ifdef DNP_XEN   

        int assigned_dnpVF_ID;  // will be -1 if no dnfvf currently have;
// dnptwo  <<<<<<<<<<
	struct task_struct *buffer_thread;
        wait_queue_head_t waitq;
        //DNP_MAX_NR_PAGE is the limit ..    
        unsigned long skb_with_driver;  
        unsigned long skb_freed;
        unsigned long skb_receive_count;
        
        struct page **page_mapped;  
        struct gnttab_map_grant_ref *dnp_map_ops;
        uint16_t *dnp_mapped_id;
        
        unsigned int leader;  /* will increase when dom0 will get some page to 
                                    be mapped to its own address space*/
        unsigned int follower;  /* will increase when dom0 will use some mapped pages
                                            *  to the driver*/     
        bool fullflag; //will be 1 when leader == follower but queue is full, 0 other wise .. start with 0
/*        struct sk_buff_head avail_skb;  //the DB of skb
        struct sk_buff_head rx_queue;  //will keep the skbs may be that will come from the driver
        struct sk_buff_head recycle_skb;  // receive processed now this skb will be again used as avail*/
// dnptwo  >>>>>>>>>>
#endif
};

struct backend_info {
	struct xenbus_device *dev;
	struct xenvif *vif;
 #ifdef DNP_XEN       
        struct net_device *dnp_net_device;  /*[Kallol]Introduce another data type to keep the info 
                                                * of VF that can be assigned to this backend*/
#endif        
	enum xenbus_state frontend_state;
        int mode_using;          /*[Kallol] In which mode netback is currently in
                                 0: No mode specifeid yet, 1: VF assigned, 2:With bridge*/
	struct xenbus_watch hotplug_status_watch;
	u8 have_hotplug_status_watch:1;
};

static inline struct xenbus_device *xenvif_to_xenbus_device(struct xenvif *vif)
{
	return to_xenbus_device(vif->dev->dev.parent);
}


struct xenvif *xenvif_alloc(struct device *parent,
			    domid_t domid,
			    unsigned int handle);

int xenvif_connect(struct xenvif *vif, unsigned long tx_ring_ref,
		   unsigned long rx_ring_ref, unsigned int evtchn);
void xenvif_disconnect(struct xenvif *vif);

void xenvif_get(struct xenvif *vif);
void xenvif_put(struct xenvif *vif);

int xenvif_xenbus_init(void);

int xenvif_schedulable(struct xenvif *vif);

int xen_netbk_rx_ring_full(struct xenvif *vif);

int xen_netbk_must_stop_queue(struct xenvif *vif);

/* (Un)Map communication rings. */
void xen_netbk_unmap_frontend_rings(struct xenvif *vif);
int xen_netbk_map_frontend_rings(struct xenvif *vif,
				 grant_ref_t tx_ring_ref,
				 grant_ref_t rx_ring_ref);

/* (De)Register a xenvif with the netback backend. */
void xen_netbk_add_xenvif(struct xenvif *vif);
void xen_netbk_remove_xenvif(struct xenvif *vif);

/* (De)Schedule backend processing for a xenvif */
void xen_netbk_schedule_xenvif(struct xenvif *vif);
void xen_netbk_deschedule_xenvif(struct xenvif *vif);

/* Check for SKBs from frontend and schedule backend processing */
void xen_netbk_check_rx_xenvif(struct xenvif *vif);
/* Receive an SKB from the frontend */
void xenvif_receive_skb(struct xenvif *vif, struct sk_buff *skb);

/* Queue an SKB for transmission to the frontend */
void xen_netbk_queue_tx_skb(struct xenvif *vif, struct sk_buff *skb);
/* Notify xenvif that ring now has space to send an skb to the frontend */
void xenvif_notify_tx_completion(struct xenvif *vif);

/* Prevent the device from generating any further traffic. */
void xenvif_carrier_off(struct xenvif *vif);

/* Returns number of ring slots required to send an skb to the frontend */
unsigned int xen_netbk_count_skb_slots(struct xenvif *vif, struct sk_buff *skb);

struct xenvif *poll_net_schedule_list(struct xen_netbk *);
void xen_netbk_kick_thread(struct xen_netbk *);
#ifdef DNP_XEN

struct dnp_cb {
    uint16_t    id; 
    grant_ref_t gref;  //not required may be ...
    struct page *pgad;
    grant_handle_t handle;
};


#define MAX_POSSIBLE_VF 500

struct dnpvf{
        char name[10];          //eth 1 2 3 whatever       
        int vm_domid;           //VM domid to which it is assigned, if not assigned then -1
        struct net_device *dnp_netdev; //netdevice structure mapped to this VF
        struct xenvif *xennetvif;
        u8   fe_dev_addr[6];  //MAC address of this vf        
};

int associate_dnpvf_with_backend(struct backend_info *);
struct xenvif *vfnetdev_to_xenvif(struct net_device *);
struct net_device *getNetdev(int );
void updatednpVF(int ,int ,int);
int dnpvf_can_be_assigned(void);
void dnp_controller_init(void);
int backend_allocate_dnpVF(struct backend_info *);
int vfway_send_pkt_to_guest(struct sk_buff *, struct net_device *, int); //dnptwo modification
unsigned vfway_xen_netbk_tx_build_gops(struct xen_netbk *);
void xen_netbk_tx_submit_to_dnpvf(struct xen_netbk *);
void vfway_xen_netbk_tx_action(struct xen_netbk *);
extern netdev_tx_t igbvf_xmit_frame(struct sk_buff *skb,struct net_device *netdev);
extern int dvfa_set_mac(struct net_device *netdev, u8 *mac_addr);

// dnptwo  <<<<<<<<<<
extern void set_restart_igbvf(struct net_device *,int );
void dnp_map_frontend_buffer(struct xenvif *vif);
struct sk_buff *dnp_alloc_skb(struct net_device *, unsigned int , int);
void dnp_free_skb(struct sk_buff *skb, int vfid);
int dnp_buffer_kthread(void *data);
// dnptwo  >>>>>>>>>>

#endif //DNP_XEN
#endif /* __XEN_NETBACK__COMMON_H__ */
