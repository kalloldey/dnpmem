/* 
 * File:   dnp.h
 * Author: kallol
 *
 * Created on October 3, 2013, 4:23 PM
 */

#ifndef __XEN_NETBACK__DNP_H__
#define __XEN_NETBACK__DNP_H__

#include "igbvf.h"
#include <linux/netdevice.h>
#define MAX_POSSIBLE_VF 500
/*A data structure to be kept by the dnp_controller to keep track minimal informations
about the VFs*/
/*
struct xen_netbk;
struct backend_info;

struct dnpvf{
        char name[10];          //eth 1 2 3 whatever       
        int vm_domid;           //VM domid to which it is assigned, if not assigned then -1
        struct net_device *dnp_netdev; //netdevice structure mapped to this VF
        struct xen_netbk *xennetbk;        
        u8   fe_dev_addr[6];  //MAC address of this vf        
};
struct dnpvf alldnpVFs[MAX_POSSIBLE_VF];
int total_VF,total_VF_Assigned;
static int associate_dnpvf_with_backend(struct backend_info *);
struct xen_netbk *vfnetdev_to_netbk(struct net_device *);
struct net_device *getNetdev(int );
void updatednpVF(int ,int ,int);
int dnpvf_can_be_assigned(void);
int dnp_controller_init(void);
int backend_allocate_dnpVF(struct backend_info *);
int vfway_send_pkt_to_guest(struct sk_buff *, struct net_device *);
unsigned vfway_xen_netbk_tx_build_gops(struct xen_netbk *);
void xen_netbk_tx_submit_to_dnpvf(struct xen_netbk *);
void vfway_xen_netbk_tx_action(struct xen_netbk *);


extern netdev_tx_t igbvf_xmit_frame(struct sk_buff *skb,struct net_device *netdev)
extern int dvfa_set_mac(struct net_device *netdev, u8 *mac_addr);
*/
#endif	/* DNP_XEN */
