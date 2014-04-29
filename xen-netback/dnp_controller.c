/* File Name: dnp_controller.c
 * Purpose of this file: 
 * [03-10-2013] 
 *      1.This will be initiated when netback initiates. 
 *      2.This will control the netbacks. Several APIs provided by 
 *      the netback to change the communication channel to bridge
 *      to VF and viceversa will be called from this module based 
 *      on some given policy. 
 *      3. In case of moving to bridge to VF, this module will tell 
 *      the netback in which VF they should move.
 *      4.This will provide all the administrative information support
 *      to the netbacks corresponding to the netfronts. If netback need 
 *      those information.
 */
/* Revision History:
 * [03-10-2013] Initial Create
 *
 */
//#include<linux/skbuff.h>
#include "common.h"
#include "igbvf.h"
/**
 * 
 * @return net_device structure to a particular vf.
 */
struct dnpvf *alldnpVFs[MAX_POSSIBLE_VF];
static int total_VF,total_VF_Assigned;

struct dnp_counters *dnpctrs=NULL;
struct kobject *dnp_kobj;

static ssize_t vmswitch_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
        return sprintf(buf, "%u\n",0U);
}

static ssize_t vmswitch(struct kobject *kobj, struct kobj_attribute *attr,
                        const char *buf,size_t count)
{
        int vmid = *((int *)buf);
        if(count <= 0 || count > 6 || vmid < 0)
            return -EINVAL;
       //do the switching here
        return 0;
}
static struct kobj_attribute dnpctrs_vmswitch_attribute = __ATTR(dnp_switch,0644,vmswitch_show,vmswitch);

static ssize_t failed_maps_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
        return sprintf(buf, "%lu\n",dnpctrs->failed_maps);
}

static struct kobj_attribute dnpctrs_failed_maps_attribute = __ATTR(dnp_failed_maps,0444,failed_maps_show,NULL);

static ssize_t total_rcvd_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
        return sprintf(buf, "%lu\n",dnpctrs->num_rcvd);
}

static struct kobj_attribute dnpctrs_total_rcvd_attribute = __ATTR(total_rcvd,0444,total_rcvd_show,NULL);

static struct attribute *attrs[] = {
        &dnpctrs_vmswitch_attribute.attr,
        &dnpctrs_failed_maps_attribute.attr,
        &dnpctrs_total_rcvd_attribute.attr,
        NULL,   /* need to NULL terminate the list of attributes */
};
static struct attribute_group attr_group = {
        .attrs = attrs,
};

static void noinline do_check_kpage(void *vaddr)
{    
    char *a = (char *) vaddr;
    memset(a,'a',10);
}

int associate_dnpvf_with_backend(struct backend_info *be){  

    int i;
    struct igbvf_adapter *tmp;

    for(i=0;i<total_VF;i++){
        if(alldnpVFs[i]->vm_domid!=-1)
            continue;
        else
            break;
    }
    if(i==total_VF){
        printk(KERN_INFO "[Error] no vf is there to assign but still you called %s!",__func__);
        return -1;
    }
    //Update all the states here ...
    //two entry to update on backend info ..one on vif attached with be
    be->dnp_net_device=alldnpVFs[i]->dnp_netdev;
    be->mode_using=1; //can use VF mode  
    be->vif->assigned_dnpVF_ID=i; //check using this value -1 or other value
// dnptwo  <<<<<<<<<<    
    be->vif->dnp_map_ops = kmalloc(sizeof(be->vif->dnp_map_ops[0]) * DNP_MAX_NR_PAGE, GFP_KERNEL);
    be->vif->page_mapped = (struct page **)kmalloc(sizeof (be->vif->page_mapped[0]) * DNP_MAX_NR_PAGE, GFP_KERNEL);
    be->vif->dnp_mapped_id = kmalloc(sizeof(uint16_t) * DNP_MAX_NR_PAGE, GFP_KERNEL);

    be->vif->buffer_thread = kthread_create(dnp_buffer_kthread,
                                            (void *)be->vif,
                                            "dnp_buffer/%u",i);    
    if (IS_ERR(be->vif->buffer_thread)) {
	printk(KERN_ALERT "DNPMEM Buffer thread() creation fails \n");
    }
    MASSERT(be->vif->dnp_map_ops ||be->vif->page_mapped||    be->vif->dnp_mapped_id) //not happened FINE
    
    init_waitqueue_head(&be->vif->waitq);
    be->vif->leader = 0;
    be->vif->follower = 0; 
    be->vif->fullflag = false;
    
    be->vif->skb_with_driver = 0;
    be->vif->skb_freed = 0;
    be->vif->skb_receive_count = 0;    
    
    wake_up_process(be->vif->buffer_thread);   
// dnptwo  >>>>>>>>>>
    
    //Two entry to be updated on alldnpVFs
    alldnpVFs[i]->vm_domid=be->vif->domid;  
    alldnpVFs[i]->xennetvif=be->vif;
    
    //Two entry to update on igbvf respective to this vf
    tmp=netdev_priv(be->dnp_net_device);        
    tmp->deliver_packet_to_netback = vfway_send_pkt_to_guest;
    tmp->alloc_dnpskb = dnp_alloc_skb;
    tmp->free_dnpskb = dnp_free_skb;  
    wmb();  //be->dnp_net_device use this
    set_restart_igbvf(be->dnp_net_device,i) ; //FINE -16.03_2148
    

    total_VF_Assigned++;        

 //   EXIT();
    return i;
}
//Write a similar disassociate function here also ..like the upper



