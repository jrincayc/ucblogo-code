#!/usr/bin/env python3
"""Xvfb screenshot smoke test for the native wx build of UCB Logo.

Usage: wx_smoke.py [/path/to/ucblogo [screenshot-dir]]
       (executable defaults to $UCBLOGO; directory defaults to ./wx-smoke-screens)
LOGOLIB (if set) is passed through to the interpreter.

Each scene is a Logo script passed on the command line; the driver terminates
the GUI after the script runs `bye` if it remains open.
The script announces each drawn state by writing a marker file, then waits for an
acknowledgement file; the driver captures the Xvfb root window with xwd after the
marker appears and converts it to PNG with Pillow. One line per scene is printed:

    scene NAME ... Ok          or          scene NAME ... Failed: reason

so results can be graded like the unit tests:

    awk '/\\.\\.\\./ && ! /Ok$/{print "FAIL: "$0}' output.txt

Exit status is nonzero if any scene failed. Screenshots still need a human look:
the driver only rejects missing windows, crashes, and single-color images.
"""

import os
import signal
import struct
import subprocess
import sys
import tempfile
import time

from PIL import Image

SCREEN = "1024x768x24"
FIRST_DISPLAY = 90
START_TIMEOUT = 30.0     # launch until the first marker appears
STAGE_TIMEOUT = 30.0     # between markers
EXIT_TIMEOUT = 10.0      # wait for the script's bye before terminating the GUI
SETTLE = 1.0             # let wx repaint after the marker

PRELUDE = """\
to snap :marker
openwrite :marker
setwrite :marker
print "ready
setwrite []
close :marker
localmake "ackfile word :marker ".ack
repeat 1200 [
openread :ackfile
setread :ackfile
localmake "acknowledged equalp readword "ready
setread []
close :ackfile
if :acknowledged [stop]
wait 3
]
end
"""

# name -> [(stage label, Logo lines, ...)]; {tmp} is replaced with the work dir.
SCENES = {
    "text": [
        ("before-clear", [
            "print \"|<&> and 'it''s a \"quote\" test|",
            "print [a b c]",
            "show [x y]",
            "print [multi line: first]",
            "print [multi line: second]",
        ]),
        ("after-clear", [
            "cleartext",
            "setcursor [10 5]",
            "print [after setcursor 10 5]",
        ]),
    ],
    "graphics": [
        ("shapes", [
            "fd 100 rt 90 fd 100",
            "setpencolor 4",
            "label [hello]",
            "repeat 36 [fd 10 rt 10]",
        ]),
        ("background", [
            "setbackground 7",
        ]),
    ],
    "fileio": [
        ("roundtrip", [
            "openwrite \"{tmp}/data.txt",
            "setwrite \"{tmp}/data.txt",
            "print \"|<&>|",
            "setwrite []",
            "close \"{tmp}/data.txt",
            "openread \"{tmp}/data.txt",
            "setread \"{tmp}/data.txt",
            "make \"w readword",
            "setread []",
            "close \"{tmp}/data.txt",
            "(print [read back:] :w)",
        ]),
    ],
    "htmlstring": [
        ("markup", [
            "print htmlstring \"|<b>bold</b>|",
            "show htmlstring \"|<i>italic &amp; entity</i>|",
            "print \"|<b>ordinary word</b>|",
        ]),
    ],
}


def xwd_to_image(data):
    """Decode a 24/32 bpp ZPixmap xwd dump (what Xvfb 24-bit screens produce)."""
    (hsize, _ver, _fmt, _depth, w, h, _xoff, byte_order, _bu, _bbo, _bp, bpp,
     bpl, _vclass, rmask, gmask, bmask, _bprgb, _cmapn, ncolors,
     *_rest) = struct.unpack(">25I", data[:100])
    if bpp not in (24, 32) or (rmask, gmask, bmask) != (0xFF0000, 0xFF00, 0xFF):
        raise ValueError("unsupported xwd layout bpp=%d masks=%x/%x/%x"
                         % (bpp, rmask, gmask, bmask))
    start = hsize + ncolors * 12
    mode = {(24, 0): "BGR", (24, 1): "RGB", (32, 0): "BGRX",
            (32, 1): "XRGB"}[(bpp, byte_order)]
    return Image.frombytes("RGB", (w, h), data[start:start + h * bpl],
                           "raw", mode, bpl, 1)


def capture(display):
    r = subprocess.run(["xwd", "-root", "-silent", "-display", display],
                       capture_output=True, timeout=30)
    if r.returncode != 0:
        raise RuntimeError("xwd failed: " + r.stderr.decode(errors="replace"))
    return xwd_to_image(r.stdout)


def start_xvfb():
    for n in range(FIRST_DISPLAY, FIRST_DISPLAY + 100):
        if os.path.exists("/tmp/.X%d-lock" % n) or \
           os.path.exists("/tmp/.X11-unix/X%d" % n):
            continue
        proc = subprocess.Popen(
            ["Xvfb", ":%d" % n, "-screen", "0", SCREEN, "-nolisten", "tcp"],
            stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL, start_new_session=True)
        deadline = time.time() + 10
        while time.time() < deadline and proc.poll() is None:
            if os.path.exists("/tmp/.X11-unix/X%d" % n):
                return proc, ":%d" % n
            time.sleep(0.1)
        stop(proc)   # lost a race for this display number; try the next
    raise RuntimeError("could not start Xvfb")


