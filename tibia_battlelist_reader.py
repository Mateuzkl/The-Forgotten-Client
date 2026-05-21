#!/usr/bin/env python3
"""
tibia_battlelist_reader.py

Lists every populated entry in the Tibia 8.60 BattleList. Unlike
tibia_player_reader_gui.py (which only finds the local player), this
walks all 250 slots and shows everyone visible around you.

Layout matches Tibia 8.60 / TFC (when running with elfbot_compat.cpp's
shim writes):
  BattleList starts at  0x0063FEF8
  Slot stride            0xA8  (168 bytes)
  Maximum slots          250

Within each slot (offsets):
  +0   Id           u32
  +3   Type         u8  (also the high byte of Id)
  +4   Name         char[32]
  +36  X            u32
  +40  Y            u32
  +44  Z            u32
  +76  IsWalking    u32
  +80  Direction    u32
  +96  Outfit       u32  (looktype)
  +100 ColorHead    u32
  +104 ColorBody    u32
  +108 ColorLegs    u32
  +112 ColorFeet    u32
  +116 Addon        u32
  +120 Light        u32
  +124 LightColor   u32
  +132 BlackSquare  u32
  +136 HpBar%       u8
  +140 WalkSpeed    u32
  +144 IsVisible    u32
  +148 Skull        u8

Install: py -m pip install psutil
Run:     py tibia_battlelist_reader.py
"""

import ctypes
from ctypes import wintypes
import struct
import sys
import tkinter as tk
from tkinter import ttk, messagebox

try:
    import psutil
except ImportError:
    print("Missing dependency. Install with:  py -m pip install psutil")
    sys.exit(1)

PROCESS_VM_READ = 0x0010
PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
SIZE_T = ctypes.c_size_t

kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
kernel32.OpenProcess.restype = wintypes.HANDLE
kernel32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
kernel32.ReadProcessMemory.argtypes = [
    wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, SIZE_T,
    ctypes.POINTER(SIZE_T),
]
kernel32.ReadProcessMemory.restype = wintypes.BOOL
kernel32.CloseHandle.argtypes = [wintypes.HANDLE]

# Tibia 8.60 layout
PLAYER_ID_ADDR    = 0x0063FE98
BATTLELIST_START  = 0x0063FEF8
BATTLELIST_STEP   = 0xA8
BATTLELIST_MAX    = 250

CREATURE_OFFSETS = {
    "id":         (0,   "u32"),
    "type":       (3,   "u8"),
    "name":       (4,   "str32"),
    "x":          (36,  "u32"),
    "y":          (40,  "u32"),
    "z":          (44,  "u32"),
    "walking":    (76,  "u32"),
    "direction":  (80,  "u32"),
    "outfit":     (96,  "u32"),
    "colorHead":  (100, "u32"),
    "colorBody":  (104, "u32"),
    "colorLegs":  (108, "u32"),
    "colorFeet":  (112, "u32"),
    "addon":      (116, "u32"),
    "light":      (120, "u32"),
    "lightColor": (124, "u32"),
    "hpBar":      (136, "u8"),
    "speed":      (140, "u32"),
    "visible":    (144, "u32"),
    "skull":      (148, "u8"),
}


class MemReader:
    def __init__(self, pid):
        access = PROCESS_VM_READ | PROCESS_QUERY_LIMITED_INFORMATION
        self.handle = kernel32.OpenProcess(access, False, pid)
        if not self.handle:
            raise OSError(ctypes.get_last_error(),
                          f"OpenProcess({pid}) failed. Run as Administrator?")
        self.pid = pid

    def close(self):
        if self.handle:
            kernel32.CloseHandle(self.handle)
            self.handle = None

    def read(self, address, size):
        buf = ctypes.create_string_buffer(size)
        n = SIZE_T(0)
        ok = kernel32.ReadProcessMemory(self.handle, ctypes.c_void_p(address),
                                         buf, size, ctypes.byref(n))
        if not ok or n.value != size:
            return None
        return buf.raw[:n.value]

    def u8(self, a):
        b = self.read(a, 1)
        return b[0] if b else 0

    def u32(self, a):
        b = self.read(a, 4)
        return struct.unpack("<I", b)[0] if b else 0

    def cstr(self, a, maxlen=32):
        b = self.read(a, maxlen)
        if not b:
            return ""
        return b.split(b"\x00", 1)[0].decode("latin-1", "replace")


