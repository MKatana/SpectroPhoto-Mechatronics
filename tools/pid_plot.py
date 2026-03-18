#!/usr/bin/env python3
import argparse
import csv
import math
import threading
import time
from collections import deque
from queue import Empty, Queue

import matplotlib.pyplot as plt
from matplotlib.widgets import Button, TextBox
import serial


STATE_QUERY = "TEMP:PID:STATE?"


class ScpiClient:
    def __init__(self, port: str, baudrate: int, timeout: float) -> None:
        self.ser = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)
        self.lock = threading.Lock()

    def send(self, command: str) -> None:
        with self.lock:
            self.ser.write((command.strip() + "\n").encode("ascii"))
            self.ser.flush()

    def query(self, command: str) -> str:
        with self.lock:
            self.ser.reset_input_buffer()
            self.ser.write((command.strip() + "\n").encode("ascii"))
            self.ser.flush()
            return self.ser.readline().decode("utf-8", errors="replace").strip()

    def close(self) -> None:
        with self.lock:
            if self.ser.is_open:
                self.ser.close()


class Poller(threading.Thread):
    def __init__(self, app, interval_s: float, out_queue: Queue) -> None:
        super().__init__(daemon=True)
        self.app = app
        self.interval_s = interval_s
        self.out_queue = out_queue
        self.running = True

    def run(self) -> None:
        while self.running:
            started = time.time()
            try:
                line = self.app.query_pid_state_line()
                self.out_queue.put(("state", started, line))
            except Exception as exc:  # pragma: no cover
                self.out_queue.put(("error", started, str(exc)))

            elapsed = time.time() - started
            time.sleep(max(0.0, self.interval_s - elapsed))

    def stop(self) -> None:
        self.running = False