def stop(proc):
    if proc.poll() is None:
        for sig in (signal.SIGTERM, signal.SIGKILL):
            try:
                os.killpg(proc.pid, sig)
            except ProcessLookupError:
                break
            try:
                proc.wait(timeout=5)
                break
            except subprocess.TimeoutExpired:
                pass
    try:
        proc.wait(timeout=1)
    except subprocess.TimeoutExpired:
        pass


def marker_ready(path):
    try:
        with open(path) as f:
            return f.read().strip() == "ready"
    except OSError:
        return False


def window_listing(display):
    r = subprocess.run(["xwininfo", "-root", "-tree", "-display", display],
                       capture_output=True, text=True, timeout=10)
    return r.stdout


def describe_exit(rc):
    if rc < 0:
        return "crashed (signal %d)" % -rc
    return "exited with status %d" % rc


def run_scene(exe, display, name, stages, outdir, tmp, env):
    """Returns (error or None, [png paths])."""
    lines = [PRELUDE]
    markers = []
    for label, body in stages:
        marker = "%s/%s-%s" % (tmp, name, label)
        markers.append(marker)
        with open(marker + ".ack", "w") as acknowledgement:
            acknowledgement.write("pending\n")
        lines += [l.format(tmp=tmp) for l in body]
        lines.append("snap \"%s" % marker)
    lines += ["wait 60", "bye"]
    script = os.path.join(tmp, name + ".lg")
    with open(script, "w") as f:
        f.write("\n".join(lines) + "\n")

    log = open(os.path.join(tmp, name + ".log"), "w")
    proc = subprocess.Popen([exe, script], env=dict(env, DISPLAY=display),
                            stdin=subprocess.DEVNULL, stdout=log, stderr=log,
                            start_new_session=True)
    pngs = []
    try:
        for i, ((label, _), marker) in enumerate(zip(stages, markers)):
            deadline = time.time() + (START_TIMEOUT if i == 0 else STAGE_TIMEOUT)
            while not marker_ready(marker):
                if proc.poll() is not None:
                    return "app %s before stage %s" % (
                        describe_exit(proc.returncode), label), pngs
                if time.time() > deadline:
                    shot = os.path.join(outdir, "%s-%s-timeout.png"
                                        % (name, label))
                    capture(display).save(shot)
                    pngs.append(shot)
                    return ("stage %s not reached in time (see %s and %s)"
                            % (label, shot, log.name)), pngs
                time.sleep(0.1)
            time.sleep(SETTLE)
            img = capture(display)
            path = os.path.join(outdir, "%s-%s.png" % (name, label))
            img.save(path)
            pngs.append(path)
            colors = img.getcolors(1 << 24)
            print("  %s: %d colors" % (os.path.basename(path), len(colors)),
                  file=sys.stderr)
            if all(lo == hi for lo, hi in img.getextrema()):
                return "screenshot %s is a single color" % label, pngs
            with open(marker + ".ack.tmp", "w") as acknowledgement:
                acknowledgement.write("ready\n")
            os.replace(marker + ".ack.tmp", marker + ".ack")
        if proc.poll() is not None:
            return "app exited before bye", pngs
        try:
            rc = proc.wait(timeout=EXIT_TIMEOUT)
        except subprocess.TimeoutExpired:
            return None, pngs
        if rc != 0:
            return "app %s after bye" % describe_exit(rc), pngs
        return None, pngs
    finally:
        stop(proc)
        log.close()


def main():
    exe = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("UCBLOGO")
    if not exe or not os.access(exe, os.X_OK):
        print("usage: wx_smoke.py UCBLOGO [OUTDIR]  (or set $UCBLOGO)",
              file=sys.stderr)
        return 2
    exe = os.path.abspath(exe)
    outdir = os.path.abspath(sys.argv[2] if len(sys.argv) > 2
                             else "wx-smoke-screens")
    os.makedirs(outdir, exist_ok=True)

    tmp = tempfile.mkdtemp(prefix="wxsmoke")
    env = dict(os.environ, HOME=os.path.join(tmp, "home"))
    os.makedirs(env["HOME"])
    xvfb, display = start_xvfb()
    print("Xvfb %s pid %d, work dir %s" % (display, xvfb.pid, tmp),
          file=sys.stderr)
    failed = False
    allpngs = []
    try:
        for name, stages in SCENES.items():
            try:
                err, pngs = run_scene(exe, display, name, stages, outdir, tmp,
                                      env)
            except Exception as e:   # noqa: BLE001 - report any driver error
                err, pngs = "driver error: %r" % e, []
            allpngs += pngs
            if err:
                failed = True
                print("scene %s ... Failed: %s" % (name, err))
            else:
                print("scene %s ... Ok" % name)
            sys.stdout.flush()
    finally:
        stop(xvfb)
    for p in allpngs:
        print("png: " + p)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
