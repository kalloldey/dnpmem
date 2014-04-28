#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xb9d3067e, "module_layout" },
	{ 0x2a43c47e, "ethtool_op_get_link" },
	{ 0xc8221daa, "eth_validate_addr" },
	{ 0xdc1ac580, "eth_mac_addr" },
	{ 0xdf72a1ec, "xenbus_unregister_driver" },
	{ 0xd8f400c3, "xenbus_register_frontend" },
	{ 0x8b66f9e0, "xen_platform_pci_unplug" },
	{ 0x731dba7a, "xen_domain_type" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0x6981228d, "consume_skb" },
	{ 0x3f695780, "netif_skb_features" },
	{ 0x4302d0eb, "free_pages" },
	{  0x64d22, "xenbus_switch_state" },
	{ 0x564d77b4, "netif_carrier_on" },
	{ 0x6e720ff2, "rtnl_unlock" },
	{ 0x447942a6, "netdev_update_features" },
	{ 0xc7a4fbed, "rtnl_lock" },
	{ 0xca81ea9a, "xenbus_transaction_end" },
	{ 0x73013896, "xenbus_printf" },
	{ 0x8c06a108, "xenbus_transaction_start" },
	{ 0xe41534ce, "bind_evtchn_to_irqhandler" },
	{ 0x2970864f, "xenbus_alloc_evtchn" },
	{ 0xea71f815, "xenbus_grant_ring" },
	{ 0xe52947e7, "__phys_addr" },
	{ 0x9b388444, "get_zeroed_page" },
	{ 0x37a0cba, "kfree" },
	{ 0xb99d5837, "xenbus_read" },
	{ 0xbedf9534, "netif_notify_peers" },
	{ 0x3513d4af, "xenbus_frontend_closed" },
	{ 0x45f845f2, "_dev_info" },
	{ 0xd5f2172f, "del_timer_sync" },
	{ 0x466649c3, "dev_get_drvdata" },
	{ 0x7712771a, "unbind_from_irqhandler" },
	{ 0xedbc6f67, "gnttab_end_foreign_access" },
	{ 0x43261dca, "_raw_spin_lock_irq" },
	{ 0x8f64aa4, "_raw_spin_unlock_irqrestore" },
	{ 0x9327f5ce, "_raw_spin_lock_irqsave" },
	{ 0xcb03efb2, "netif_receive_skb" },
	{ 0xadaabe1b, "pv_lock_ops" },
	{ 0x7bb8a2ac, "__napi_complete" },
	{ 0x78764f4e, "pv_irq_ops" },
	{ 0x525c0aec, "eth_type_trans" },
	{ 0x5435c56e, "__free_pages" },
	{ 0x69acdf38, "memcpy" },
	{ 0x7628f3c7, "this_cpu_off" },
	{ 0xf28c90c8, "skb_put" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0xd52bf1ce, "_raw_spin_lock" },
	{ 0xc159c8f5, "__napi_schedule" },
	{ 0xba63339c, "_raw_spin_unlock_bh" },
	{ 0x1637ff0f, "_raw_spin_lock_bh" },
	{ 0x20000329, "simple_strtoul" },
	{ 0xc6cbbc89, "capable" },
	{ 0x8834396c, "mod_timer" },
	{ 0x7d11c268, "jiffies" },
	{ 0x86623fd7, "notify_remote_via_irq" },
	{ 0x18f83fab, "gnttab_grant_foreign_access_ref" },
	{ 0xfe727411, "get_phys_to_machine" },
	{ 0x55526907, "xen_features" },
	{ 0x5af03a28, "gnttab_claim_grant_reference" },
	{ 0xe9299b17, "kfree_skb" },
	{ 0xb2a58b77, "alloc_pages_current" },
	{ 0xad37de41, "__netdev_alloc_skb" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0xf9a482f9, "msleep" },
	{ 0x8029af7f, "netdev_info" },
	{ 0x2ece1902, "unregister_netdev" },
	{ 0xcc9bdf7b, "device_remove_file" },
	{ 0x8ebd8d50, "device_create_file" },
	{ 0x607533d9, "register_netdev" },
	{ 0x6a1ceaf2, "dev_set_drvdata" },
	{ 0x6012462c, "xenbus_dev_fatal" },
	{ 0x6b47929d, "free_netdev" },
	{ 0xc9ec4e21, "free_percpu" },
	{ 0x2596c00f, "netif_carrier_off" },
	{ 0x71366580, "netif_napi_add" },
	{ 0x9d3850e1, "gnttab_alloc_grant_references" },
	{ 0x949f7342, "__alloc_percpu" },
	{ 0xfb0e29f, "init_timer_key" },
	{ 0xfe94161a, "alloc_etherdev_mqs" },
	{ 0xf5945bac, "gnttab_free_grant_references" },
	{ 0x71c8282f, "dev_warn" },
	{ 0xfe7c4287, "nr_cpu_ids" },
	{ 0xc0a3d105, "find_next_bit" },
	{ 0x3928efe9, "__per_cpu_offset" },
	{ 0xb9249d16, "cpu_possible_mask" },
	{ 0xde861d8f, "__netif_schedule" },
	{ 0x27e1a049, "printk" },
	{ 0xb4e14553, "gnttab_query_foreign_access" },
	{ 0xd572b98e, "dev_kfree_skb_irq" },
	{ 0x3f84d4c9, "gnttab_release_grant_reference" },
	{ 0xfd51b281, "gnttab_end_foreign_access_ref" },
	{ 0x964add15, "xenbus_scanf" },
	{ 0x91715312, "sprintf" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "E58E05973CC85A32C2F5065");
