## 结构体

### usbg_state

```c
struct usbg_state
{
    char *path;
    char *configfs_path;

    TAILQ_HEAD(ghead, usbg_gadget) gadgets;
    TAILQ_HEAD(uhead, usbg_udc) udcs;
    config_t *last_failed_import;
};
```

## 函数

pass


