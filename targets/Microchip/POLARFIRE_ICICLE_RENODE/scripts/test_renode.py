#!/usr/bin/env python3
#
# Copyright (c) 2026 Eclipse ThreadX contributors
#
# This program and the accompanying materials are made available
# under the terms of the MIT license which is available at
# https://opensource.org/license/mit.
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

def run_test():
    renode = find_renode()
    script_dir = os.path.dirname(os.path.abspath(__file__))
    target_dir = os.path.dirname(script_dir)
    resc_path = os.path.join(target_dir, "renode", "polarfire_demo.resc").replace("\\", "/")
    
    print(f"[*] Starting headless Renode test using: {renode}")
    print(f"[*] Loading script: {resc_path}")
    
    cmd = [
        renode,
        "--plain",
        "--disable-gui",
        "-e", f"include @{resc_path}"
    ]
    
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1
    )
    
    output_lines = []
    found_ticks = False
    found_alarm = False
    start_time = time.time()
    timeout_seconds = 20.0
    
    try:
        while time.time() - start_time < timeout_seconds:
            line = proc.stdout.readline()
            if not line and proc.poll() is not None:
                break
            if line:
                output_lines.append(line)
                print(line, end="")
                if "ThreadX Ticks" in line:
                    found_ticks = True
                if "OVERTEMP ALARM TRIGGERED" in line:
                    found_alarm = True
                if found_ticks and found_alarm:
                    print("\n[+] SUCCESS: Both ThreadX system ticks and LM75 overtemperature alarm detected!")
                    break
            time.sleep(0.01)
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            proc.kill()
            
    if found_ticks and found_alarm:
        print("[+] Renode headless test PASSED.")
        sys.exit(0)
    else:
        print(f"\n[-] FAILED: Timed out waiting for expected telemetry. (found_ticks={found_ticks}, found_alarm={found_alarm})")
        sys.exit(1)

if __name__ == "__main__":
    run_test()
