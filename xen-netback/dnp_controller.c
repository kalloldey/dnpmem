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
#include "common.h"
#include "igbvf.h"
/**
 * 
 * @return net_device structure to a particular vf.
 */
struct dnpvf *alldnpVFs[MAX_POSSIBLE_VF];
static int total_VF, total_VF_Assigned;

struct xenvif *allXenVifs[MAX_VM];
static int total_VM, effective_VM;

struct dnp_counters *dnpctrs=NULL;
struct kobject *dnp_kobj;

static ssize_t vmswitch_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
        return sprintf(buf, "%d\n",0);
}

static ssize_t vmswitch(struct kobject *kobj, struct kobj_attribute *attr,
                        const char *buf,size_t count)
{
        int vmid;
        sscanf(buf, "%d", &vmid);

        //dnpctrs->vm_id = vmid;
        printk(KERN_INFO "DNPMEM nb going for switching of the vm with id= %d\n",vmid);
        switch_vif_netif(vmid,1);
        return count;
}
static struct kobj_attribute dnpctrs_vmswitch_attribute = __ATTR(dnp_switch,0666,vmswitch_show,vmswitch);

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
#if 0
static void noinline do_check_kpage(void *vaddr)
{    
    char *a = (char *) vaddr;
    memset(a,'a',10);
}
#endif


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
    total_VM = 0;
    effective_VM = 0;
    total_VF=2;
    total_VF_Assigned=0;
    alldnpVFs[0]=(struct dnpvf *)kzalloc(sizeof(struct dnpvf),GFP_KERNEL);
    alldnpVFs[1]=(struct dnpvf *)kzalloc(sizeof(struct dnpvf),GFP_KERNEL);
    strcpy(alldnpVFs[0]->name,"eth5");
    alldnpVFs[0]->vm_domid=-1;
    alldnpVFs[0]->dnp_netdev=dev_get_by_name(&init_net,alldnpVFs[0]->name);
    memcpy(alldnpVFs[0]->default_mac, alldnpVFs[0]->dnp_netdev->dev_addr, alldnpVFs[0]->dnp_netdev->addr_len);
    
    MASSERT(alldnpVFs[0]->dnp_netdev);
    strcpy(alldnpVFs[1]->name,"eth6");
    alldnpVFs[1]->vm_domid=-1;
    alldnpVFs[1]->dnp_netdev=dev_get_by_name(&init_net,alldnpVFs[1]->name);
    memcpy(alldnpVFs[1]->default_mac, alldnpVFs[1]->dnp_netdev->dev_addr, alldnpVFs[1]->dnp_netdev->addr_len);
    
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
}