/**
 * Will give the net back of a particular vf is currently assigned. 
 * This function implementation is currently redundant. 
 
 */
struct xenvif *vfnetdev_to_xenvif(struct net_device *dev){
    struct xenvif *vif = NULL;
    struct igbvf_adapter *tmp=netdev_priv(dev);
    if(tmp->dnpvf_id >=0 )    
        vif=alldnpVFs[tmp->dnpvf_id]->xennetvif;
    else
        MASSERT(0);
    return vif;
}

/*Get netdevice with respect to the dnpvf  
 */
struct net_device *getNetdev(int ind){
    if(ind==-1)
        return NULL;
    return alldnpVFs[ind]->dnp_netdev;
}

/* Will tell num of VF is available to be assigned
 * Will return 0 if no vf available to be assigned.
 */

int dnpvf_can_be_assigned(void){
    return (total_VF-total_VF_Assigned);
}



/**
 * This is the initial module of the controller .. need to called only once
 * Works:
 * Will search for vf and will populate one data structure kept.
 * @return 
 */
void dnp_controller_init(void){    
    //alldnpVFs[] need to be populated...
    /*Search for all the netdevice in init_net namespace .. 
     * Identify if they are vf .. if so then save their info in alldnpVFs
     */
    //Temporary Hack: use the available vfs name hardcoded ... remove it as early as possible
    //take two vf .. eth4 and eth5 .. total vf=2;
  //  ENTER();
    total_VF=2;
    total_VF_Assigned=0;
    alldnpVFs[0]=(struct dnpvf *)kzalloc(sizeof(struct dnpvf),GFP_KERNEL);
    alldnpVFs[1]=(struct dnpvf *)kzalloc(sizeof(struct dnpvf),GFP_KERNEL);
    strcpy(alldnpVFs[0]->name,"eth5");
    alldnpVFs[0]->vm_domid=-1;
    alldnpVFs[0]->dnp_netdev=dev_get_by_name(&init_net,alldnpVFs[0]->name);
    MASSERT(alldnpVFs[0]->dnp_netdev);
    strcpy(alldnpVFs[1]->name,"eth6");
    alldnpVFs[1]->vm_domid=-1;
    alldnpVFs[1]->dnp_netdev=dev_get_by_name(&init_net,alldnpVFs[1]->name);
    dnp_kobj = kobject_create_and_add("dnp", NULL);
    if (!dnp_kobj)
           printk(KERN_WARNING "%s: kobj create error\n", __func__);
    dnpctrs = kmalloc(sizeof(struct dnp_counters),GFP_KERNEL);
    if(dnpctrs){
        memset(dnpctrs,0,sizeof(struct dnp_counters));
        if(sysfs_create_group(dnp_kobj, &attr_group)){
               printk(KERN_WARNING "%s: kobj group create errot\n",__func__);
        }else{
               printk(KERN_WARNING "Out of memory: %s\n",__func__);
        }
    }

 //   EXIT();
    /*<-- Temporary hack ends here ... remove it ASAP[03.10.2013]*/        
}

