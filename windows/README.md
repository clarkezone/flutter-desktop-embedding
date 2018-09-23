# The Flutter Windows Desktop Embedder

This framework provides the basic functionality for embedding a Flutter app on
the Windows desktop.

This README assumes you have already read [the top level README](../README.md),
which contains important information and setup common to all platforms.

## Minimum Flutter Version

The minimum version of Flutter supported on Windows is
[32941a8](https://github.com/flutter/flutter/commit/32941a8cc0df5d7653a5c2c40ffb180c4db1c15d).

## Using the Library

Build the GLFW Library project in Visual Studio into a static or dynamic library,
then link `flutter_embedder.lib` and include `embedder.h` into your binary. Also
ensure that the `flutter_engine.dll`, and if using a dynamic library
`flutter_embedder.dll`, are in valid DLL include paths.

The output files are located in `bin\x64\$(Configuration)\GLFW Library\`.

## Example Application

The application under `example\` shows an example application using the
library.

You should be able to build the GLFW Example project in Visual Studio and have
access to `GLFW Example.exe` located in `bin\x64\$(Configuration)\GLFW Example\`.

The resulting binary expects to be run from this directory like so:

```
> ".\bin\x64\$(Configuration)\GLFW Example\GLFW Example.exe"
```

e.g.:

```
> ".\bin\x64\Debug Dynamic Library\GLFW Example\GLFW Example.exe"
```

or you can use Visual Studio's inbuilt debugger to build the library and run the
example application automatically.

## Caveats

* While GLFW is in use in the current iteration, this is not going to be the
  final state for Windows support.
* Text input does not work yet.
* Plugins are not currently supported.

## Angle as an alternative to GLFW: Addendum for Clarkezone fork

This fork of the Desktop Embedder project contains a proof-of-concept implementation of the Windows Desktop Embedder running on Angle rather than on GLFW and the Windows OpenGL backend.  It is currently living side-by-side with the GLFW version in a parallel Examples directory.  In order to build and run that version, you'll first need to build a custom version of the Flutter Engine first from here: 

https://github.com/clarkezone/engine.git

You'll need to rebase on the appropriate engine.stamp hash that matches the version of Flutter you have installed.  This fork of the Flutter Engine contains a change with the necessary hook to support Angle.  It also enables modern Windows Compositor integration using SpriteVisual.

At present, the Angle dependency is fetched in binary form as a DLL from a custom fork of Angle which has also been enlightened about with modern compositor hosting and SpriteVisual.  This change is in the process of being contributed to the Angle upstream.  It is available here:

https://github.com/clarkezone/angle-1/

Once the Desktop Embedder project moves to GN, and the Angle project accepts the SpriteVisual contribution, the dependency can be fulfilled in source from and statically linked into the embedder project.