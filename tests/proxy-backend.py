"""Exercise the remote helper with a real socat and a simulated serial port."""

import contextlib
import os
import pty
import select
import signal
import subprocess
import sys
import termios
import time
import unittest
from pathlib import Path

BACKEND = sys.argv.pop(1)
SOCAT = sys.argv.pop(1)
ENV = {**os.environ, "PATH": f"{Path(SOCAT).parent}:{os.environ.get('PATH', '')}"}


def read_bytes(fd, count):
    """Read an exact byte count within a bounded time."""
    result = b""
    deadline = time.monotonic() + 3
    while len(result) < count:
        remaining = deadline - time.monotonic()
        if remaining <= 0 or not select.select([fd], [], [], remaining)[0]:
            message = f"Timed out after receiving {result!r}"
            raise AssertionError(message)
        chunk = os.read(fd, count - len(result))
        if not chunk:
            message = f"Unexpected EOF after {result!r}"
            raise AssertionError(message)
        result += chunk
    return result


@contextlib.contextmanager
def backend(*args):
    """Start the actual helper on a deliberately non-raw terminal."""
    master, slave = pty.openpty()
    settings = termios.tcgetattr(slave)
    settings[0] |= termios.ICRNL | termios.IXON | termios.IXOFF
    settings[1] |= termios.OPOST
    settings[3] |= termios.ECHO | termios.ICANON | termios.ISIG
    termios.tcsetattr(slave, termios.TCSANOW, settings)
    process = subprocess.Popen(  # noqa: S603
        ["/bin/sh", BACKEND, os.ttyname(slave), *args],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=ENV,
    )
    try:
        deadline = time.monotonic() + 3
        while termios.tcgetattr(slave)[3] & (termios.ECHO | termios.ICANON):
            if process.poll() is not None:
                raise AssertionError(process.stderr.read().decode())
            if time.monotonic() >= deadline:
                raise AssertionError("Helper did not configure the serial port")
            time.sleep(0.01)
        yield process, master, slave
    finally:
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=3)
        for stream in (process.stdin, process.stdout, process.stderr):
            stream.close()
        os.close(master)
        os.close(slave)


class BackendTest(unittest.TestCase):
    """Verify serial settings, transparent forwarding, and termination."""

    def test_bytes_and_prompt(self):
        with backend() as (process, master, slave):
            settings = termios.tcgetattr(slave)
            self.assertEqual(settings[4:6], [termios.B115200, termios.B115200])
            self.assertFalse(
                settings[0] & (termios.ICRNL | termios.IXON | termios.IXOFF)
            )
            self.assertFalse(settings[1] & termios.OPOST)
            self.assertFalse(
                settings[3] & (termios.ECHO | termios.ICANON | termios.ISIG)
            )
            data = b"AT\r\n\x00\x1b\x1a\x11\x13\xff"
            process.stdin.write(data)
            process.stdin.flush()
            self.assertEqual(read_bytes(master, len(data)), data)
            os.write(master, data)
            self.assertEqual(read_bytes(process.stdout.fileno(), len(data)), data)
            self.assertFalse(select.select([master], [], [], 0.1)[0], "Serial echo")
            # SMS sending must see this prompt without waiting for a newline.
            os.write(master, b"\r\n> ")
            self.assertEqual(read_bytes(process.stdout.fileno(), 4), b"\r\n> ")

    def test_baud_and_eof(self):
        with backend("19200") as (process, _master, slave):
            self.assertEqual(termios.tcgetattr(slave)[4:6], [termios.B19200] * 2)
            process.stdin.close()
            self.assertEqual(process.wait(timeout=3), 0)
            self.assertEqual(process.stderr.read(), b"")

    def test_signal_shutdown(self):
        for sig in (signal.SIGHUP, signal.SIGTERM):
            with self.subTest(signal=sig), backend() as (process, _master, _slave):
                process.send_signal(sig)
                process.wait(timeout=3)

    def test_missing_socat(self):
        result = subprocess.run(  # noqa: S603
            ["/bin/sh", BACKEND, "/dev/null"],
            capture_output=True,
            env={**ENV, "PATH": "/nonexistent/gammu-test-path"},
            timeout=3,
            check=False,
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, b"")
        self.assertIn(b"requires socat", result.stderr)

    def test_invalid_arguments(self):
        for args in (
            [],
            ["/nonexistent/gammu-device"],
            ["/dev/null", "invalid"],
            ["/dev/null", ""],
            ["/dev/null", "12345"],
            ["/dev/null", "0"],
            ["/dev/null", "9600", "extra"],
            ["/dev/null,ignoreeof"],
            ["/dev/null:option"],
            ["/dev/null!!STDIO"],
        ):
            with self.subTest(args=args):
                result = subprocess.run(  # noqa: S603
                    ["/bin/sh", BACKEND, *args],
                    capture_output=True,
                    env=ENV,
                    timeout=3,
                    check=False,
                )
                self.assertNotEqual(result.returncode, 0)
                self.assertEqual(result.stdout, b"")
                self.assertTrue(result.stderr)


if __name__ == "__main__":
    unittest.main()
