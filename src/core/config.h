#ifndef NYE_CONFIG_H
#define NYE_CONFIG_H

int config_load(const char *path);
const char *config_get_database(void);
const char *config_get_socket_path(void);
const char *config_get_log_file(void);
int config_get_ebpf_ringbuf_size(void);

#endif