// dnptwo  <<<<<<<<<<

struct sk_buff *dnp_alloc_skb(struct net_device *netdevice, unsigned int length, int netdev_index){
    
    struct sk_buff *skb = NULL;
    //struct xenvif *vif = vfnetdev_to_xenvif(getNetdev(netdev_index));
    struct xenvif *vif = vfnetdev_to_xenvif(netdevice);
    unsigned int loc;
    int nr_buf;
    struct dnp_cb *tag = NULL;
  //  ENTER();

    if(vif ==NULL){
        MASSERT(0);
        return NULL;
    }
//    printk(KERN_INFO "%s %d:%s vif leader=%u, vif follwer=%u  \n",__FILE__,__LINE__,__func__,vif->leader, vif->follower);
    if(freeloc(vif->leader, vif->follower, vif->fullflag) == DNP_MAX_NR_PAGE){
        printk(KERN_ERR "DNPMEM nb No page present that can be allocated \n");
      //  MASSERT(0)
        if(RING_HAS_UNCONSUMED_REQUESTS(&vif->rx))
                wake_up(&vif->waitq);        
        return NULL;
    }

    skb = alloc_skb(DNP_SKB_SIZE, GFP_ATOMIC);  //remember to kfree_skb end     
    if(!skb){
       // MASSERT(0)
        printk(KERN_ERR "DNPMEM nb alloc skb fail: Not able to allocate skb\n");
        return NULL;
    }
    
    nr_buf = DNP_SKB_BUF_REQ(length);
 
    MASSERT(nr_buf == 1)
    if (nr_buf > MAX_SKB_FRAGS){
        MASSERT(0)
         printk(KERN_ERR "DNPMEM nb allocate skb nr_buf more than 1\n");
         return NULL;
    }

    loc = INCR(vif->follower, 0);
    BUG_ON(!vif->page_mapped[loc]);
        
    skb_shinfo(skb)->frags[0].page.p = vif->page_mapped[loc]; //check if we need to any kmap kind of thing here
    skb_shinfo(skb)->frags[0].page_offset = 0;        
    skb_shinfo(skb)->frags[0].size = PAGE_SIZE; 
    skb_shinfo(skb)->nr_frags=1;
    skb->dev = netdevice;
#if 0    
    addr = kmap_atomic(skb_shinfo(skb)->frags[0].page.p );
    do_check_kpage(addr);
    kunmap_atomic(addr);
#endif    

    tag = (struct dnp_cb *)skb->cb;
    tag->id = vif->dnp_mapped_id[loc];
    tag->gref = vif->dnp_map_ops[loc].ref;
    tag->handle = vif->dnp_map_ops[loc].handle;
    tag->pgad = vif->page_mapped[loc];
 //   printk(KERN_INFO "DNPMEM nb %s:%d func=%s | skbCB: id=%u, grantref=%u pageaddr=%p, grant_handle =%u\n",__FILE__,__LINE__,__func__,tag->id,tag->gref,tag->pgad,tag->handle);
    
    vif->skb_with_driver++;
 //   printk(KERN_INFO "DNPMEM nb skbs given to driver = %lu,  buffer from loc= %u\n",vif->skb_with_driver, vif->follower);
    //Below two are atomic ..need to ensure
    vif->fullflag = INCR(vif->follower, 0) == vif->leader ? false: vif->fullflag;
    vif->follower = INCR(vif->follower, 1);    
    
    if (!(vif->skb_with_driver % 12))
        wake_up(&vif->waitq);
    //keep some check to kick other buffer getting service and skb freeing service here
    return skb;
}

void dnp_free_skb(struct sk_buff *skb, int vfid){ //DONE 80
    struct xenvif *vif = NULL;
    struct dnp_cb *dnc = NULL;    
    vif = alldnpVFs[vfid]->xennetvif;  //sure ..
    FLOW();
    /*so this page pointer will be added to the top of leader
     * leader will increased by nr of pages with skb
     * follower will keep same
     */
    //Total assumption of single page packet ... will break down otherwise   
    dnc = (struct dnp_cb *)skb->cb;
    vif->page_mapped[vif->leader] = dnc->pgad;
    vif->dnp_mapped_id[vif->leader] = dnc->id;    
    vif->leader = INCR(vif->leader, 1);
    vif->skb_freed++;
    vif->skb_with_driver-- ;
    kfree_skb(skb);
}

