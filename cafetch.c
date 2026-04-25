#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>

#define INFO_LINES 16
#define ASCII_LINES 8
#define C_CYAN   "\033[1;36m"
#define C_RESET  "\033[0m"
#define C_BOLD   "\033[1m"

void read_sys_file(const char *path, char *buffer, size_t size) {
    FILE *fp = fopen(path, "r");
    if (fp) {
        if (fgets(buffer, size, fp)) buffer[strcspn(buffer, "\n")] = 0;
        fclose(fp);
    } else { strncpy(buffer, "N/A", size); }
}

void get_display_server(char *server, size_t size) {
    if (getenv("WAYLAND_DISPLAY")) strncpy(server, "Wayland", size);
    else if (getenv("DISPLAY")) {
        if (access("/usr/bin/XLibre", F_OK) == 0) strncpy(server, "XLibre (X11)", size);
        else if (access("/usr/bin/Xorg", F_OK) == 0) strncpy(server, "Xorg (X11)", size);
        else strncpy(server, "X11", size);
    } else strncpy(server, "TTY/None", size);
}

void get_de_wm(char *de, size_t size) {
    char *xdg_de = getenv("XDG_CURRENT_DESKTOP");
    char *xdg_sess = getenv("DESKTOP_SESSION");
    
    if (xdg_de) strncpy(de, xdg_de, size);
    else if (xdg_sess) strncpy(de, xdg_sess, size);
    else {
        if (access("/usr/bin/dwm", F_OK) == 0 && getenv("DISPLAY")) strncpy(de, "dwm", size);
        else if (access("/usr/bin/i3", F_OK) == 0) strncpy(de, "i3", size);
        else if (access("/usr/bin/xfce4-session", F_OK) == 0) strncpy(de, "XFCE4", size);
        else if (access("/usr/bin/lxqt-session", F_OK) == 0) strncpy(de, "LXQt", size);
        else if (access("/usr/bin/openbox", F_OK) == 0) strncpy(de, "Openbox", size);
        else strncpy(de, "Unknown", size);
    }
}

void get_init_system(char *init, size_t size) {
    char comm[64] = "";
    read_sys_file("/proc/1/comm", comm, sizeof(comm));
    if (strstr(comm, "systemd")) strncpy(init, "systemd", size);
    else if (access("/run/openrc", F_OK) == 0) strncpy(init, "OpenRC", size);
    else if (access("/run/runit", F_OK) == 0) strncpy(init, "Runit", size);
    else if (access("/run/dinit", F_OK) == 0 || strstr(comm, "dinit")) strncpy(init, "dinit", size);
    else if (access("/run/s6", F_OK) == 0) strncpy(init, "s6", size);
    else strncpy(init, comm, size);
}

