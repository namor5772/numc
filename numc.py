"""Tkinter GUI wrapper around numc.exe — the Numbers round solver.

Enter the six numbers and the target, press [Calculate], and the solver's
output appears in the text box below. The search itself runs in the compiled
C++ executable (x64/Release/numc.exe), so build the Release|x64 configuration
first (see README.md).

Run with:  python numc.py
"""

import queue
import subprocess
import sys
import threading
import time
import tkinter as tk
from pathlib import Path
from tkinter import ttk

EXE_PATH = Path(__file__).resolve().parent / "x64" / "Release" / "numc.exe"
DEFAULT_NUMBERS = ("100", "75", "25", "10", "7", "5")
DEFAULT_TARGET = "666"
INT32_MIN, INT32_MAX = -2**31, 2**31 - 1
POLL_MS = 100
RUN_TIMEOUT_S = 600


def tidy(text):
    """Strip the trailing tabs numc.exe pads its lines with, and outer blanks."""
    lines = [line.rstrip() for line in text.splitlines()]
    while lines and not lines[0]:
        lines.pop(0)
    while lines and not lines[-1]:
        lines.pop()
    return "\n".join(lines) + ("\n" if lines else "")


class NumcApp:
    def __init__(self, root):
        self.root = root
        self.result_queue = queue.Queue()
        self.running = False
        self.start_time = 0.0

        root.title("numc — Numbers Round Solver")
        root.minsize(620, 380)

        main = ttk.Frame(root, padding=10)
        main.grid(sticky="nsew")
        root.rowconfigure(0, weight=1)
        root.columnconfigure(0, weight=1)
        main.rowconfigure(2, weight=1)
        main.columnconfigure(0, weight=1)

        # input fields: six numbers, the target, and the Calculate button
        inputs = ttk.Frame(main)
        inputs.grid(row=0, column=0, sticky="w")

        ttk.Label(inputs, text="Numbers:").grid(row=0, column=0, padx=(0, 6))
        self.number_entries = []
        for i, value in enumerate(DEFAULT_NUMBERS):
            entry = ttk.Entry(inputs, width=6, justify="center")
            entry.insert(0, value)
            entry.grid(row=0, column=1 + i, padx=2)
            self.number_entries.append(entry)

        ttk.Label(inputs, text="Target:").grid(row=0, column=7, padx=(14, 6))
        self.target_entry = ttk.Entry(inputs, width=8, justify="center")
        self.target_entry.insert(0, DEFAULT_TARGET)
        self.target_entry.grid(row=0, column=8)

        self.calc_button = ttk.Button(inputs, text="Calculate", command=self.calculate)
        self.calc_button.grid(row=0, column=9, padx=(14, 0))

        # status line between the inputs and the output box
        self.status_var = tk.StringVar(value="Ready")
        self.status_label = ttk.Label(main, textvariable=self.status_var)
        self.status_label.grid(row=1, column=0, sticky="w", pady=(8, 4))

        # scrollable read-only output box
        outframe = ttk.Frame(main)
        outframe.grid(row=2, column=0, sticky="nsew")
        outframe.rowconfigure(0, weight=1)
        outframe.columnconfigure(0, weight=1)

        self.output = tk.Text(outframe, width=90, height=24, wrap="none",
                              font=("Consolas", 10), state="disabled")
        self.output.grid(row=0, column=0, sticky="nsew")
        yscroll = ttk.Scrollbar(outframe, orient="vertical", command=self.output.yview)
        yscroll.grid(row=0, column=1, sticky="ns")
        xscroll = ttk.Scrollbar(outframe, orient="horizontal", command=self.output.xview)
        xscroll.grid(row=1, column=0, sticky="ew")
        self.output.configure(yscrollcommand=yscroll.set, xscrollcommand=xscroll.set)

        root.bind("<Return>", lambda _event: self.calculate())
        self.number_entries[0].focus_set()

    def read_args(self):
        """Validate the seven input fields; return the argument strings or None."""
        fields = [(f"number {i + 1}", entry) for i, entry in enumerate(self.number_entries)]
        fields.append(("target", self.target_entry))
        args = []
        for name, entry in fields:
            text = entry.get().strip()
            try:
                value = int(text)
            except ValueError:
                shown = text if text else "(empty)"
                self.show_status(f"{name}: '{shown}' is not a valid integer.", error=True)
                entry.focus_set()
                entry.select_range(0, "end")
                return None
            if not INT32_MIN <= value <= INT32_MAX:
                self.show_status(f"{name}: {value} does not fit in a 32-bit integer.", error=True)
                entry.focus_set()
                entry.select_range(0, "end")
                return None
            args.append(str(value))
        return args

    def calculate(self):
        if self.running:
            return
        args = self.read_args()
        if args is None:
            return
        self.running = True
        self.calc_button.state(["disabled"])
        self.set_output("")
        self.show_status("Calculating…")
        self.start_time = time.perf_counter()
        threading.Thread(target=self._worker, args=(args,), daemon=True).start()
        self.root.after(POLL_MS, self._poll_result)

    def _worker(self, args):
        """Run numc.exe off the UI thread and post the result to the queue."""
        try:
            flags = subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0
            proc = subprocess.run([str(EXE_PATH), *args], capture_output=True,
                                  text=True, timeout=RUN_TIMEOUT_S, creationflags=flags)
        except FileNotFoundError:
            self.result_queue.put(("error", f"{EXE_PATH} not found.\n\n"
                                   "Build the Release|x64 configuration first (see README.md)."))
        except subprocess.TimeoutExpired:
            self.result_queue.put(("error", f"numc.exe did not finish within {RUN_TIMEOUT_S} s."))
        except OSError as exc:
            self.result_queue.put(("error", f"Failed to run numc.exe:\n{exc}"))
        else:
            text = tidy(proc.stdout)
            if proc.stderr.strip():
                text += "\n" + proc.stderr.strip() + "\n"
            if proc.returncode != 0:
                text += f"\n[numc.exe exited with code {proc.returncode}]\n"
            self.result_queue.put(("ok", text))

    def _poll_result(self):
        try:
            kind, text = self.result_queue.get_nowait()
        except queue.Empty:
            elapsed = time.perf_counter() - self.start_time
            self.show_status(f"Calculating… {elapsed:.0f} s")
            self.root.after(POLL_MS, self._poll_result)
            return
        elapsed = time.perf_counter() - self.start_time
        self.set_output(text)
        if kind == "ok":
            self.show_status(f"Done in {elapsed:.1f} s")
        else:
            self.show_status("Error — see output below.", error=True)
        self.running = False
        self.calc_button.state(["!disabled"])

    def set_output(self, text):
        self.output.configure(state="normal")
        self.output.delete("1.0", "end")
        self.output.insert("1.0", text)
        self.output.configure(state="disabled")

    def show_status(self, message, error=False):
        self.status_var.set(message)
        self.status_label.configure(foreground="red" if error else "")


def main():
    if sys.platform == "win32":
        try:
            from ctypes import windll
            windll.shcore.SetProcessDpiAwareness(1)  # crisp text on high-DPI displays
        except Exception:
            pass
    root = tk.Tk()
    icon = Path(__file__).resolve().parent / "numc.ico"
    if icon.exists():
        try:
            root.iconbitmap(str(icon))
        except tk.TclError:
            pass
    NumcApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
