#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>

using namespace std;

class Warning
{
public:
    void clearScreenBlackBg() {
        cout << "\033[2J\033[1;1H";                  // 清屏并移动光标到左上角
        cout << "\033[40m\033[97m";                 // 黑底白字
    }

    string execCmd(const string& cmd) {
        string data;
        FILE* stream;
        const int max_buffer = 256;
        char buffer[max_buffer];
        stream = popen(cmd.c_str(), "r");
        if (stream) {
            while (fgets(buffer, max_buffer, stream) != nullptr)
                data.append(buffer);
            pclose(stream);
        }
        return data;
    }

    void beep() {
        // 方式一：终端蜂鸣
        cout << "\a" << flush;
        // 方式二：调用播放命令
        // system("aplay /usr/share/sounds/freedesktop/stereo/alarm-clock-elapsed.oga 2>/dev/null &");
    }

    void printStackTrace() {
        vector<string> symbols = {
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
        for (const auto& sym : symbols) {
            printf(" [<%lx>]  %s\n", rsp, sym.c_str());
            rsp += 0x10 + (rand() % 10);
            this_thread::sleep_for(chrono::milliseconds(80));
        }
    }

    void countdownReboot() {
        for (int i = 5; i > 0; --i) {
            cout << "REBOOTING IN " << i << "...\n";
            beep();
            this_thread::sleep_for(chrono::milliseconds(700));
        }
        cout << "System restarting now...\n";
    }

    void simulatePanic() {
        clearScreenBlackBg();

        srand(time(nullptr));
        auto hostname = execCmd("hostname");
        auto uname = execCmd("uname -a");
        auto mem = execCmd("free -m");

        cout << "[ " << fixed << 104.932 + rand() % 10 << "] BUG: kernel NULL pointer dereference, address: 0000000000000000\n";
        cout << "[ " << 104.932 + rand() % 10 << "] #PF: supervisor instruction fetch in kernel mode\n";
        cout << "[ " << 104.932 + rand() % 10 << "] #PF: error_code(0x0010) - not-present page\n";
        cout << "[ " << 104.932 + rand() % 10 << "] PGD 0 P4D 0\n";
        cout << "[ " << 104.932 + rand() % 10 << "] Oops: 0010 [#1] SMP NOPTI\n";
        cout << "[ " << 104.932 + rand() % 10 << "] CPU: " << (rand() % 8) << " PID: " << 1000 + rand() % 4000
            << " Comm: kworker/" << rand() % 8 << ":1 Not tainted 6.1.33-arch1-1 #1\n";
        cout << "[ " << 104.932 + rand() % 10 << "] Hardware name: QEMU Standard PC (Q35 + ICH9, 2009)\n";
        cout << "[ " << 104.932 + rand() % 10 << "] RIP: 0010:0x0\n";
        cout << "[ " << 104.932 + rand() % 10 << "] Code: Unable to access opcode bytes at RIP 0x0.\n";
        cout << "[ " << 104.932 + rand() % 10 << "] RSP: 0018:ffffb0b00080f8f0 EFLAGS: 00010246\n";

        printStackTrace();

        cout << "---[ end trace " << hex << rand() << dec << " ]---\n";
        cout << "Kernel panic - not syncing: Fatal exception\n";
        cout << "SYSTEM FAILURE: Unable to recover\n";
        cout << "CRITICAL ERROR: Unable to mount root fs on unknown-block(0,0)\n";
        cout << "System halt requested by kernel\n";

        this_thread::sleep_for(chrono::milliseconds(500));

        cout << "Dumping stack trace...\n";
        cout << "↘ PID: 1   COMMAND: init\n";
        cout << "↘ PID: 137 systemd-journald\n";
        cout << "↘ PID: 253 Xorg\n";
        cout << "sh: line 1: hostname: command not found\n\n";

        cout << "SYSTEM INFO:\n";
        cout << "Hostname: " << hostname;
        cout << "Kernel:   " << uname;
        cout << "Memory Info:\n" << mem;

        for (int i = 0; i < 3; ++i) {
            cout << "!! FATAL ERROR DETECTED !!\n";
            this_thread::sleep_for(chrono::milliseconds(300));
        }

        countdownReboot();
    }
};


