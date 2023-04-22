#include <kern/chan/channel.h>
#include <kern/chan/queuing.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/pmm.h>
#include <layouts.h>
#include <lib/hashmap.h>
#include <lib/string.h>

static sys_addr_t channel_base = CHAN_BASE;

long channel_create(struct channel_conf *cc) {
  assert(cc->cc_media == CM_MEMORY);
  assert(cc->cc_type == CT_QUEUING || cc->cc_type == CT_SAMPLING);
  info("create channel: media=memory,type=%s,ports=",
       cc->cc_type == CT_QUEUING ? "queuing" : "sampling");
  for (char **port_name = cc->cc_port_names; *port_name != NULL; ++port_name) {
    printk("'%s'%c", *port_name, *(port_name + 1) == NULL ? '\n' : '&');
  }
  struct channel *ch = kalloc(sizeof(struct channel));
  memset(ch, 0, sizeof(struct channel));
  ch->ch_media = cc->cc_media;
  ch->ch_type = cc->cc_type;
  if (cc->cc_type == CT_QUEUING) {
    size_t port_cnt = 0;
    for (char **port_name = cc->cc_port_names; *port_name != NULL; ++port_name) {
      catch_e(queuing_port_conf_chan(*port_name, ch), {
        kfree(ch);
        return err;
      });
      port_cnt++;
    }
    if (port_cnt != 2) { // only unicast is supported
      kfree(ch);
      return -KER_PORT_ER;
    }
    ch->ch_view.queuing.ch_nb_msg = 0;
    ch->ch_view.queuing.ch_off_b = 0;
    ch->ch_view.queuing.ch_off_e =
        -(ch->ch_view.queuing.ch_max_msg_size + sizeof(struct comm_msg));
    list_init(&ch->ch_view.queuing.ch_waiting_procs);
  } else {
    UNIMPLEMENTED;
  }
  ch->ch_data = (void *)channel_base;
  channel_base += ch->ch_cap;
  spinlock_init(&ch->ch_lock);
  return KER_SUCCESS;
}

long channel_mem_setup(void) {
  struct fmt_mem_range range = {.addr = CHAN_BASE, .size = channel_base - CHAN_BASE};
  info("channel memory setup: %pM (size: %pB)\n", &range, &range.size);
  for (vaddr_t vaddr = {.val = CHAN_BASE}; vaddr.val < channel_base; vaddr.val += PGSIZE) {
    struct phy_frame *frame;
    uintptr_t pa;
    catch_e(phy_frame_alloc(&frame));
    panic_e(frame2pa(frame, &pa));
    perm_t perm = {.bits = {.g = 1, .r = 1, .w = 1}};
    paddr_t paddr = {.val = pa};
    panic_e(pt_map(kern_pgdir, vaddr, paddr, perm));
  }
  part_pt_sync_kern_pgdir();
  return KER_SUCCESS;
}