def read_slot(mem, slot_index):
    base = BATTLELIST_START + slot_index * BATTLELIST_STEP
    out = {"slot": slot_index, "base": base}
    for key, (off, kind) in CREATURE_OFFSETS.items():
        if kind == "u32":
            out[key] = mem.u32(base + off)
        elif kind == "u8":
            out[key] = mem.u8(base + off)
        elif kind == "str32":
            out[key] = mem.cstr(base + off, 32)
    return out


def find_tibia_processes():
    rows = []
    for p in psutil.process_iter(["pid", "name", "exe"]):
        n = (p.info.get("name") or "").lower()
        if "tibia" in n:
            rows.append((p.info["pid"], p.info["name"], p.info.get("exe") or ""))
    return rows


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Tibia BattleList Reader -- ALL Slots")
        self.geometry("1100x650")
        self.mem = None
        self.local_id = 0

        top = ttk.Frame(self, padding=8)
        top.pack(fill="x")

        ttk.Label(top, text="Tibia.exe process:").pack(side="left")
        self.pid_var = tk.StringVar()
        self.pid_combo = ttk.Combobox(top, textvariable=self.pid_var, width=70)
        self.pid_combo.pack(side="left", padx=6)
        ttk.Button(top, text="Refresh", command=self.refresh_pids).pack(side="left", padx=3)
        ttk.Button(top, text="Connect", command=self.connect).pack(side="left", padx=3)
        ttk.Button(top, text="Disconnect", command=self.disconnect).pack(side="left", padx=3)

        self.status = ttk.Label(top, text="Pick the Tibia.exe process and click Connect.")
        self.status.pack(side="left", padx=20)

        cols = ("slot", "id", "name", "x", "y", "z", "outfit",
                "addon", "hp", "speed", "visible", "skull", "dir")
        self.tree = ttk.Treeview(self, columns=cols, show="headings", height=24)
        widths = {"slot": 50, "id": 90, "name": 180, "x": 60, "y": 60, "z": 40,
                  "outfit": 70, "addon": 60, "hp": 50, "speed": 60,
                  "visible": 60, "skull": 50, "dir": 50}
        for c in cols:
            self.tree.heading(c, text=c)
            self.tree.column(c, width=widths.get(c, 80),
                             anchor="center" if c not in ("name",) else "w")
        self.tree.pack(fill="both", expand=True, padx=8, pady=8)

        self.refresh_pids()
        self.after(800, self.tick)

    def refresh_pids(self):
        rows = find_tibia_processes()
        items = [f"{pid}  {name}  {exe}" for pid, name, exe in rows]
        self.pid_combo["values"] = items
        if items and not self.pid_var.get():
            self.pid_var.set(items[0])
        self.status.config(text=f"Found {len(rows)} Tibia process(es).")

    def connect(self):
        sel = self.pid_var.get().strip()
        if not sel:
            return
        try:
            pid = int(sel.split()[0])
            self.disconnect()
            self.mem = MemReader(pid)
            self.status.config(text=f"Connected to PID {pid}.")
        except Exception as e:
            messagebox.showerror("Connect failed", str(e))

    def disconnect(self):
        if self.mem:
            self.mem.close()
            self.mem = None
        self.tree.delete(*self.tree.get_children())

    def tick(self):
        if self.mem:
            try:
                self.update_list()
            except Exception as e:
                self.status.config(text=f"Read error: {e}")
        self.after(500, self.tick)

    def update_list(self):
        local_id = self.mem.u32(PLAYER_ID_ADDR)
        self.local_id = local_id
        rows = []
        for i in range(BATTLELIST_MAX):
            slot = read_slot(self.mem, i)
            if slot["id"] == 0:
                continue   # empty
            rows.append(slot)

        # Replace tree contents
        self.tree.delete(*self.tree.get_children())
        for s in rows:
            tag = "self" if s["id"] == local_id else ""
            self.tree.insert("", "end", values=(
                s["slot"], s["id"], s["name"], s["x"], s["y"], s["z"],
                s["outfit"], s["addon"], s["hpBar"], s["speed"],
                s["visible"], s["skull"], s["direction"]
            ), tags=(tag,))
        self.tree.tag_configure("self", background="#fff2c4")

        self.status.config(text=(
            f"PID {self.mem.pid}  |  Local Player Id 0x{local_id:08X} ({local_id})  "
            f"|  populated slots: {len(rows)} / {BATTLELIST_MAX}"
        ))


def main():
    a = App()
    a.protocol("WM_DELETE_WINDOW", lambda: (a.disconnect(), a.destroy()))
    a.mainloop()


if __name__ == "__main__":
    main()