class PidPlotApp:
    def __init__(self, client: ScpiClient, poll_interval_s: float, history_s: float) -> None:
        self.client = client
        self.history_s = history_s
        self.queue: Queue = Queue()
        self.poller = Poller(self, poll_interval_s, self.queue)
        self.last_redraw_s = 0.0

        self.t = deque()
        self.temp = deque()
        self.setpoint = deque()
        self.error = deque()
        self.integral = deque()
        self.derivative = deque()
        self.output = deque()
        self.heater = deque()
        self.fan = deque()
        self.fault = deque()

        self.start_time = time.time()
        self.last_status = "Idle"
        self.timer = None

        self.fig = plt.figure(figsize=(12, 8))
        self.ax_temp = self.fig.add_axes([0.08, 0.33, 0.64, 0.60])
        self.ax_pid = self.fig.add_axes([0.08, 0.08, 0.64, 0.18])

        self.ax_temp.set_title("Spectrophoto PID Tuning")
        self.ax_temp.set_ylabel("Temperature [C]")
        self.ax_pid.set_ylabel("PID internals")
        self.ax_pid.set_xlabel("Time [s]")

        (self.line_temp,) = self.ax_temp.plot([], [], label="Temperature")
        (self.line_setpoint,) = self.ax_temp.plot([], [], label="Setpoint")
        (self.line_error,) = self.ax_pid.plot([], [], label="Error")
        (self.line_integral,) = self.ax_pid.plot([], [], label="Integral")
        (self.line_derivative,) = self.ax_pid.plot([], [], label="Derivative")
        (self.line_output,) = self.ax_pid.plot([], [], label="Output %")

        self.ax_temp.legend(loc="upper left")
        self.ax_pid.legend(loc="upper left", ncol=4, fontsize=8)

        self.status_text = self.fig.text(0.08, 0.95, "Status: starting", fontsize=10)
        self.digital_text = self.fig.text(0.74, 0.90, "", fontsize=10, family="monospace")

        self._build_controls()

    def _build_controls(self) -> None:
        self.kp_box = TextBox(self.fig.add_axes([0.76, 0.74, 0.18, 0.05]), "Kp", initial="")
        self.ki_box = TextBox(self.fig.add_axes([0.76, 0.66, 0.18, 0.05]), "Ki", initial="")
        self.kd_box = TextBox(self.fig.add_axes([0.76, 0.58, 0.18, 0.05]), "Kd", initial="")
        self.sp_box = TextBox(self.fig.add_axes([0.76, 0.50, 0.18, 0.05]), "Setpoint", initial="")

        self.apply_btn = Button(self.fig.add_axes([0.76, 0.42, 0.18, 0.05]), "Apply Gains")
        self.control_on_btn = Button(self.fig.add_axes([0.76, 0.34, 0.085, 0.05]), "Control ON")
        self.control_off_btn = Button(self.fig.add_axes([0.855, 0.34, 0.085, 0.05]), "Control OFF")
        self.refresh_btn = Button(self.fig.add_axes([0.76, 0.26, 0.18, 0.05]), "Refresh Params")

        self.apply_btn.on_clicked(self.apply_values)
        self.control_on_btn.on_clicked(lambda event: self.send_command("TEMP:CONTROL 1"))
        self.control_off_btn.on_clicked(lambda event: self.send_command("TEMP:CONTROL 0"))
        self.refresh_btn.on_clicked(lambda event: self.refresh_parameters())

    def send_command(self, command: str) -> None:
        try:
            self.client.send(command)
            self.last_status = f"Sent: {command}"
        except Exception as exc:  # pragma: no cover
            self.last_status = f"Send failed: {exc}"

    def query_value(self, command: str) -> str:
        try:
            value = self.client.query(command)
            self.last_status = f"Query OK: {command}"
            return value
        except Exception as exc:  # pragma: no cover
            self.last_status = f"Query failed: {exc}"
            return ""

    def query_pid_state_line(self) -> str:
        deadline = time.time() + 1.5

        while time.time() < deadline:
            line = self.query_value(STATE_QUERY)
            if self.parse_state(line) is not None:
                return line

        self.last_status = "PID state query timed out"
        return ""

    def refresh_parameters(self) -> None:
        self.kp_box.set_val(self.query_value("TEMP:KP?"))
        self.ki_box.set_val(self.query_value("TEMP:KI?"))
        self.kd_box.set_val(self.query_value("TEMP:KD?"))
        state_line = self.query_pid_state_line()
        state = self.parse_state(state_line)
        if state is not None:
            self.sp_box.set_val(f"{state['setpoint']:.3f}")

    def apply_values(self, event) -> None:
        entries = [
            ("TEMP:KP", self.kp_box.text),
            ("TEMP:KI", self.ki_box.text),
            ("TEMP:KD", self.kd_box.text),
            ("TEMP", self.sp_box.text),
        ]

        for cmd, value in entries:
            value = value.strip()
            if value:
                self.send_command(f"{cmd} {value}")

    def start(self) -> None:
        self.refresh_parameters()
        self.poller.start()
        self.timer = self.fig.canvas.new_timer(interval=250)
        self.timer.add_callback(self.update_plot)
        self.timer.start()
        self.fig.canvas.mpl_connect("close_event", self.on_close)
        plt.show()

    def on_close(self, event) -> None:
        self.poller.stop()
        self.client.close()

    def update_plot(self) -> None:
        while True:
            try:
                kind, timestamp, payload = self.queue.get_nowait()
            except Empty:
                break

            if kind == "error":
                self.last_status = f"Poll failed: {payload}"
                continue

            parsed = self.parse_state(payload)
            if parsed is None:
                self.last_status = f"Ignored non-state line: {payload}"
                continue

            t_rel = timestamp - self.start_time
            self.t.append(t_rel)
            self.temp.append(parsed["temperature"])
            self.setpoint.append(parsed["setpoint"])
            self.error.append(parsed["error"])
            self.integral.append(parsed["integral"])
            self.derivative.append(parsed["derivative"])
            self.output.append(parsed["output"])
            self.heater.append(parsed["heater"])
            self.fan.append(parsed["fan"])
            self.fault.append(parsed["fault"])
            self.last_status = "Polling OK"

        self.trim_history()

        now = time.time()
        if (now - self.last_redraw_s) >= 0.25:
            self.redraw()
            self.last_redraw_s = now

    def trim_history(self) -> None:
        if not self.t:
            return

        cutoff = self.t[-1] - self.history_s
        while self.t and self.t[0] < cutoff:
            self.t.popleft()
            self.temp.popleft()
            self.setpoint.popleft()
            self.error.popleft()
            self.integral.popleft()
            self.derivative.popleft()
            self.output.popleft()
            self.heater.popleft()
            self.fan.popleft()
            self.fault.popleft()

    def redraw(self) -> None:
        if not self.t:
            return

        self.line_temp.set_data(self.t, self.temp)
        self.line_setpoint.set_data(self.t, self.setpoint)
        self.line_error.set_data(self.t, self.error)
        self.line_integral.set_data(self.t, self.integral)
        self.line_derivative.set_data(self.t, self.derivative)
        self.line_output.set_data(self.t, self.output)

        self.ax_temp.set_xlim(max(0.0, self.t[0]), self.t[-1] + 0.1)
        self.ax_pid.set_xlim(max(0.0, self.t[0]), self.t[-1] + 0.1)

        if len(self.t) > 1:
            temp_values = [v for v in list(self.temp) + list(self.setpoint) if isinstance(v, (int, float)) and math.isfinite(v)]
            if temp_values:
                temp_min = min(temp_values)
                temp_max = max(temp_values)
                pad = max(0.5, (temp_max - temp_min) * 0.1)
                self.ax_temp.set_ylim(temp_min - pad, temp_max + pad)

            pid_values = [v for v in (list(self.error) + list(self.integral) + list(self.derivative) + list(self.output))
                          if isinstance(v, (int, float)) and math.isfinite(v)]
            if pid_values:
                pid_min = min(pid_values)
                pid_max = max(pid_values)
                pid_pad = max(0.5, (pid_max - pid_min) * 0.1)
                self.ax_pid.set_ylim(pid_min - pid_pad, pid_max + pid_pad)

        self.status_text.set_text(f"Status: {self.last_status}")
        self.digital_text.set_text(
            "\n".join(
                [
                    f"Heater: {self.heater[-1]}",
                    f"Fan:    {self.fan[-1]}",
                    f"Fault:  {self.fault[-1]}",
                    f"Output: {self.output[-1]:.1f} %",
                ]
            )
        )
        self.fig.canvas.draw_idle()

    @staticmethod
    def parse_state(line: str):
        try:
            row = next(csv.reader([line]))
        except Exception:
            return None

        if len(row) != 10:
            return None

        def parse_float(text: str) -> float:
            return float("nan") if text == "NaN" else float(text)

        try:
            return {
                "temperature": parse_float(row[0]),
                "setpoint": float(row[1]),
                "error": float(row[2]),
                "integral": float(row[3]),
                "derivative": float(row[4]),
                "output": float(row[5]),
                "heater": int(row[6]),
                "fan": int(row[7]),
                "fault": int(row[8]),
                "valid": int(row[9]),
            }
        except ValueError:
            return None


def main() -> None:
    parser = argparse.ArgumentParser(description="Live PID plotter for Spectrophoto firmware")
    parser.add_argument("--port", required=True, help="Serial port, e.g. COM5")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate")
    parser.add_argument("--poll-ms", type=int, default=1000, help="Polling interval in milliseconds")
    parser.add_argument("--history-s", type=float, default=300.0, help="Seconds of history to display")
    args = parser.parse_args()

    client = ScpiClient(args.port, args.baud, timeout=0.5)
    app = PidPlotApp(client, args.poll_ms / 1000.0, args.history_s)
    app.start()


if __name__ == "__main__":
    main()
