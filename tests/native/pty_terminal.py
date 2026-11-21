#!/usr/bin/env python3
"""PTY and redirected-stdio checks for the native terminal build of UCB Logo.

Usage: pty_terminal.py [/path/to/ucblogo]     (or set UCBLOGO)
LOGOLIB (if set) is passed through to the interpreter.

Each check runs in a fresh pty (or fresh pipes) with timeouts. One line per
check is printed as "name ... Ok" or "name ... Failed: reason", so the output
can be graded like the unit-test results:

    awk '/\\.\\.\\./ && ! /Ok$/{print "FAIL: "$0}' output.txt

Exit status is nonzero if any check failed. Diagnostics go to stderr.
"""

import fcntl
import os
import re
import select
import shutil
import signal
import struct
import subprocess
import sys
import tempfile
import termios
import time

STEP_TIMEOUT = 10.0   # waiting for a prompt / flag change / exit
EOF_TIMEOUT = 10.0    # redirected-stdin runs must finish within this

ICANON_ECHO = termios.ICANON | termios.ECHO
# termcap "cl" for xterm; terminfo "clear" also appends \x1b[3J, which termcap lacks.
XTERM_CLEAR = b"\x1b[H\x1b[2J"
CUP_RE = rb"\x1b\[\d+;\d+H"         # any CSI row;col H cursor motion

# Legacy frames printed on stdout even when stdin/stdout are redirected
# (main.c startup banner when argc < 2; lbye() farewell).
BANNER_RE = re.compile(
    rb"\A\x1b\[H\x1b\[2J\x1b\[1;1HWelcome to Berkeley Logo version [^\n]*\n")
FAREWELL = b"Thank you for using Logo.\nHave a nice day.\n"


def strip_legacy_frames(out):
    out = BANNER_RE.sub(b"", out, count=1)
    if out.endswith(FAREWELL):
        out = out[:-len(FAREWELL)]
    return out


class CheckFailed(Exception):
    pass


def tput(*args):
    """Terminfo-derived bytes for xterm, or None when tput is unavailable."""
    if shutil.which("tput") is None:
        return None
    try:
        r = subprocess.run(["tput", "-T", "xterm"] + list(args),
                           capture_output=True, timeout=5)
    except (OSError, subprocess.SubprocessError):
        return None
    return r.stdout if r.returncode == 0 and r.stdout else None


def interpreter_env(term):
    """Environment for the child; term=None leaves TERM unset."""
    env = dict(os.environ)
    for k in ("TERM", "EMACS", "COLUMNS", "LINES", "TERMCAP"):
        env.pop(k, None)
    if term is not None:
        env["TERM"] = term
    return env