unsigned int freeloc(unsigned int x, unsigned int y, bool flag){
    if(INCR(x, 0) == INCR(y, 0)){
        if(flag)
                return 0; //flag == 1 means that though x==y queue is full
        else
                return DNP_MAX_NR_PAGE;
    }
    if(x>y){
        return (DNP_MAX_NR_PAGE - (x)+(y));
    }
    return y - x;
}
// dnptwo  >>>>>>>>>>


int vfway_send_pkt_to_guest(struct sk_buff *skb, struct net_device *dev, int netdev_index) //FINE 80
{
        //Directly send packet to guest .. and unmap page
        u16 flags = 0;
        struct xenvif *vif = NULL;
        struct xen_netif_rx_response *resp;	
        struct dnp_cb *data=NULL;
        struct gnttab_unmap_grant_ref unmap[1];
        struct page *pp[1];
        int ret;
        unsigned long tmp_mfn;
        ENTER();
        vif = vfnetdev_to_xenvif(getNetdev(netdev_index));
        if(!vif){
            MASSERT(0)
            goto bypass;
        }        
        if(!skb){
            MASSERT(0)
            goto bypass;
        }
        data = (struct dnp_cb *)skb->cb;
        if(!data){
            MASSERT(0)
            goto bypass;
        }
       // printk(KERN_INFO "DNPMEM nb %s:%d func=%s | skbCB: id=%u, grantref=%u pageaddr=%p, grant_handle =%u, page virt addr=%lu\n",__FILE__,__LINE__,__func__,data->id,data->gref,data->pgad,data->handle, (unsigned long)pfn_to_kaddr(page_to_pfn(data->pgad)));
     if (skb->ip_summed == CHECKSUM_PARTIAL)
                flags |= XEN_NETRXF_csum_blank | XEN_NETRXF_data_validated;
        else if (skb->ip_summed == CHECKSUM_UNNECESSARY)

                flags |= XEN_NETRXF_data_validated;
        else
                flags = 0;                       
//        printk(KERN_INFO "DNPMEM nb, VFWAY req prod=%u req cons=%u resp prod=%u in req unconsumed=%d\n",vif->rx.sring->req_prod,vif->rx.req_cons,vif->rx.rsp_prod_pvt,RING_HAS_UNCONSUMED_REQUESTS(&vif->rx));
       // tmp_mfn = pfn_to_mfn(page_to_pfn(data->pgad));
#if 0
        if(data->pgad){
                void *addr = kmap_atomic((struct page*)data->pgad );
                do_check_kpage(addr);
                kunmap_atomic(addr);            
        }
#endif                    
        pp[0]=(struct page *)skb_shinfo(skb)->frags[0].page.p;//data->pgad;        
        gnttab_set_unmap_op(&unmap[0],
                (unsigned long)pfn_to_kaddr(page_to_pfn(skb_shinfo(skb)->frags[0].page.p)),
                GNTMAP_host_map,
                data->handle);
   
        ret = gnttab_unmap_refs(unmap, NULL,pp,1 );
        if(ret)
            printk(KERN_INFO "DNPMEM nb, Error In Ring UNMAP\n");
        MASSERT(ret == 0)
        if (unlikely(unmap[0].status != 0)) {
                printk(KERN_INFO "DNPMEM nb, Not UNMAPPED correctly status=%d \n", unmap[0].status);
        }                
                
        resp = RING_GET_RESPONSE(&vif->rx, vif->rx.rsp_prod_pvt);
	resp->offset     = 0;
	resp->flags      = flags;
	resp->id         = data->id;
	resp->status     = (s16)skb->len; //DOUBT. correct ?? lets proceed with this ..
        vif->rx.rsp_prod_pvt++;
        printk(KERN_INFO "DNPMEM nb, VFWAY reqP=%u reqC=%u resp-P=%u req_to_consm=%d\n",vif->rx.sring->req_prod,vif->rx.req_cons,vif->rx.rsp_prod_pvt,RING_HAS_UNCONSUMED_REQUESTS(&vif->rx));        
       // printk(KERN_INFO "DNPMEM nb, RING RSP Producer Increased #################### \n");
  /*      printk(KERN_INFO "DNPMEM nb , vfway mfn BEF unmap = %lu,\
                        mfn AFT unmap=%lu\n", tmp_mfn,\
                        pfn_to_mfn(page_to_pfn(data->pgad)));  */     
   
        if(skb_shinfo(skb)->frags[0].page.p){
#if 0
            void *addr = kmap_atomic((struct page*)(skb_shinfo(skb)->frags[0].page.p));
            do_check_kpage(addr);
            kunmap_atomic(addr);            
#endif            
            int pgc= page_count((struct page *)(skb_shinfo(skb)->frags[0].page.p));            
            printk(KERN_INFO "DNPMEM nb refcount gr8 than %d\n",pgc);
          //  __free_page((struct page*)(skb_shinfo(skb)->frags[0].page.p));                       
        }
        else
            MASSERT(0)                   
       kfree_skb(skb);     
       RING_PUSH_RESPONSES_AND_CHECK_NOTIFY(&vif->rx, ret);
        if(ret)
            notify_remote_via_irq(vif->irq);
       dnpctrs->num_rcvd++;
        //RING_PUSH_RESPONSES(&vif->rx);
        //printk(KERN_INFO "DNPMEM nb Response Updated,  Data Id=%d \n",data->id);
bypass:          
	return NETDEV_TX_OK;
/*
 drop:
	vif->dev->stats.tx_dropped++;
	dnp_free_skb(skb, vif->assigned_dnpVF_ID);
	return 0;
 */
}



