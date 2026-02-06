// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");

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

static int __init funny_kbd_init(void)
{
    pr_info("funny-kbd: Module loaded\n");
    kb_wq = alloc_workqueue("kb_wq", WQ_UNBOUND, 1);
    if (!kb_wq)
        return -ENOMEM;

    return register_keyboard_notifier(&kbd_nb);
}

static void __exit funny_kbd_exit(void)
{
    unregister_keyboard_notifier(&kbd_nb);

    flush_workqueue(kb_wq);
    destroy_workqueue(kb_wq);

    pr_info("funny-kbd: Module unloaded\n");
    return;
}

module_init(funny_kbd_init);
module_exit(funny_kbd_exit);