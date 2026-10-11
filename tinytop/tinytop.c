#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define MAX_PROCS 4096
#define TOP_ROWS 20

struct proc {
    int pid;
    char name[64];
    char state;
    long rss_kb;
};

static int read_comm(int pid, char *name, size_t size, char *state)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *file = fopen(path, "r");
    if (!file)
        return 0;

    char line[1024];
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return 0;
    }
    fclose(file);

    char *open_paren = strchr(line, '(');
    char *close_paren = strrchr(line, ')');
    if (!open_paren || !close_paren || close_paren < open_paren)
        return 0;

    int len = (int)(close_paren - open_paren - 1);
    if (len >= (int)size)
        len = (int)size - 1;
    memcpy(name, open_paren + 1, len);
    name[len] = '\0';

    *state = (close_paren[1] == ' ') ? close_paren[2] : '?';
    return 1;
}

static long read_rss(int pid)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *file = fopen(path, "r");
    if (!file)
        return 0;

    char line[256];
    long rss = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", &rss);
            break;
        }
    }

    fclose(file);
    return rss;
}

static int scan_procs(struct proc *procs)
{
    DIR *dir = opendir("/proc");
    if (!dir)
        return 0;

    int count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) && count < MAX_PROCS) {
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9')
            continue;

        struct proc *p = &procs[count];
        p->pid = atoi(entry->d_name);

        if (!read_comm(p->pid, p->name, sizeof(p->name), &p->state))
            continue;

        p->rss_kb = read_rss(p->pid);
        count++;
    }

    closedir(dir);
    return count;
}

static int by_rss(const void *a, const void *b)
{
    const struct proc *pa = a;
    const struct proc *pb = b;

    if (pa->rss_kb < pb->rss_kb)
        return 1;
    if (pa->rss_kb > pb->rss_kb)
        return -1;
    return 0;
}

static void mem_total(long *total_kb, long *avail_kb)
{
    *total_kb = 0;
    *avail_kb = 0;

    FILE *file = fopen("/proc/meminfo", "r");
    if (!file)
        return;

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "MemTotal:", 9) == 0)
            sscanf(line + 9, "%ld", total_kb);
        else if (strncmp(line, "MemAvailable:", 13) == 0)
            sscanf(line + 13, "%ld", avail_kb);
    }

    fclose(file);
}

static void draw(struct proc *procs, int count)
{
    double loads[3] = { 0, 0, 0 };
    getloadavg(loads, 3);

    long total_kb, avail_kb;
    mem_total(&total_kb, &avail_kb);

    printf("\033[H\033[J");
    printf("tinytop  %d procs   load %.2f %.2f %.2f   mem %ld/%ld MiB\n",
           count, loads[0], loads[1], loads[2],
           (total_kb - avail_kb) / 1024, total_kb / 1024);
    printf("%8s  %5s  %10s  %s\n", "PID", "STATE", "RSS", "COMMAND");

    int rows = count < TOP_ROWS ? count : TOP_ROWS;

    for (int i = 0; i < rows; i++) {
        printf("%8d  %5c  %7ld KiB  %s\n",
               procs[i].pid, procs[i].state, procs[i].rss_kb, procs[i].name);
    }
}

int main(int argc, char **argv)
{
    (void)argv;
    int once = !isatty(STDOUT_FILENO) || argc > 1;

    struct termios saved;
    int have_tty = isatty(STDIN_FILENO);

    if (!once && have_tty) {
        tcgetattr(STDIN_FILENO, &saved);

        struct termios raw = saved;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    static struct proc procs[MAX_PROCS];

    for (;;) {
        int count = scan_procs(procs);
        qsort(procs, count, sizeof(procs[0]), by_rss);
        draw(procs, count);

        if (once)
            break;

        struct timespec delay = { .tv_sec = 1, .tv_nsec = 0 };
        nanosleep(&delay, NULL);

        char key;
        if (read(STDIN_FILENO, &key, 1) == 1 && (key == 'q' || key == 'Q'))
            break;
    }

    if (!once && have_tty)
        tcsetattr(STDIN_FILENO, TCSANOW, &saved);

    return 0;
}