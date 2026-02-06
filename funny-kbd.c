// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/workqueue.h>
#include <uapi/linux/input-event-codes.h>
#include <linux/input.h>

MODULE_LICENSE("GPL");

static struct input_dev *virt_kbd;

unsigned int eng_word[] = {
    KEY_H,
    KEY_E,
    KEY_L,
    KEY_L,
    KEY_O};

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

    struct kb_work *kw =
        container_of(work, struct kb_work, work);

    pr_info("funny-kbd: action = %lu keycode = %u  shift = %d\n", kw->action, kw->keycode, kw->shift);

    if (kw->keycode == KEY_COMMA)
    {
        send_keys(virt_kbd, eng_word, sizeof(eng_word) / sizeof(unsigned int));
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