int main() {
    char user[64], host[64], os[128], cpu[128], model[128], battery[64], gpu[128], init[64], display[64], de[64];
    struct sysinfo si;
    struct utsname un;
    struct statvfs vfs;

    getlogin_r(user, sizeof(user));
    gethostname(host, sizeof(host));
    uname(&un);
    get_init_system(init, sizeof(init));
    get_display_server(display, sizeof(display));
    get_de_wm(de, sizeof(de));

    strncpy(os, "Linux", sizeof(os));
    FILE *f_os = fopen("/etc/os-release", "r");
    if (f_os) {
        char line[256];
        while (fgets(line, sizeof(line), f_os)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                char *name = line + 13;
                name[strcspn(name, "\"\n")] = 0;
                strncpy(os, name, sizeof(os));
            }
        }
        fclose(f_os);
    }

    read_sys_file("/sys/class/dmi/id/product_name", model, sizeof(model));
    
    FILE *f_cpu = fopen("/proc/cpuinfo", "r");
    cpu[0] = '\0';
    if (f_cpu) {
        char line[256];
        while (fgets(line, sizeof(line), f_cpu)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *name = strchr(line, ':') + 2;
                name[strcspn(name, "\n")] = 0;
                strncpy(cpu, name, sizeof(cpu));
                break;
            }
        }
        fclose(f_cpu);
    }

    FILE *f_gpu = fopen("/sys/class/drm/card0/device/vendor", "r");
    if (f_gpu) {
        char v_id[16]; fgets(v_id, sizeof(v_id), f_gpu); fclose(f_gpu);
        if (strstr(v_id, "0x10de")) strcpy(gpu, "NVIDIA");
        else if (strstr(v_id, "0x8086")) strcpy(gpu, "Intel");
        else if (strstr(v_id, "0x1002")) strcpy(gpu, "AMD");
        else if (strstr(v_id, "0x15ad")) strcpy(gpu, "VMware/VBox");
        else strcpy(gpu, "Unknown");
    } else strcpy(gpu, "N/A");

    statvfs("/", &vfs);
    unsigned long d_t = (vfs.f_blocks * vfs.f_frsize) / 1024 / 1024 / 1024;
    unsigned long d_u = ((vfs.f_blocks - vfs.f_bfree) * vfs.f_frsize) / 1024 / 1024 / 1024;

    sysinfo(&si);
    unsigned long m_u = (si.totalram - si.freeram) * si.mem_unit / 1024 / 1024;
    unsigned long m_t = si.totalram * si.mem_unit / 1024 / 1024;

    const char *arch_art[] = {"      /\\      ","     /  \\     ","    /    \\    ","   /      \\   ","  /   ,,   \\  "," /   |  |   \\ ","/_,,_/  \\_,,_\\","              "};
    const char *tux_art[] = {"    .--.      ","   |o_o |     ","   |:_/ |     ","  //   \\ \\    "," (|     | )   ","/'\\_   _/`\\   ","\\___)=(___/   ","              "};
    const char **art = (strstr(os, "Arch") || strstr(os, "Artix")) ? arch_art : tux_art;

    char info[INFO_LINES][256];
    int c = 0;
    snprintf(info[c++], 256, C_CYAN C_BOLD "%s" C_RESET "@" C_CYAN C_BOLD "%s" C_RESET, user, host);
    snprintf(info[c++], 256, "------------------");
    snprintf(info[c++], 256, C_CYAN "OS:      " C_RESET "%s", os);
    snprintf(info[c++], 256, C_CYAN "Kernel:  " C_RESET "%s", un.release);
    snprintf(info[c++], 256, C_CYAN "Display: " C_RESET "%s", display);
    snprintf(info[c++], 256, C_CYAN "DE/WM:   " C_RESET "%s", de);
    snprintf(info[c++], 256, C_CYAN "Init:    " C_RESET "%s", init);
    snprintf(info[c++], 256, C_CYAN "Uptime:  " C_RESET "%ldh %ldm", si.uptime/3600, (si.uptime%3600)/60);
    snprintf(info[c++], 256, C_CYAN "CPU:     " C_RESET "%s", cpu);
    snprintf(info[c++], 256, C_CYAN "GPU:     " C_RESET "%s", gpu);
    snprintf(info[c++], 256, C_CYAN "Memory:  " C_RESET "%luMiB / %luMiB", m_u, m_t);
    snprintf(info[c++], 256, C_CYAN "Disk:    " C_RESET "%luGB / %luGB", d_u, d_t);
    snprintf(info[c++], 256, "");
    
    char blocks[256] = "";
    for(int i=0; i<8; i++) {
        char tmp[20];
        snprintf(tmp, sizeof(tmp), "\033[4%dm  ", i);
        strcat(blocks, tmp);
    }
    strcat(blocks, C_RESET);
    strncpy(info[c++], blocks, 256);

    printf("\n");
    for (int i = 0; i < c || i < ASCII_LINES; i++) {
        printf("  %-15s %s\n", (i < ASCII_LINES) ? art[i] : "", (i < c) ? info[i] : "");
    }
    printf("\n");

    return 0;
}