int associate_dnpvf_with_backend(struct xenvif *vif){  
 
    int i, cn;
    struct igbvf_adapter *tmp;
    
    for(i=0;i<total_VF;i++){
        if(alldnpVFs[i]->vm_domid!=-1)
            continue;
        else
            break;
    }
    if(i==total_VF){
        printk(KERN_INFO "[Error] no vf is there to assign but still you called %s!\n",__func__);
        return -1;
    }
    //Update all the states here ...
    //two entry to update on backend info ..one on vif attached with be
    vif->dnp_net_device=alldnpVFs[i]->dnp_netdev;
    vif->assigned_dnpVF_ID=i; //check using this value -1 or other value
    vif->in_transit = 0;
    vif->rx.rsp_prod_pvt = vif->rx.req_cons;
// dnptwo  <<<<<<<<<<    
    vif->map_info = (struct dnp_cb *)kmalloc(sizeof(struct dnp_cb) * DNP_MAX_NR_PAGE, GFP_KERNEL);
    vif->buffer_thread = kthread_create(dnp_buffer_kthread,
                                            (void *)vif,
                                            "dnp_buffer/%u",i);    
    if (IS_ERR(vif->buffer_thread)) {
	printk(KERN_ALERT "DNPMEM Buffer thread() creation fails \n");
    }
    MASSERT(vif->map_info) //not happened FINE
    
    init_waitqueue_head(&vif->waitq);
    
    vif->skb_with_driver = 0;
    vif->skb_freed = 0;
    vif->skb_receive_count = 0;    
    spin_lock_init(&vif->counter_lock);
    
    vif->use_pageq = 1;
    vif->all_queued_pages = (unsigned long *) kmalloc(sizeof(unsigned long) * DNP_MAX_NR_PAGE, GFP_KERNEL);;
    if (kfifo_alloc(&vif->page_queue, sizeof(unsigned long)*DNP_MAX_NR_PAGE, GFP_KERNEL)){
        printk(KERN_INFO "DNPMEM nb, PROBLEM IN CREATING KERNEL QUEUE\n");
        vif->use_pageq = 0;
    }else{    
        for(cn=0;cn <DNP_MAX_NR_PAGE; cn++){
                unsigned long val;
                struct page *tmp= alloc_page(GFP_ATOMIC);
                if (tmp == NULL){
                        printk(KERN_INFO "DNPMEM nb, PROBLEM IN ALLOCATING 256 pages\n");
                }
                val =  (unsigned long)tmp;
                kfifo_in(&vif->page_queue, &val, sizeof(unsigned long));  
                vif->all_queued_pages[cn] = (unsigned long)tmp;
        }
        printk(KERN_INFO "DNPMEM nb, Kernel queue with all pages created\n");
    }
    
    if (kfifo_alloc(&vif->map_info_queue, sizeof(struct dnp_cb)*DNP_MAX_NR_PAGE, GFP_KERNEL)){
        printk(KERN_INFO "DNPMEM nb, PROBLEM IN CREATING MAP INFO QUEUE\n");        
    }
    
    wake_up_process(vif->buffer_thread);   
// dnptwo  >>>>>>>>>>
    
    //Two entry to be updated on alldnpVFs
    alldnpVFs[i]->vm_domid=vif->domid;  
    alldnpVFs[i]->xennetvif=vif;
    
    skb_queue_head_init(&vif->avail_skb);
    //Populate skbs available
    
    //Two entry to update on igbvf respective to this vf
    tmp=netdev_priv(vif->dnp_net_device);        
    tmp->deliver_packet_to_netback = vfway_receive_rxpkt;
    tmp->alloc_dnpskb = dnp_alloc_skb;
    tmp->free_dnpskb = dnp_free_skb;  
    wmb();  //be->dnp_net_device use this
    set_restart_igbvf(vif->dnp_net_device,i) ; //FINE -16.03_2148    
    total_VF_Assigned++;        

 //   EXIT();
    return i;
}
//Write a similar disassociate function here also ..like the upper



/**
 * Will give the net back of a particular vf is currently assigned. 
 * This function implementation is currently redundant. 
 
 */
struct xenvif* noinline vfnetdev_to_xenvif(struct net_device *dev){
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
struct net_device* noinline getNetdev(int ind){
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



// dnptwo  <<<<<<<<<<

void dnp_map_frontend_buffer(struct xenvif *vif){
    
    //Declarations:
    unsigned long free_loc, i, ci;
    RING_IDX cons, prods;
    struct xen_netif_rx_request *req;
    struct gnttab_map_grant_ref *tmp_map_ops;
    struct page **tmp_page_mapped; 
    uint16_t tmpdnpid[DNP_MAX_NR_PAGE];
        
    //Function body:
    if(vif->in_transit == 1){
        printk(KERN_INFO "DNPMEM nb, IN Transit from VF to bridge .. so not making nay more buffer");
        return;
    }

    cons = vif->rx.req_cons ;
    prods = vif->rx.sring->req_prod;
    
//    printk(KERN_INFO "DNPMEM nb, DMF req prod=%u req cons=%u resp prod=%u req unconsumed=%d\n",prods,cons,vif->rx.rsp_prod_pvt,RING_HAS_UNCONSUMED_REQUESTS(&vif->rx));
    if(!RING_HAS_UNCONSUMED_REQUESTS(&vif->rx)){
        //printk(KERN_INFO "DNPMEM nb NO REQUEST In Ring .. So Return From Here \n");
        return;
    }    

    free_loc = prods - cons;  
    tmp_map_ops = (struct gnttab_map_grant_ref *)kzalloc(sizeof(struct gnttab_map_grant_ref) * free_loc, GFP_KERNEL);
    tmp_page_mapped =(struct page **)kzalloc(sizeof(unsigned long) * free_loc, GFP_KERNEL);
    for(i=0; i <free_loc ; i++){
      //  tmp_page_mapped[i] = alloc_page(GFP_ATOMIC); //remember to free the page
        if(!kfifo_is_empty(&vif->page_queue)){
                unsigned long pagetmp;
                kfifo_out(&vif->page_queue, &pagetmp, sizeof(unsigned long));
                tmp_page_mapped[i] = (struct page *)pagetmp;
        }
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
        BUG_ON(!req);
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
        if(ret){
             // printk(KERN_INFO "map error %d\n",ret);
	      BUG_ON(ret);
        }  
        WARN_ON(ret);
    }
    ci=0;
    for(i =0 ;i<free_loc; i++){
        struct dnp_cb cbtmp;
        if (unlikely(tmp_map_ops[i].status != 0)) {        
               printk(KERN_INFO "DNPMEM nb, Not mapped correctly status=%d \n",tmp_map_ops[i].status);
        }else{
                cbtmp.id = tmpdnpid[i]; 
                cbtmp.gref = tmp_map_ops[i].ref;
                cbtmp.pgad = (unsigned long) tmp_page_mapped[i];
                cbtmp.handle = tmp_map_ops[i].handle;
                kfifo_in(&vif->map_info_queue, &cbtmp, sizeof(struct dnp_cb)); 
                ci++;
        }
    }
    kfree(tmp_map_ops);
    kfree(tmp_page_mapped);
   // EXIT();
}


struct sk_buff *dnp_alloc_skb(struct net_device *netdevice, unsigned int length, int netdev_index) {

