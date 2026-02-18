#ifndef NYE_BPF_COMMON_H
#define NYE_BPF_COMMON_H

#include <linux/types.h>

struct bpf_event {
    __u64 ts;
    __u32 pid;
    __u32 ppid;
    __u32 uid;
    __u32 gid;
    char comm[16];
    int retval;
    char filename[128];
};

#endif
