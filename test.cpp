#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

using namespace std;
using namespace std::chrono_literals;

const std::string RED     = "\033[31m";
const std::string BOLD    = "\033[1m";
const std::string BLINK   = "\033[5m";
const std::string RESET   = "\033[0m";

void Beep() {
    cout << '\a' << flush;
}

void SlowPrint(const string& msg, int delay_ms = 40) {
    for (char c : msg) {
        cout << c << flush;
        this_thread::sleep_for(chrono::milliseconds(delay_ms));
    }
    cout << endl;
}

string GetCommandOutput(const string& cmd) {
    string data;
    FILE* stream = popen(cmd.c_str(), "r");
    if (!stream) return "N/A";
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stream)) {
        data += buffer;
    }
    pclose(stream);
    return data;
}

// 逐字符输出
void slow_print(const std::string& line, int delay_ms = 10, bool red = false, bool blink = false) {
    std::string prefix;
    if (red) prefix += RED;
    if (blink) prefix += BLINK;
    prefix += BOLD;

    std::cout << prefix;

    for (char c : line) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    std::cout << RESET << std::endl;
}

int main() {
    system("clear");

    std::vector<std::string> lines = {
        "[  104.932001] BUG: kernel NULL pointer dereference, address: 0000000000000000",
        "[  104.932103] #PF: supervisor instruction fetch in kernel mode",
        "[  104.932207] #PF: error_code(0x0010) - not-present page",
        "[  104.932313] PGD 0 P4D 0 ",
        "[  104.932400] Oops: 0010 [#1] SMP NOPTI",
        "[  104.932485] CPU: 3 PID: 2184 Comm: kworker/3:1 Not tainted 6.1.33-arch1-1 #1",
        "[  104.932593] Hardware name: QEMU Standard PC (Q35 + ICH9, 2009)",
        "[  104.932692] RIP: 0010:0x0",
        "[  104.932784] Code: Unable to access opcode bytes at RIP 0x0.",
        "[  104.932879] RSP: 0018:ffffb0b00080f8f0 EFLAGS: 00010246",
        "[  104.932971] Call Trace:",
        "[  104.933055]  <IRQ>",
        "[  104.933137]  panic+0x123/0x456",
        "[  104.933218]  __do_softirq+0xf4/0x2e0",
        "[  104.933302]  irq_exit_rcu+0xa4/0xd0",
        "[  104.933384]  common_interrupt+0xb4/0xd0",
        "[  104.933467]  </IRQ>",
        "[  104.933548]  kernel_thread_helper+0x0/0x10",
        "[  104.933630] ---[ end trace 37f61a0f8481e47a ]---",
        "[  104.933712] " + std::string(RED) + std::string(BLINK) + "Kernel panic - not syncing: Fatal exception" + RESET
    };

    for (const auto& line : lines) {
        bool red = line.find("BUG:") != std::string::npos ||
                   line.find("panic") != std::string::npos ||
                   line.find("Fatal") != std::string::npos;

        bool blink = line.find("panic") != std::string::npos;

        slow_print(line, 5, red, blink);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }

    // 模拟紧急错误
    cout << "\033[1;31m*** KERNEL PANIC - SYSTEM FAILURE ***\033[0m\n";
    SlowPrint("[  0.000001] CRITICAL ERROR: Unable to mount root fs on unknown-block(0,0)");
    SlowPrint("[  0.000002] System halt requested by kernel");
    SlowPrint("[  0.000003] Dumping stack trace...");

    cout << "↘ PID: 1   COMMAND: init\n";
    cout << "↘ PID: 137 systemd-journald\n";
    cout << "↘ PID: 253 Xorg\n";

    this_thread::sleep_for(1s);

    // 获取系统信息
    string hostname = GetCommandOutput("hostname");
    string kernel   = GetCommandOutput("uname -r");
    string time     = GetCommandOutput("date");

    cout << "\n\033[1;36mSYSTEM INFO:\033[0m\n";
    cout << "Hostname: " << hostname;
    cout << "Kernel:   Linux " << kernel;
    cout << "Time:     " << time;

    this_thread::sleep_for(1s);

    // 输出致命错误提示
    for (int i = 0; i < 3; ++i) {
        Beep();
        cout << "\033[5;31m!! FATAL ERROR DETECTED !!\033[0m" << endl;
        this_thread::sleep_for(500ms);
    }

    // 倒计时重启
    for (int i = 5; i > 0; --i) {
        Beep();
        cout << "\033[5;31mREBOOT IN " << i << "...\033[0m" << endl;
        this_thread::sleep_for(1s);
    }

    // 假装“重启”
    cout << "\033[1;32mSystem restarting now...\033[0m\n";
    this_thread::sleep_for(1s);
    cout << "\033[1;33m[ OK ] Rebooting...\033[0m\n";

    return 0;
}