    struct sk_buff *skb = NULL;
    //struct xenvif *vif = vfnetdev_to_xenvif(getNetdev(netdev_index));
    struct xenvif *vif = vfnetdev_to_xenvif(netdevice);
    int nr_buf;
    struct dnp_cb *tag = NULL;
    struct dnp_cb useval;

    if (vif == NULL) {
        MASSERT(0);
        return NULL;
    }
     if(kfifo_is_empty(&vif->map_info_queue)){
        if (RING_HAS_UNCONSUMED_REQUESTS(&vif->rx))
            wake_up(&vif->waitq);
        return NULL;
    }

    skb = netdev_alloc_skb_ip_align(netdevice, DNP_SKB_SIZE); //remember to kfree_skb end     
    //skb = alloc_skb(DNP_SKB_SIZE, GFP_ATOMIC);
    if (unlikely(!skb)) {
        // printk(KERN_ERR "DNPMEM nb alloc skb fail: Not able to allocate skb\n");
        return NULL;
    }

    nr_buf = DNP_SKB_BUF_REQ(length);

    MASSERT(nr_buf == 1)
    if(nr_buf != 1){
         printk(KERN_ERR "DNPMEM nb **ERROR**  Cannot allocated skb with nr_buf not equal to 1\n");
         return NULL; 
    }

    kfifo_out(&vif->map_info_queue, &useval, sizeof(struct dnp_cb));
    
    skb_shinfo(skb)->nr_frags = 1;
    skb_shinfo(skb)->frags[0].page.p = (struct page *)useval.pgad; 
    skb_shinfo(skb)->frags[0].page_offset = 0;
    skb_shinfo(skb)->frags[0].size = PAGE_SIZE;
    skb->dev = netdevice; // Not needed anymore

    tag = (struct dnp_cb *) skb->cb;

    tag->id = useval.id;
    tag->gref = useval.gref;
    tag->handle = useval.handle;
    tag->pgad = useval.pgad;
    //   printk(KERN_INFO "DNPMEM nb %s:%d func=%s | skbCB: id=%u, grantref=%u pageaddr=%p, grant_handle =%u\n",__FILE__,__LINE__,__func__,tag->id,tag->gref,tag->pgad,tag->handle);

