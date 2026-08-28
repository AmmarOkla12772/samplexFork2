#!/usr/bin/env python3
#
# Copyright (c) 2026 Eclipse ThreadX contributors
#
# This program and the accompanying materials are made available
# under the terms of the MIT license which is available at
# https://opensource.org/licenses/MIT.
#
# SPDX-License-Identifier: MIT
#

"""
Headless Renode Verification Test for PolarFire SoC Icicle Kit ThreadX Demo
"""

import os
import sys
import time
import subprocess
import shutil
import threading
import queue

# Byte injected into MMUART1 to exercise the PLIC external-interrupt path.
RX_TEST_CHAR = "X"

def find_renode():
    # Check PATH first
    renode_bin = shutil.which("renode")
    if renode_bin:
        return renode_bin
    
    # Common Windows locations
    win_paths = [
        r"C:\Program Files\Renode\renode.exe",
        os.path.expanduser(r"~\AppData\Local\Programs\Renode\renode.exe")
    ]
    for path in win_paths:
        if os.path.isfile(path):
            return path
            
    return "renode"

def reader_thread_fn(pipe, q):
    try:
        for line in iter(pipe.readline, ''):
            q.put(line)
    except Exception:
        pass
    finally:
        pipe.close()

def run_test(test_timeout_mode=False):
    renode = find_renode()
    script_dir = os.path.dirname(os.path.abspath(__file__))
    target_dir = os.path.dirname(script_dir)
    resc_path = os.path.join(target_dir, "renode", "polarfire_demo.resc").replace("\\", "/")
    
    if test_timeout_mode:
        print("[*] Running intentional timeout test mode (2.0s deadline against unresponsive wait)...")
        timeout_seconds = 2.0
    else:
        print(f"[*] Starting headless Renode test using: {renode}")
        print(f"[*] Loading script: {resc_path}")
        timeout_seconds = 25.0
    
    cmd = [
        renode,
        "--plain",
        "--disable-gui",
        "--port", "-1",
        "-e", f"include @{resc_path}"
    ]
    
    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1
    )
    
    output_q = queue.Queue()
    reader_t = threading.Thread(target=reader_thread_fn, args=(proc.stdout, output_q), daemon=True)
    reader_t.start()
    
    output_lines = []
    found_selftests = False
    found_selftest_failure = False
    found_ticks = False
    found_alarm = False
    found_plic_rx = False
    injected_char = False
    start_time = time.time()

    def inject_uart_byte():
        """Deliver a byte to MMUART1 so the PLIC external-interrupt path is
        actually exercised, rather than only having its registers inspected."""
        try:
            proc.stdin.write("sysbus.mmuart1 WriteChar %d\n" % ord(RX_TEST_CHAR))
            proc.stdin.flush()
            return True
        except Exception as exc:
            print("[!] Could not inject UART byte: %s" % exc)
            return False

    try:
        while time.time() - start_time < timeout_seconds:
            try:
                line = output_q.get(timeout=0.1)
                output_lines.append(line)
                print(line, end="")
                if test_timeout_mode:
                    continue

                if "[-] FAIL:" in line or "startup verification test(s) FAILED" in line:
                    found_selftest_failure = True
                if "[SELF-TEST] All startup verification tests PASSED!" in line:
                    found_selftests = True
                if "Ticks:" in line:
                    found_ticks = True
                    # The kernel is running, so the RX interrupt can be serviced.
                    if not injected_char:
                        injected_char = inject_uart_byte()
                if "OVERTEMP ALARM TRIGGERED" in line:
                    found_alarm = True
                if "PLIC IRQ 91 handled" in line:
                    found_plic_rx = True

                if found_selftest_failure:
                    print("\n[-] FAILED: a startup self-test reported a failure.")
                    break
                if found_selftests and found_ticks and found_alarm and found_plic_rx:
                    print("\n[+] SUCCESS: startup self-tests, ThreadX ticks, "
                          "LM75 alarm, and PLIC RX interrupt all verified!")
                    break
            except queue.Empty:
                if proc.poll() is not None:
                    break
    finally:
        try:
            proc.terminate()
            proc.wait(timeout=2)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass

    if test_timeout_mode:
        elapsed = time.time() - start_time
        print(f"\n[+] SUCCESS: Intentional timeout triggered after {elapsed:.2f}s "
              f"and terminated child process cleanly.")
        sys.exit(0)

    checks = {
        "startup self-tests passed": found_selftests and not found_selftest_failure,
        "ThreadX system tick advancing": found_ticks,
        "LM75 overtemperature alarm": found_alarm,
        "PLIC IRQ 91 RX interrupt delivered": found_plic_rx,
    }
    failed = [name for name, ok in checks.items() if not ok]

    if not failed:
        print("[+] Renode headless test PASSED.")
        sys.exit(0)

    print("\n[-] FAILED. Unmet assertions:")
    for name in failed:
        print("      - %s" % name)
    sys.exit(1)

if __name__ == "__main__":
    timeout_test = "--test-timeout" in sys.argv
    run_test(test_timeout_mode=timeout_test)