// dnptwo  <<<<<<<<<< 
//job: will map buffer from the front end to the address space of dom0 when there are not
//100 outstanding pages are mapped

void dnp_map_frontend_buffer(struct xenvif *vif){
    
    //Declarations:
    unsigned int current_mapped,free_loc, i, req_avail, loc, ci;
    RING_IDX cons, prods;
    struct xen_netif_rx_request *req;
    struct gnttab_map_grant_ref *tmp_map_ops;
    struct page **tmp_page_mapped; 
    uint16_t tmpdnpid[60];
        
    //Function body:
   // ENTER();
    free_loc = freeloc(vif->leader, vif->follower, vif->fullflag);
    current_mapped = DNP_MAX_NR_PAGE - free_loc;

    if(current_mapped >= DNP_THRESHOLD){
        MASSERT(0);
        return; //we have enough buffer ...
    }
    if(free_loc == 0){
        MASSERT(0);
        return; //no place in your DNPRING
    }
    cons = vif->rx.req_cons ;
    prods = vif->rx.sring->req_prod;
    
    printk(KERN_INFO "DNPMEM nb, DMF req prod=%u req cons=%u resp prod=%u in req unconsumed=%d\n",prods,cons,vif->rx.rsp_prod_pvt,RING_HAS_UNCONSUMED_REQUESTS(&vif->rx));
    if(!RING_HAS_UNCONSUMED_REQUESTS(&vif->rx)){
        //printk(KERN_INFO "DNPMEM nb NO REQUEST In Ring .. So Return From Here \n");
        return;
    }    
    /*if(prods > cons)
        req_avail = ( prods & (XEN_NETIF_RX_RING_SIZE -1)) - (cons & (XEN_NETIF_RX_RING_SIZE -1));
    else
        req_avail = XEN_NETIF_RX_RING_SIZE + (prods & (XEN_NETIF_RX_RING_SIZE -1)) - ( cons& (XEN_NETIF_RX_RING_SIZE -1));
    */
    req_avail = prods - cons;
    free_loc = free_loc < req_avail ? free_loc : req_avail;
    //ONLY DO THE REQUIRED AMOUNT TO CROSS THE THRESHOLD
    free_loc = free_loc < (DNP_THRESHOLD - current_mapped) ? free_loc :(DNP_THRESHOLD - current_mapped);
    
    tmp_map_ops = (struct gnttab_map_grant_ref *)kzalloc(sizeof(struct gnttab_map_grant_ref) * free_loc, GFP_KERNEL);
    tmp_page_mapped =(struct page **)kzalloc(sizeof(unsigned long) * free_loc, GFP_KERNEL);
    for(i=0; i <free_loc ; i++){
        tmp_page_mapped[i] = alloc_page(GFP_ATOMIC); //remember to free the page
       // tmp_mfn_arr[i] = pfn_to_mfn(page_to_pfn(tmp_page_mapped[i]));
        MASSERT(tmp_page_mapped[i]);
        if (tmp_page_mapped[i] == NULL) {                
                printk(KERN_INFO "netback  Error:: No page to allocate in map\n");
                break;
        }
    }
    free_loc= i;
    i=0;   
    while(1){ 
        /*two break condition .. no place in DNPRING and 
         * no more request to consume in rx ring */         
        if(i == free_loc)
            break;                
        req = RING_GET_REQUEST(&vif->rx, vif->rx.req_cons++);
        gnttab_set_map_op(&tmp_map_ops[i],
                    		(unsigned long)pfn_to_kaddr(page_to_pfn(tmp_page_mapped[i])),
                    		GNTMAP_host_map,
                    		req->gref,
                    		(domid_t)vif->domid); //fine	        
        tmpdnpid[i] = req->id;
        i++;      
    }    

    if(free_loc!=0){        
        int ret = gnttab_map_refs(tmp_map_ops, NULL, tmp_page_mapped, free_loc);
        printk(KERN_INFO "DNPMEM nb, gntmap done\n");
        if(ret){
              printk(KERN_INFO "map error %d\n",ret);
	      BUG_ON(ret);
        }  
        WARN_ON(ret);
    }
    ci=0;
    for(i =0 ;i<free_loc; i++){  
        if (unlikely(tmp_map_ops[i].status != 0)) {        
                printk(KERN_INFO "DNPMEM nb, Not mapped correctly status=%d \n",tmp_map_ops[i].status);
        }else{
                loc = INCR(vif->leader, ci);
                vif->page_mapped[loc] = tmp_page_mapped[i];         
                vif->dnp_map_ops[loc] = tmp_map_ops[i];
                vif->dnp_mapped_id[loc] = tmpdnpid[i];        
        
             /*   printk(KERN_INFO "DNPMEM nb , get buf 4m FE loc = %u, mfn BEF map = %lu,\
                        mfn AFT map=%lu\n",INCR(vif->leader, i),\
                        tmp_mfn_arr[i],pfn_to_mfn(page_to_pfn(tmp_page_mapped[i]))); */
#if 0        
                addr = kmap_atomic(vif->page_mapped[loc] );
                do_check_kpage(addr);
                kunmap_atomic(addr);
#endif                
                ci++;
        }
    }
    //DO THE BELOW TWO ATOMICALLY
    vif->fullflag =  vif->follower == INCR(vif->leader , ci)? true:false;
    vif->leader = INCR(vif->leader , ci);
    //no need to save handle as I will get it from dnp_map_ops     
    printk(KERN_INFO "DNPMEM nb ,mapbuffer: leader= %u follower=%u map_possibility=%u correct-mapped=%u\n",vif->leader,vif->follower,free_loc,ci);
    kfree(tmp_map_ops);
    kfree(tmp_page_mapped);
   // EXIT();
}

int dnp_buffer_kthread(void *data){
    struct xenvif *vif = (struct xenvif *)data;
    while (!kthread_should_stop()) {
		wait_event_interruptible(vif->waitq,
                        ( ((DNP_MAX_NR_PAGE - (freeloc(vif->leader, vif->follower, vif->fullflag))) < DNP_THRESHOLD) ) 
                        || kthread_should_stop() );
                cond_resched();                  
                //PRN("Thread inside after wake up...\n");
                if (kthread_should_stop())
			break;
                if(RING_HAS_UNCONSUMED_REQUESTS(&vif->rx))
                        dnp_map_frontend_buffer(vif);                                        
    }
    return 0;                    
}

int backend_allocate_dnpVF(struct backend_info *be){
    
    ENTER();
    if(dnpvf_can_be_assigned()){        
        return associate_dnpvf_with_backend(be);
    }
    else
        return -1;
}


int vf_connect(struct net_device *dnpvf, unsigned long tx_ring_ref,
		   unsigned long rx_ring_ref, unsigned int evtchn){
    //change the vfs mac address same as the VMs mac address
    //call the api to modify the interrupt
	return 0;
}
