Overview
========

This project consists of a library with an implementation of the GJK algorithm
(for determining if two convex 3D objects are intersecting), and a graphical
application for demonstrating this library.

The demo is a graphical program which allows multiple 3D objects to be
positioned and oriented in 3D space, and indicates when they are intersecting.
When the demo application is started, there are no objects in the scene so the
window appears black. Objects can be created and positioned with the controls
described in the next section.

Every object has an associated mesh, which defines its convex hull and how it
looks. Meshes need to be loaded from a file before an object can be created.
Only OFF files are supported. Meshes can be loaded with a command line option,
or via the text interface which is described in the last section. The demo
script starts the demo application and loads a directory of sample meshes.

One object in the scene is always selected (unless there are no objects in the
scene). The selected object can be positioned in the scene with the keyboard
controls described in the next section. The mesh of the selected object can
be set to any of the loaded meshes.

Objects in the scene are coloured differently based on whether they are
selected, and whether they are intersecting other objects. An object that is not
selected is grey when it is not intersecting any other object, and red when it
is. The selected object is blue when it is not intersecting any other object,
and magenta when it is.

Controls
========

Object Selection:
- The up arrow creates a new object and selects it.
- The down arrow deletes the selected object.
- The left and right Arrows change the selected object.
- While holding left-shift, the left and right arrows change the mesh of the
  selected object, from the list of loaded meshes.

Translation:
- W moves the selected object forward (into the screen).
- S moves the selected object backward (out of the screen).
- A moves the selected object left.
- D moves the selected object right.
- Q moves the selected object down.
- E moves the selected object up.

Rotation:
- I and K rotate the selected object about the horizontal axis.
- J and L rotate the selected object about the vertical axis.
- U and O rotate the selected object about the axis pointing into/out-of the
  screen.

Camera:
- Holding left-shift allows the rotation controls to affect the entire scene,
  effectively allowing the camera to orbit the scene.

Text Interface
==============

When the demo application is running, the terminal from which it was launched
displays a prompt which can accept commands.

The "load" command loads a .off mesh from a file, or loads all .off meshes in a
folder. Example usage:

    > load demo_meshes/cube.off
    Loaded mesh "demo_meshes/cube.off".

    > load demo_meshes
    Loading meshes in directory "demo_meshes":
    Loaded mesh "demo_meshes/monkey.off".
    Loaded mesh "demo_meshes/cone.off".
    Loaded mesh "demo_meshes/monkey_cvx.off".
    Loaded mesh "demo_meshes/cube.off".

The "list mesh" command lists the following information for each loaded mesh:
its ID, the number of vertices, and the file from which it was loaded. Example
usage:

    > list mesh
    Mesh ID   Number of Vertices   Filename
    0         507                  demo_meshes/monkey.off
    1         33                   demo_meshes/cone.off
    2         66                   demo_meshes/monkey_cvx.off
    3         8                    demo_meshes/cube.off

The "mesh" command sets the mesh of the currently selected object to the mesh
with a specific ID. Example usage:

    > mesh 2
    Mesh 2 selected.

The "exit" or "quit" command closes the demo application.
