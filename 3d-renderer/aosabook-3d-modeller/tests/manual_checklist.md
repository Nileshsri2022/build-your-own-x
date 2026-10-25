# Manual checklist — window-dependent checks (NOT run here)

These need a real window, a real OpenGL context, and a human looking
at it. They are NOT claimed as passing: on this machine PyOpenGL,
GLUT and glumpy are absent (see T0 in the review below), so the
window cannot be opened here. Run on a machine with Python + numpy
+ PyOpenGL + GLUT + glumpy, then check each item off by hand.

- [ ] Window opens at 640 x 480, titled "3D Modeller", showing the
      sample scene (cube, sphere, snow figure) plus a unit grid, on
      a mid-grey background.
- [ ] Dragging orbits the scene smoothly via the trackball, with no
      jump or flip when passing over the top (gimbal-lock check).
- [ ] Mouse-wheel moves the scene along Z; drag translates it in
      the plane.
- [ ] Clicking an object selects it and it visibly glows; clicking
      empty space clears selection.
- [ ] With an object selected, up/down arrows scale it visibly
      (larger / smaller), left/right arrows cycle its colour.
- [ ] Pressing `s` adds a sphere, `c` adds a cube, and the new
      object appears at the expected depth and can then be selected
      and moved.
- [ ] Closing the window ends the program cleanly.