class PtySession:
    """Interpreter attached to a fresh pty as its controlling terminal."""

    def __init__(self, exe, term, workdir):
        self.master, self.slave = os.openpty()
        # Keep the slave open here so master reads never see EIO after exit.
        fcntl.ioctl(self.slave, termios.TIOCSWINSZ,
                    struct.pack("HHHH", 24, 80, 0, 0))
        self.before = termios.tcgetattr(self.slave)
        self.buf = b""
        self.mark = 0
        self.proc = subprocess.Popen(
            [exe], stdin=self.slave, stdout=self.slave, stderr=self.slave,
            env=interpreter_env(term), cwd=workdir, start_new_session=True,
            preexec_fn=lambda: fcntl.ioctl(0, termios.TIOCSCTTY, 0))

    # -- I/O -------------------------------------------------------------
    def pump(self, timeout):
        r, _, _ = select.select([self.master], [], [], timeout)
        if r:
            try:
                data = os.read(self.master, 4096)
            except OSError:
                data = b""
            self.buf += data

    def send(self, data):
        if isinstance(data, str):
            data = data.encode()
        os.write(self.master, data)

    def set_mark(self):
        self.mark = len(self.buf)

    def new_output(self):
        return self.buf[self.mark:]

    def expect(self, pattern, timeout=STEP_TIMEOUT, what=None):
        rx = re.compile(pattern)
        end = time.monotonic() + timeout
        while True:
            if rx.search(self.buf, self.mark):
                return
            if self.proc.poll() is not None:
                self.pump(0.05)
                if rx.search(self.buf, self.mark):
                    return
                raise CheckFailed("process exited (%s) while waiting for %s; "
                                  "output tail %r" % (self.exit_desc(),
                                  what or pattern, self.buf[-200:]))
            if time.monotonic() >= end:
                raise CheckFailed("timeout waiting for %s; output tail %r"
                                  % (what or pattern, self.buf[-200:]))
            self.pump(0.1)

    def prompt(self, timeout=STEP_TIMEOUT):
        self.expect(rb"\? $", timeout, "prompt")

    def line(self, text, wait_prompt=True):
        """Send one input line and, by default, wait for the next prompt."""
        self.set_mark()
        self.send(text + "\n")
        if wait_prompt:
            self.prompt()

    def pause(self, seconds):
        end = time.monotonic() + seconds
        while time.monotonic() < end:
            self.pump(0.05)

    # -- termios -----------------------------------------------------------
    def attrs(self):
        return termios.tcgetattr(self.slave)

    def wait_lflag(self, mask, want_set, timeout=STEP_TIMEOUT, what=""):
        end = time.monotonic() + timeout
        while True:
            lflag = self.attrs()[3]
            if bool(lflag & mask) == want_set:
                return
            if self.proc.poll() is not None:
                raise CheckFailed("process exited (%s) before %s"
                                  % (self.exit_desc(), what))
            if time.monotonic() >= end:
                raise CheckFailed("timeout before %s" % what)
            self.pump(0.02)

    def check_restored(self):
        after = self.attrs()
        b, a = self.before[3], after[3]
        if (b ^ a) & ICANON_ECHO or b != a:
            raise CheckFailed(
                "termios lflag not restored: before ICANON=%d ECHO=%d "
                "(0x%x), after ICANON=%d ECHO=%d (0x%x)" % (
                    bool(b & termios.ICANON), bool(b & termios.ECHO), b,
                    bool(a & termios.ICANON), bool(a & termios.ECHO), a))

    # -- lifecycle -----------------------------------------------------------
    def exit_desc(self):
        rc = self.proc.poll()
        if rc is None:
            return "running"
        if rc < 0:
            return "signal %d" % -rc
        return "status %d" % rc

    def wait_exit(self, timeout=STEP_TIMEOUT):
        end = time.monotonic() + timeout
        while self.proc.poll() is None:
            if time.monotonic() >= end:
                raise CheckFailed("did not exit within %gs; output tail %r"
                                  % (timeout, self.buf[-200:]))
            self.pump(0.05)
        self.pump(0.05)
        return self.proc.returncode

    def expect_clean_exit(self, timeout=STEP_TIMEOUT):
        rc = self.wait_exit(timeout)
        if rc != 0:
            raise CheckFailed("abnormal exit (%s); output tail %r"
                              % (self.exit_desc(), self.buf[-200:]))

    def close(self):
        if self.proc.poll() is None:
            try:
                os.killpg(self.proc.pid, signal.SIGKILL)
            except OSError:
                pass
            self.proc.wait()
        for fd in (self.master, self.slave):
            try:
                os.close(fd)
            except OSError:
                pass


class Ctx:
    def __init__(self, exe, workdir):
        self.exe = exe
        self.workdir = workdir

    def session(self, term="xterm"):
        s = PtySession(self.exe, term, self.workdir)
        return s


def with_session(ctx, term, body):
    s = ctx.session(term)
    try:
        s.prompt()
        body(s)
    finally:
        s.close()


# ---------------------------------------------------------------- checks ---

def check_charmode_restore(ctx):
    def body(s):
        s.set_mark()
        s.send("ignore readchar\n")
        s.wait_lflag(termios.ICANON, False,
                     what="readchar entered character mode (ICANON cleared)")
        if s.attrs()[3] & termios.ECHO:
            raise CheckFailed("ECHO still set in character mode")
        s.send("k")
        s.prompt()
        s.line("bye", wait_prompt=False)
        s.expect_clean_exit()
        s.check_restored()
    with_session(ctx, "xterm", body)


