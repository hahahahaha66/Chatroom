#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;
using namespace std::chrono_literals;

class Warning {
public:
    void clearScreenBlackBg() {
        cout << "\033[2J\033[1;1H";      // 清屏并移动
        cout << "\033[40m\033[97m";       // 黑底白字
    }

    string execCmd(const string& cmd) {
        string data;
        FILE* stream = popen(cmd.c_str(), "r");
        if (!stream) return "";
        char buf[256];
        while (fgets(buf, sizeof buf, stream))
            data += buf;
        pclose(stream);
        return data;
    }

    void beep() {
        cout << "\a" << flush;           // 蜂鸣
    }

    void printStackTrace() {
        vector<string> syms = {
            "panic+0x123/0x456",
            "do_page_fault+0x29f/0x4b0",
            "__do_kernel_fault+0x134/0x1f0",
            "exc_page_fault+0x87/0x180",
            "asm_exc_page_fault+0x22/0x30",
            "schedule+0x45/0xe0",
            "worker_thread+0x15a/0x3b0",
            "kthread+0x133/0x170",
            "ret_from_fork+0x1f/0x30"
        };
        uint64_t rsp = 0xffffb0b00080f8f0;
        mt19937_64 rnd(time(nullptr));
        for (auto& s : syms) {
            printf(" [<%016lx>] %s\n", rsp, s.c_str());
            rsp += 0x10 + (rnd() % 0x20);
            this_thread::sleep_for(60ms);
        }
    }

    void countdownReboot(int seconds = 10) {
        for (int i = seconds; i > 0; --i) {
            printf("!!! REBOOT IN %2d SECONDS !!!\n", i);
            beep();
            this_thread::sleep_for(700ms);
        }
        printf(">>> System restarting now... <<<\n");
    }

    void simulatePanic() {
        clearScreenBlackBg();
        srand(time(nullptr));
        cout << "\033[1;31m";  // 红色加粗

        // 初始 Panic 框
        printf("╔══════════════════════════════════════════════════════════════════════════════════════╗\n");
        printf("║                         KERNEL PANIC - CRITICAL SYSTEM ERROR                        ║\n");
        printf("╚══════════════════════════════════════════════════════════════════════════════════════╝\n");
        this_thread::sleep_for(1s);

        // 关键 Panic 信息
        printf("\n[    0.000000] Kernel panic - not syncing: Fatal exception in interrupt\n");
        printf("[    0.000001] CPU: %d PID: %d Comm: %s Tainted: %s\n",
               rand()%8, 2 + rand()%5000, "kworker/0:1", "G    6.1.33-arch1-1 #1");
        printf("[    0.000002] Hardware name: QEMU Standard PC (Q35 + ICH9, 2009)\n");
        printf("[    0.000003] Call Trace:\n");
        printStackTrace();
        this_thread::sleep_for(500ms);

        // 加载更多日志条目，刷屏效果
        vector<string> extraLogs = {
            "EXT4-fs warning: mounting fs with errors, running journal recovery",
            "sd 0:0:0:0: [sda] tag#0 FAILED Result: hostbyte=DID_OK driverbyte=DRIVER_SENSE",
            "Buffer I/O error on dev sda1, logical block 0",
            "end_request: I/O error, dev sda, sector 8",
            "eth0: link down",
            "eth0: link up, 100Mbps, full-duplex, lpa 0x45E1",
            "tcp: too many orphaned sockets",
            "oom-killer: killing process 3456 (chrome) score 987 or sacrifice child",
            "PCIe Bus Error: severity=Corrected, type=Data Link Layer, (Transmitter ID)",
            "audit: type=1400 audit(0.123:45): apparmor=\"DENIED\" operation=\"open\" profile=\"snapd\" name=\"/etc/shadow\"",
            "NETDEV WATCHDOG: eth0: transmit queue 0 timed out",
            "systemd-journald[1]: Failed to rotate journal: Input/output error"
        };
        // 随机刷屏 80 行
        for (int i = 0; i < 80; ++i) {
            int idx = rand() % extraLogs.size();
            double t = (rand() % 1000) / 1000.0 + 0.010 * i;
            printf("[ %8.6f] %s\n", t, extraLogs[idx].c_str());
            auto delay = (i % 2 == 0 ? 30ms : 50ms);
            this_thread::sleep_for(delay);
        }

        // 系统信息
        auto host = execCmd("hostname");
        auto uni  = execCmd("uname -a");
        auto mem  = execCmd("free -m");
        printf("\n--- SYSTEM INFO ---\nHostname: %s", host.c_str());
        printf("Kernel:   %s", uni.c_str());
        printf("Memory:\n%s", mem.c_str());
        this_thread::sleep_for(1s);

        // 再一轮刷屏，模拟更多 I/O、网络、文件系统日志
        for (int i = 0; i < 60; ++i) {
            int idx = rand() % extraLogs.size();
            double t = (rand() % 1000) / 1000.0 + 1.500 + 0.005 * i;
            printf("[ %8.6f] %s\n", t, extraLogs[idx].c_str());
            auto delay = (i % 3 == 0 ? 40ms : 60ms);
            this_thread::sleep_for(delay);
        }

        // 最终倒计时（保留原来15秒）
        countdownReboot(5);
        cout << "\033[0m";  // 恢复颜色
    }
};

int main() {
    Warning w;
    w.simulatePanic();
    return 0;
}
