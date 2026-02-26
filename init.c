#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
  mount("none", "/proc", "proc", 0, NULL);
  mount("none", "/sys", "sysfs", 0, NULL);
  mount("none", "/dev", "devtmpfs", 0, NULL);
  mkdir("/dev/pts", 0755);
  mount("none", "/dev/pts", "devpts", 0, NULL);
  mount("none", "/tmp", "tmpfs", 0, NULL);
  setenv("PATH", "/bin:/sbin:/usr/bin:/usr/local/bin", 1);
  setenv("HOME", "/root", 1);
  setenv("TERM", "linux", 1);
  char *argv[] = {"/bin/ball", NULL};
  char *envp[] = {
    "PATH=/bin:/sbin:/usr/bin:/usr/local/bin",
    "HOME=/root",
    "TERM=linux",
    "USER=root",
    NULL
};
  execve("/bin/ball", argv, envp);
  perror("shi.... execve failed");
  reboot(RB_AUTOBOOT);
  return 1;
}