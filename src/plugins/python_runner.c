#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "core/logger.h"

int python_plugin_run(const char *script, const char *json_event)
{
    if (!script) return -1;
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        execlp("python3", "python3", script, (char *)NULL);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        int rc = WEXITSTATUS(status);
        if (rc != 0) nulleye_log(NYE_LOG_WARN, "python plugin %s exited %d", script, rc);
        return rc == 0 ? 0 : -1;
    }
    return -1;
}