    vif->skb_with_driver++;        
    if(RING_HAS_UNCONSUMED_REQUESTS(&vif->rx))
        wake_up(&vif->waitq);
    //keep some check to kick other buffer getting service and skb freeing service here
    return skb;
}

void dnp_free_skb(struct sk_buff *skb, int vfid){ //DONE 80
    struct xenvif *vif = NULL;
    struct dnp_cb *dnc = NULL;
    struct dnp_cb requeue;
    vif = vfnetdev_to_xenvif(getNetdev(vfid));
    //Total assumption of single page packet ... will break down otherwise   
    dnc = (struct dnp_cb *)skb->cb;

    if(dnc == NULL)
        return;
    requeue.id = dnc->id;
    requeue.gref = dnc->gref;
    requeue.pgad = dnc->pgad;
    requeue.handle = dnc->handle;
    kfifo_in(&vif->map_info_queue, &requeue, sizeof(struct dnp_cb));    
     
    vif->skb_freed++;
    vif->skb_with_driver-- ;
    skb_shinfo(skb)->frags[0].page.p = NULL;
    skb_shinfo(skb)->nr_frags = 0;
    kfree_skb(skb);
}

unsigned long freeloc(unsigned long x, unsigned long y, bool flag){
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

int vfway_receive_rxpkt(struct sk_buff *skb, struct net_device *dev, int netdev_index) //FINE 80
{
        struct xenvif *vif = NULL;
        vif = vfnetdev_to_xenvif(getNetdev(netdev_index));
        if(!vif){
            MASSERT(0)
            goto pass;
        }
        if(!skb){
            MASSERT(0)
            goto pass;
        }
        //keep a counter for how many are queued
	skb_queue_tail(&vif->avail_skb, skb);
        vif->skb_receive_count++;
        if(vif->skb_receive_count > 8){
            wake_up(&vif->waitq);
        }
        return NETDEV_TX_OK;
pass:
        printk(KERN_INFO "DNPMEM nb Dropped packet\n");
        return NETDEV_TX_OK;

        
}

void vfway_send_pkt_to_guest(struct xenvif *vif) {
    //Directly send packet to guest .. and unmap page

    struct gnttab_unmap_grant_ref *unmap;
    struct page **pp;
    int ret, ci=0;
    struct sk_buff *skb = NULL;
    int placereq = DNP_MAX_NR_PAGE; 
    unmap = (struct gnttab_unmap_grant_ref *)kzalloc(sizeof(struct gnttab_unmap_grant_ref) * placereq, GFP_KERNEL);
    pp = (struct page **)kzalloc(sizeof(unsigned long) * placereq, GFP_KERNEL);
  
    while ((skb = skb_dequeue(&vif->avail_skb)) != NULL) {
        u16 flags = 0;
        struct xen_netif_rx_response *resp = NULL;
        struct dnp_cb *data = NULL;
        unsigned long vaddr;
        unsigned long tmpval;
        data = (struct dnp_cb *) skb->cb;
        if (!data) {
                printk(KERN_INFO "DNPMEM nb skb-cb is nulll\n");
                MASSERT(0)            
                goto bypass;
        }
        BUG_ON(skb_shinfo(skb) == NULL);
        //printk(KERN_INFO "DNPMEM nb, VFWAY req prod=%u req cons=%u resp prod=%u in req unconsumed=%d\n",vif->rx.sring->req_prod,vif->rx.req_cons,vif->rx.rsp_prod_pvt,RING_HAS_UNCONSUMED_REQUESTS(&vif->rx));

        BUG_ON((unsigned long)skb_shinfo(skb)->frags[0].page.p != data->pgad);
        vaddr = (unsigned long) pfn_to_kaddr(page_to_pfn(skb_shinfo(skb)->frags[0].page.p));
        gnttab_set_unmap_op(&unmap[ci],
                vaddr,
                GNTMAP_host_map,
                data->handle);
        pp[ci] = virt_to_page(vaddr);

        resp = RING_GET_RESPONSE(&vif->rx, vif->rx.rsp_prod_pvt+ci);
        BUG_ON(!resp);
        resp->offset = 0;
        resp->flags = flags;
        resp->id = data->id;
        resp->status = (s16) skb->len; //DOUBT. correct ?? lets proceed with this ..
        //vif->rx.rsp_prod_pvt++;
        // printk(KERN_INFO "DNPMEM nb, VFWAY reqP=%u reqC=%u resp-P=%u req_to_consm=%d\n",vif->rx.sring->req_prod,vif->rx.req_cons,vif->rx.rsp_prod_pvt,RING_HAS_UNCONSUMED_REQUESTS(&vif->rx));

        if (!kfifo_is_full(&vif->page_queue)) {
            tmpval = (unsigned long) skb_shinfo(skb)->frags[0].page.p;
            kfifo_in(&vif->page_queue, &tmpval, sizeof (unsigned long));
        } else {
            printk(KERN_INFO "DNPMEM nb error Page Queue is full\n");
        }
        skb_shinfo(skb)->frags[0].page.p = NULL;
        skb_shinfo(skb)->nr_frags = 0;
        kfree_skb(skb);
        dnpctrs->num_rcvd++;
        ci++;
        continue;
bypass:
        printk(KERN_INFO "DNPMEM nb Dropped packet\n");
    }
    if(ci>0){
        ret = gnttab_unmap_refs(unmap, NULL, pp, ci);

        if (ret)
            printk(KERN_INFO "DNPMEM nb, Error In Ring UNMAP\n");
        if (unlikely(unmap[0].status != 0)) { //just checking the first one .. can check all
            printk(KERN_INFO "DNPMEM nb, Not UNMAPPED correctly status=%d \n", unmap[0].status);
            BUG_ON(1);
        }
    
    
        vif->rx.rsp_prod_pvt =  vif->rx.rsp_prod_pvt+ci;
        RING_PUSH_RESPONSES_AND_CHECK_NOTIFY(&vif->rx, ret);
        if (ret)
                notify_remote_via_irq(vif->irq);
    }
    vif->skb_receive_count = 0;
    kfree(pp);
    kfree(unmap);
    /*
     drop:
            vif->dev->stats.tx_dropped++;
            dnp_free_skb(skb, vif->assigned_dnpVF_ID);
            return 0;
     */
}

int dnp_buffer_kthread(void *data){
    struct xenvif *vif = (struct xenvif *)data;
    while (!kthread_should_stop()) {
                wait_event_interruptible_timeout(vif->waitq,
                        RING_HAS_UNCONSUMED_REQUESTS(&vif->rx) || (!skb_queue_empty(&vif->avail_skb))
                        || kthread_should_stop(),msecs_to_jiffies(1));
                cond_resched();                  
                //PRN("Thread inside after wake up...\n");
                if (kthread_should_stop())
			break;
                if(!skb_queue_empty(&vif->avail_skb))
                        vfway_send_pkt_to_guest(vif);
                if(RING_HAS_UNCONSUMED_REQUESTS(&vif->rx))
                        dnp_map_frontend_buffer(vif); 
                
    }
    return 0;                    
}

int backend_allocate_dnpVF(struct xenvif *vif ){
    
    ENTER();
    if(dnpvf_can_be_assigned()){        
        return associate_dnpvf_with_backend(vif);
    }
    else
        return -1;
}



int transit_vf_to_bridge(struct xenvif *vif){
    int i, rc;
    unsigned long *frame_list; 
    struct gnttab_unmap_grant_ref *unmap;
    struct page **pp;
    int ret, ci=0;
    int placereq = DNP_MAX_NR_PAGE; 
    struct xen_memory_reservation reservation = {
        .address_bits = 0,
        .extent_order = 0,
        .domid        = DOMID_SELF
    };
    unmap = (struct gnttab_unmap_grant_ref *)kzalloc(sizeof(struct gnttab_unmap_grant_ref) * placereq, GFP_KERNEL);
    pp = (struct page **)kzalloc(sizeof(unsigned long) * placereq, GFP_KERNEL);
   // ENTER();
  //  printk(KERN_INFO "DNPMEM nb Values reqP=%d reqC=%d respP=%d \n",vif->rx.sring->req_prod,vif->rx.req_cons, vif->rx.rsp_prod_pvt);
  
    vif->in_transit = 1;
    //Stop kthread
    kthread_stop(vif->buffer_thread);
    //Send in flight packets to the guest
    if(!skb_queue_empty(&vif->avail_skb))
        vfway_send_pkt_to_guest(vif); 
    //Disable IRQ
    disable_irq(vif->irq);
    frame_list =(unsigned long *)kmalloc(sizeof(unsigned long) * DNP_MAX_NR_PAGE, GFP_KERNEL);
    //Will reset the VF with its default MAC    
    set_restart_igbvf(vif->dnp_net_device,-1);
    //consider calling it inside set_restart, in between igbvf up and down
    dvfa_set_mac(vif->dnp_net_device,alldnpVFs[vif->assigned_dnpVF_ID]->default_mac);      
    //Remove all the given buffer to the VF    
      
    
    //Unmap rest of the pages
    while(!kfifo_is_empty(&vif->map_info_queue)){
            struct dnp_cb useval;
            unsigned long vaddr;
            kfifo_out(&vif->map_info_queue, &useval, sizeof(struct dnp_cb));
            vaddr = (unsigned long) pfn_to_kaddr(page_to_pfn((struct page *)useval.pgad));
            gnttab_set_unmap_op(&unmap[ci],
                vaddr,
                GNTMAP_host_map,
                useval.handle);
            pp[ci++] = virt_to_page(vaddr);
    }
    
    if(ci>0){
        ret = gnttab_unmap_refs(unmap, NULL, pp, ci);

        if (ret)
            printk(KERN_INFO "DNPMEM nb, Error In Ring UNMAP\n");
        if (unlikely(unmap[0].status != 0)) { //just checking the first one .. can check all
            printk(KERN_INFO "DNPMEM nb, Not UNMAPPED correctly status=%d \n", unmap[0].status);
            BUG_ON(1);
        }else{
            //printk(KERN_INFO "DNPMEM nb, Left out mapped pages unmap done\n");
        }
    }
    
    //Will rebuild mfn mapping for all pages
     for (i = 0; i < DNP_MAX_NR_PAGE; i++) {                
        frame_list[i] = page_to_pfn((struct page *)vif->all_queued_pages[i]);
     }
     set_xen_guest_handle(reservation.extent_start, frame_list);
     reservation.nr_extents = DNP_MAX_NR_PAGE;
     rc = HYPERVISOR_memory_op(XENMEM_populate_physmap, &reservation);
     if (rc <= 0)
        return -1;
     
    // What will you do with the grefs on hold? Do some heuristics    
    //Will update used/unused VF counter        
    vif->in_transit = 0;
    kfree(frame_list);
    kfree(vif->map_info);
    kfree(vif->all_queued_pages);
    kfifo_free(&vif->map_info_queue);
    kfifo_free(&vif->page_queue);
    vif->dnp_net_device = NULL;
    //vif->rx.req_cons = vif->rx.rsp_prod_pvt+1; //GAMBLE Let's if it works
    vif->rx.rsp_prod_pvt = vif->rx.req_cons;
    vif->rx_req_cons_peek = 0;
    alldnpVFs[vif->assigned_dnpVF_ID]->vm_domid = -1;            
    vif->assigned_dnpVF_ID = -1;
    total_VF_Assigned--;
    enable_irq(vif->irq);
    //printk(KERN_INFO "DNPMEM nb Values reqP=%d reqC=%d respP=%d \n",vif->rx.sring->req_prod,vif->rx.req_cons, vif->rx.rsp_prod_pvt);
   // EXIT(); //Reaching here successfully   
    return 0;
}

/*
int bridge_to_vf_enabler(struct xenvif *vif){
    
}
*/
/*
 * switch_vif_netif: Will be called to switch a VM from VF to bridge and vice versa
 * vmid: id of the vm
 * flag: 1 just flipflop the present settings,
 *       2 means VF to Bridge
 *       3 means bridge to VF
 */
void switch_vif_netif(int vmid, int flag) {

    struct xenvif *vif = NULL;
    int err;
    vif = allXenVifs[vmid];
    if (vif->assigned_dnpVF_ID != -1 && flag == 3) {
        printk(KERN_INFO "[DNPMEM] nb Already have VF .. nothing to do\n");
        return; //already present
    }

    if (vif->assigned_dnpVF_ID == -1 && flag == 2) {
        printk(KERN_INFO "[DNPMEM] nb Already have Bridge .. nothing to do\n");
        return; //already present
    }
    if (flag == 1 && vif->assigned_dnpVF_ID == -1) {
        //VF alloc code
        err = backend_allocate_dnpVF(vif);
        if (err == -1) { /*[DNP] success part taken care inside function*/
            printk(KERN_INFO "[DNPMEM] nb Error in allocating VF to VM\n");
            vif->assigned_dnpVF_ID = -1;
            vif->dnp_net_device = NULL;
        } else {
            dvfa_set_mac(vif->dnp_net_device, vif->fe_dev_addr); //[KALLOL]  --our Mac
            wake_up(&vif->waitq);
            printk(KERN_INFO "[DNPMEM] nb {INFO} Switching from Bridge to VF done for VM ID= %d ******\n", vmid);
        }        
        //printk(KERN_INFO "DNPMEM nb SWITCH: Values reqP=%d reqC=%d respP=%d \n",vif->rx.sring->req_prod,vif->rx.req_cons, vif->rx.rsp_prod_pvt);
        return;
    }
    if (flag == 1 && vif->assigned_dnpVF_ID != -1) {
        // printk(KERN_INFO "[DNPMEM] nb SWITHCED TO Bridge STARTED\n");
        printk(KERN_INFO "[DNPMEM] nb {INFO} Switching from VF to Bridge done for VM ID= %d ******\n", vmid);
        transit_vf_to_bridge(vif);
        return;
    }

}

void register_vif(struct xenvif *vif){
    allXenVifs[(int)vif->domid]=vif;
    printk(KERN_INFO "[DNPMEM] nb Registered VM= %d\n", vif->domid);
    total_VM = total_VM > vif->domid ? total_VM: vif->domid;
}