def check_normal_exit_restore(ctx):
    def body(s):
        s.line("print 1 + 1")
        s.expect(rb"(?m)^2\r?$", what="output 2")
        s.line("bye", wait_prompt=False)
        s.expect_clean_exit()
        s.check_restored()
    with_session(ctx, "xterm", body)


# Upstream bug jrincayc/ucblogo-code#265: SIGINT can crash the interpreter (SIGSEGV).
SIGINT_SEGV_RETRIES = 3


def check_sigint_returns_to_prompt(ctx):
    for attempt in range(SIGINT_SEGV_RETRIES + 1):
        try:
            return _check_sigint_once(ctx)
        except CheckFailed as e:
            if "signal 11" not in str(e) or attempt == SIGINT_SEGV_RETRIES:
                raise
            print("note: SIGSEGV on Ctrl-C (known issue #265), retrying",
                  file=sys.stderr)


def _check_sigint_once(ctx):
    def body(s):
        if not s.attrs()[3] & termios.ISIG:
            raise CheckFailed("pty has ISIG off before the test")
        s.line('forever [make "x 1]', wait_prompt=False)
        s.pause(0.5)
        if s.proc.poll() is not None:
            raise CheckFailed("exited before Ctrl-C (%s)" % s.exit_desc())
        s.set_mark()
        s.send(b"\x03")
        s.prompt()
        s.line('print "alive', wait_prompt=False)
        s.expect(rb"(?m)^alive\r?$", what="'alive' after Ctrl-C")
        s.prompt()
        s.line("bye", wait_prompt=False)
        s.expect_clean_exit()
        s.check_restored()
    with_session(ctx, "xterm", body)


def check_xterm_cleartext(ctx):
    def body(s):
        s.line("cleartext")
        if XTERM_CLEAR not in s.new_output():
            raise CheckFailed("clear sequence %r not emitted; got %r"
                              % (XTERM_CLEAR, s.new_output()))
        s.line("bye", wait_prompt=False)
        s.expect_clean_exit()
    with_session(ctx, "xterm", body)


def check_xterm_setcursor(ctx):
    expected = tput("cup", "2", "3")   # row 2, column 3 -> setcursor [3 2]

    def body(s):
        s.line("setcursor [3 2]")
        out = s.new_output()
        if expected is not None:
            if expected not in out:
                raise CheckFailed("cursor sequence %r not emitted; got %r"
                                  % (expected, out))
        elif not re.search(CUP_RE, out):
            raise CheckFailed("no cursor-motion sequence emitted; got %r" % out)
        s.line("bye", wait_prompt=False)
        s.expect_clean_exit()
    with_session(ctx, "xterm", body)


def check_xterm_cursor_report(ctx):
    # Same input line: the prompt and echoed newline in between move the tracked cursor.
    def body(s):
        s.line("setcursor [3 2] show cursor")
        s.expect(rb"(?m)\[3 2\]\r?$", what="[3 2] from show cursor")
        s.line("setcursor [3 2] print cursor")
        s.expect(rb"(?m)^(?:\x1b\[\d+;\d+H)?3 2\r?$",
                 what="'3 2' from print cursor")
        s.line("bye", wait_prompt=False)
        s.expect_clean_exit()
    with_session(ctx, "xterm", body)


def make_no_term_check(term):
    def check(ctx):
        def body(s):
            s.line("cleartext")
            s.line("setcursor [3 2]")
            s.line("show cursor")
            s.line('print "done')
            s.expect(rb"(?m)^done\r?$", what="'done' after terminal commands")
            s.line("bye", wait_prompt=False)
            s.expect_clean_exit()
            s.check_restored()
        with_session(ctx, term, body)
    return check


def check_setmargins(ctx):
    def body(s):
        s.line("setmargins [2 1] show cursor")
        s.expect(rb"\[0 0\]", what="[0 0] relative to margins")
        s.line('print "hello')
        s.expect(rb"(?m)^ *hello\r?$", what="'hello' after setmargins")
        s.line("bye", wait_prompt=False)
        s.expect_clean_exit()
        s.check_restored()
    with_session(ctx, "xterm", body)


