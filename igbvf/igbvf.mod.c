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
	{ 0xb2a58b77, "alloc_pages_current" },
	{ 0x3ce4ca6f, "disable_irq" },
	{ 0xd83ea029, "kmalloc_caches" },
	{ 0xd2b09ce5, "__kmalloc" },
	{ 0xf9a482f9, "msleep" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x15692c87, "param_ops_int" },
	{ 0x91eb9b4, "round_jiffies" },
	{ 0x754d539c, "strlen" },
	{ 0x6a1ceaf2, "dev_set_drvdata" },
	{ 0x79aa04a2, "get_random_bytes" },
	{ 0x317830a3, "dma_set_mask" },
	{ 0x907f3da9, "napi_complete" },
	{ 0x2c74531e, "pci_disable_device" },
	{ 0xa39cff96, "pci_disable_msix" },
	{ 0x564d77b4, "netif_carrier_on" },
	{ 0xf89843f9, "schedule_work" },
	{ 0xc0a3d105, "find_next_bit" },
	{ 0x2596c00f, "netif_carrier_off" },
	{ 0x88bfa7e, "cancel_work_sync" },
	{ 0x4632e3d8, "x86_dma_fallback_dev" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xb45fd755, "pci_release_regions" },
	{ 0xfb0e29f, "init_timer_key" },
	{ 0x999e8297, "vfree" },
	{ 0x91715312, "sprintf" },
	{ 0x3a68bc1c, "netif_napi_del" },
	{ 0x7d11c268, "jiffies" },
	{ 0xad37de41, "__netdev_alloc_skb" },
	{ 0x27c33efe, "csum_ipv6_magic" },
	{ 0x8db98a2f, "pci_set_master" },
	{ 0xd5f2172f, "del_timer_sync" },
	{ 0xfb578fc5, "memset" },
	{ 0x9e7884fe, "pci_enable_msix" },
	{ 0x8490a655, "pci_restore_state" },
	{ 0x23ac266, "dev_err" },
	{ 0x2a43c47e, "ethtool_op_get_link" },
	{ 0x27e1a049, "printk" },
	{ 0x449ad0a7, "memcmp" },
	{ 0x6b47929d, "free_netdev" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0x9166fada, "strncpy" },
	{ 0x607533d9, "register_netdev" },
	{ 0xb4390f9a, "mcount" },
	{ 0xcb03efb2, "netif_receive_skb" },
	{ 0x5792f848, "strlcpy" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x8834396c, "mod_timer" },
	{ 0x71366580, "netif_napi_add" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0x1c2e11b9, "__get_page_tail" },
	{ 0x304bb38c, "dev_kfree_skb_any" },
	{ 0xd572b98e, "dev_kfree_skb_irq" },
	{ 0x98c4266e, "netif_device_attach" },
	{ 0x45f845f2, "_dev_info" },
	{ 0x40a9b349, "vzalloc" },
	{ 0x687240ef, "netif_device_detach" },
	{ 0x42c8de35, "ioremap_nocache" },
	{ 0xc159c8f5, "__napi_schedule" },
	{ 0x525c0aec, "eth_type_trans" },
	{ 0x7ce16b86, "pskb_expand_head" },
	{ 0xa3313886, "pci_unregister_driver" },
	{ 0xcc5005fe, "msleep_interruptible" },
	{ 0xa2e429a9, "kmem_cache_alloc_trace" },
	{ 0xe52947e7, "__phys_addr" },
	{ 0xfcec0987, "enable_irq" },
	{ 0x37a0cba, "kfree" },
	{ 0x69acdf38, "memcpy" },
	{ 0xfe951431, "pci_request_regions" },
	{ 0xa5a7ba32, "dma_supported" },
	{ 0xedc03953, "iounmap" },
	{ 0x8b1ce044, "__pci_register_driver" },
	{ 0xec7e91a4, "put_page" },
	{ 0xb352177e, "find_first_bit" },
	{ 0x2ece1902, "unregister_netdev" },
	{ 0xde861d8f, "__netif_schedule" },
	{ 0x6981228d, "consume_skb" },
	{ 0x5a49816f, "pci_enable_device_mem" },
	{ 0xf28c90c8, "skb_put" },
	{ 0x466649c3, "dev_get_drvdata" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0xfd5f002d, "dma_ops" },
	{ 0xf20dabd8, "free_irq" },
	{ 0xad64ab3, "pci_save_state" },
	{ 0xfe94161a, "alloc_etherdev_mqs" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v00008086d000010CAsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001520sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "EE5ADA87EBCC84FA42EB996");
