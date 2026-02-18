#define _GNU_SOURCE
#include "utils/sandbox.h"
#include "core/logger.h"
#include <sys/resource.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>

int sandbox_apply_limits(void)
{
    struct rlimit rl;
    rl.rlim_cur = rl.rlim_max = 65536;
    if (setrlimit(RLIMIT_NOFILE, &rl) != 0) {
        nulleye_log(NYE_LOG_WARN, "setrlimit(RLIMIT_NOFILE) failed: %s", strerror(errno));
    }
    rl.rlim_cur = rl.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rl) != 0) {
        nulleye_log(NYE_LOG_WARN, "setrlimit(RLIMIT_CORE) failed: %s", strerror(errno));
    }
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) {
        nulleye_log(NYE_LOG_WARN, "prctl(NO_NEW_PRIVS) failed: %s", strerror(errno));
    }
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT) != 0) {
        nulleye_log(NYE_LOG_WARN, "seccomp strict not available or failed: %s", strerror(errno));
    } else {
        nulleye_log(NYE_LOG_INFO, "seccomp strict enabled");
    }
    struct passwd *pw = getpwnam("nobody");
    if (pw) {
        if (setgid(pw->pw_gid) != 0) nulleye_log(NYE_LOG_WARN, "setgid failed: %s", strerror(errno));
        if (setuid(pw->pw_uid) != 0) nulleye_log(NYE_LOG_WARN, "setuid failed: %s", strerror(errno));
    }
    return 0;
}