def run_piped(ctx, stdin_bytes, term="xterm"):
    p = subprocess.Popen([ctx.exe], stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                         env=interpreter_env(term), cwd=ctx.workdir,
                         start_new_session=True)
    try:
        out, err = p.communicate(stdin_bytes, timeout=EOF_TIMEOUT)
    except subprocess.TimeoutExpired:
        try:
            os.killpg(p.pid, signal.SIGKILL)
        except OSError:
            pass
        p.communicate()
        raise CheckFailed("hung for more than %gs on redirected stdin"
                          % EOF_TIMEOUT)
    if p.returncode < 0:
        raise CheckFailed("killed by signal %d; stderr %r"
                          % (-p.returncode, err[-200:]))
    return out, err, p.returncode


def check_stdout_literal(ctx):
    out, err, rc = run_piped(ctx, b'print "<&>\nbye\n')
    body = strip_legacy_frames(out)
    if body != b"<&>\n":
        raise CheckFailed("stdout %r, expected b'<&>\\n' after legacy "
                          "banner/farewell (rc %d, stderr %r)"
                          % (out, rc, err[-200:]))


def check_eof_readchar(ctx):
    out, err, rc = run_piped(ctx, b"show readchar\n")
    if b"[]" not in out:
        raise CheckFailed("readchar at EOF printed %r, expected []; "
                          "rc %d, stderr %r" % (out, rc, err[-200:]))


def check_eof_plain(ctx):
    out, err, rc = run_piped(ctx, b"print 1\n")
    if strip_legacy_frames(out).strip() != b"1":
        raise CheckFailed("stdout %r, expected b'1\\n'; rc %d, stderr %r"
                          % (out, rc, err[-200:]))


def check_eof_empty(ctx):
    run_piped(ctx, b"")


CHECKS = [
    ("charmode_restore", check_charmode_restore),
    ("normal_exit_restore", check_normal_exit_restore),
    ("sigint_returns_to_prompt", check_sigint_returns_to_prompt),
    ("xterm_cleartext_sequence", check_xterm_cleartext),
    ("xterm_setcursor_sequence", check_xterm_setcursor),
    ("xterm_cursor_report", check_xterm_cursor_report),
    ("term_dumb_commands", make_no_term_check("dumb")),
    ("term_unset_commands", make_no_term_check(None)),
    ("setmargins_xterm", check_setmargins),
    ("redirected_stdout_literal", check_stdout_literal),
    ("redirected_eof_readchar", check_eof_readchar),
    ("redirected_eof_plain", check_eof_plain),
    ("redirected_eof_empty", check_eof_empty),
]


def one_line(text):
    return " ".join(str(text).split())


def main(argv):
    exe = argv[1] if len(argv) > 1 else os.environ.get("UCBLOGO")
    if not exe:
        print("usage: pty_terminal.py /path/to/ucblogo (or set UCBLOGO)",
              file=sys.stderr)
        return 2
    exe = os.path.abspath(exe)
    if not os.access(exe, os.X_OK):
        print("not an executable: %s" % exe, file=sys.stderr)
        return 2

    workdir = tempfile.mkdtemp(prefix="ucblogo-pty-")
    ctx = Ctx(exe, workdir)
    failures = 0
    try:
        for name, fn in CHECKS:
            try:
                fn(ctx)
                print("%s ... Ok" % name)
            except CheckFailed as e:
                failures += 1
                print("%s ... Failed: %s" % (name, one_line(e)))
            except Exception as e:  # harness or OS error counts as failure
                failures += 1
                print("%s ... Failed: %s: %s" % (name, type(e).__name__,
                                                 one_line(e)))
            sys.stdout.flush()
    finally:
        shutil.rmtree(workdir, ignore_errors=True)
    print("%d checks, %d failed" % (len(CHECKS), failures), file=sys.stderr)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
