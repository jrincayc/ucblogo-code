# Turtle Drawing Patterns

## Establish state before drawing

Use a setup procedure when a drawing depends on mutable state. For portable examples, establish the fields the drawing actually uses:

```logo
setpencolor 4
setpensize 2
penpaint
showturtle
setbackground 7
setshape "turtle
wrap
setpos [0 0]
setheading 0
```

`penpaint` sets paint mode and down state together. `pendown` would set only down state and leave an existing erase/reverse mode unchanged. `setpensize` should be explicit because initial pen size is not consistently portable.

The setup above is intentionally finite and relies only on the current shared primitives. A background or palette of `7` is less portable than a simple setup claiming identical RGB appearance; verify exact rendering when colors matter.

## Procedure-local state

Save position and heading as local values when a helper must preserve them:

```logo
make "oldpos pos
make "oldheading heading
make "oldcolor pencolor
make "oldsize pensize
make "oldpendown pendownp
make "oldmode penmode
```

Restore with `setpos`, `setheading`, `setpencolor`, `setpensize`, and the saved mode/up-down state. `pen`/`setpen` can capture the pen bundle, but it does not include all fields above.

## Complete square example

This complete standalone example establishes every state on which its result depends:

```logo
setpencolor 4
setpensize 2
setbackground 7
penpaint
showturtle
setshape "turtle
wrap
setpos [0 0]
setheading 0
repeat 4 [forward 100 right 90]
showturtle
```

## Movement versus coordinate placement

Use `forward`, `back`, `left`, and `right` for relative drawing. Use `setpos`, `setxy`, `setx`, or `sety` for absolute placement, followed by an explicit heading if later relative turning matters.

Before coordinate placement, use `penup` so that travel leaves no line. When drawing resumes, use `pendown` if paint mode is already known or use `penpaint` to establish both paint mode and down state:

```logo
penpaint
setpos [0 0]
setheading 0
penup
setpos [50 50]
pendown
forward 40
```

This fragment is runnable only as part of a state-initialized session. It still establishes pen mode and position in sequence.

## Cleanup choices

Use `clean` when erasing should leave position and heading unchanged. Use `clearscreen` when a procedure also intends to return to `[0 0]` heading `0`.

Both commands preserve pen mode, up/down, color, size, background, visibility, shape, wrap/scrunch, and palette state. Clearing may trigger graphics/split-screen preparation, so do not describe it as a complete layout reset.

## Finite Web-safe outline

For a finite Web Edition scene, establish state, stop an existing animation, clear, draw, and stop the animation when done:

```logo
animationstop
setpencolor 2
setpensize 2
setbackground 7
penpaint
showturtle
setshape "turtle
wrap
setpos [0 0]
setheading 0
clearscreen
repeat 3 [repeat 6 [forward 60 right 60] right 120]
animationstop
```

`animationstop` is a Web-only browser primitive. The example has no frame loop and is suitable for finite browser extraction. It also establishes state before `clearscreen`, avoiding dependence on the clear as reset.

## Sprite and animation caution

Web startup procedures provide a sprite facility and one turtle/sprite. Animation facilities are browser-only. Runtime appearance and timing are unverified unless tested in a browser. Do not promise extra independent turtle sprites or a runnable sprite demo here.

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->
