# Virtual Printer

This simulates a virtual, simplified 9-pin dot matrix ("impact") printer. Currently, this
only supports plain text and only CR/LF. I'll add other control characters (like tab) and
ESC/P support later.

## What's the resolution of a dot matrix printer?

9-pin dot matrix printers were pretty common in the early personal computing era. For the
idealized (i.e. simplified) case, assume that the "top 8" pins are only for printing
characters, and the 9th pin is dedicated for underline. I'll count from "zero" so it looks 
like this:

```
pin 7
pin 6
pin 5
pin 4
pin 3
pin 2
pin 1
pin 0
pin 9
```

Let's do some quick math. If we assume 9 pins to print a "line" and leave an extra "pin
height" gap between lines, then each line is effectively 10 "pins" high.

For 66 lines on an 11-inch piece of paper (US Letter) that works out to 60 DPI.

If we assume 60 DPI horizontally, then for an 8 1/2 inch paper (US Letter) at 10 CPI,
that works out to 5 pins for each character, plus an extra "pin width" gap between each
character. So 6 pins total:

```
  012345
7 *****.
6 *****.
5 *****.
4 *****.
3 *****.
2 *****.
1 *****.
0 *****.
9 -----.
x ......
```

Doing the math, we can figure out how many "pins" or "dots" or "pixels" define a single
sheet of US Letter paper:

* 8 1/2 inches x 60 DPI = 510 px wide
* 11 inches x 60 DPI = 660 px tall

That means:

* 640x480 is wide enough, but too "short"
* 800x600 is also wide enough, but still too "short"
* 1024x768 is both wide enough and long enough
