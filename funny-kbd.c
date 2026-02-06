// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/workqueue.h>
#include <uapi/linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");

static unsigned long kallsyms_lookup_name_hack(const char *name)
{
    struct kprobe kp = {
        .symbol_name = "kallsyms_lookup_name"
    };
    unsigned long addr = 0;

    if (register_kprobe(&kp) < 0)
    {
        pr_err("Failed to register kprobe on kallsyms_lookup_name\n");
        return 0;
    }

    addr = (unsigned long)kp.addr;

    unregister_kprobe(&kp);

    return addr;
}

static unsigned short **get_key_maps(void)
{
    typedef unsigned long (*kallsyms_fn)(const char *);
    kallsyms_fn lookup = (kallsyms_fn)kallsyms_lookup_name_hack("kallsyms_lookup_name");

    if (!lookup)
        return NULL;

    unsigned long addr = lookup("key_maps");
    if (!addr)
        return NULL;

    return (unsigned short **)addr;
}

static unsigned short **maps;

static struct input_dev *virt_kbd;

unsigned int russian_word[] = {
    KEY_COMMA,
    KEY_K,
    KEY_B,
    KEY_Y};

unsigned int english_word[] = {
    KEY_S,
    KEY_H,
    KEY_I,
    KEY_T,
};

static void send_keys(struct input_dev *kbd, unsigned int *keys, size_t num)
{
    for (int i = 0; i < num; i++)
    {
        input_report_key(kbd, keys[i], 1);
        input_report_key(kbd, keys[i], 0);
        input_sync(kbd);
    }
}

static struct workqueue_struct *kb_wq;

struct kb_work
{
    struct work_struct work;
    unsigned long action;
    unsigned int keycode;
    int shift;
};

static void kb_work_fn(struct work_struct *work)
{
    bool place_russian = false;

    struct kb_work *kw =
        container_of(work, struct kb_work, work);

    unsigned short symbol = maps[kw->shift][kw->keycode];

    if (maps[kw->shift][27] == 0x42a || maps[kw->shift][27] == 0x44a)
    {
        place_russian = true;
    }
    else
    {
        place_russian = false;
    }

    pr_info("funny-kbd: keycode=%u action=%lu shift=%d symbol=%hx place_russian=%d\n",
            kw->keycode, kw->action, kw->shift, symbol, place_russian);

    if (kw->keycode == KEY_COMMA && !place_russian && symbol == 0xf02c)
    {
        send_keys(virt_kbd, english_word, sizeof(english_word) / sizeof(unsigned int));
    }
    else if (kw->keycode == KEY_SLASH && place_russian && symbol == 0xf02c)
    {
        send_keys(virt_kbd, russian_word, sizeof(russian_word) / sizeof(unsigned int));
    }
    kfree(kw);
}

static int keyboard_notifier_cb(struct notifier_block *nblock,
                                unsigned long action, void *param)
{
    struct kb_work *kw;

    struct keyboard_notifier_param *p = param;

    if (action != KBD_KEYCODE || !p->down)
        return NOTIFY_OK;

    kw = kmalloc(sizeof(*kw), GFP_ATOMIC);

    if (!kw)
        return NOTIFY_OK;

    INIT_WORK(&kw->work, kb_work_fn);

    kw->action = action;
    kw->keycode = p->value;
    kw->shift = p->shift;

    queue_work(kb_wq, &kw->work);

    return NOTIFY_OK;
}

static struct notifier_block kbd_nb = {
    .notifier_call = keyboard_notifier_cb};

static int init_virt_kbd(void)
{
    int ret;

    virt_kbd = input_allocate_device();

    if (!virt_kbd)
    {
        pr_err("input_allocate_device failed\n");
        return -ENOMEM;
    }

    virt_kbd->name = "Funny Keyboard";
    virt_kbd->phys = "virtual/filtered/kbd";
    virt_kbd->id.bustype = BUS_VIRTUAL;
    virt_kbd->id.vendor = 0xDEAD;
    virt_kbd->id.product = 0xBEEF;

    set_bit(EV_KEY, virt_kbd->evbit);
    set_bit(EV_REP, virt_kbd->evbit);

    for (int i = 0; i < KEY_CNT; i++)
    {
        set_bit(i, virt_kbd->keybit);
    }

    ret = input_register_device(virt_kbd);

    if (ret)
    {
        pr_err(KERN_ERR "input_register_device failed: %d\n", ret);
        input_free_device(virt_kbd);
        return ret;
    }

    return 0;
}
static int __init funny_kbd_init(void)
{
    pr_info("funny-kbd: Module loaded\n");
    maps = get_key_maps();
    kb_wq = alloc_workqueue("kb_wq", WQ_UNBOUND, 1);
    if (!kb_wq)
        return -ENOMEM;
    init_virt_kbd();
    return register_keyboard_notifier(&kbd_nb);
}

static void __exit funny_kbd_exit(void)
{
    unregister_keyboard_notifier(&kbd_nb);

    input_unregister_device(virt_kbd);
    input_free_device(virt_kbd);

    flush_workqueue(kb_wq);
    destroy_workqueue(kb_wq);

    pr_info("funny-kbd: Module unloaded\n");
    return;
}

module_init(funny_kbd_init);
module_exit(funny_kbd_exit);