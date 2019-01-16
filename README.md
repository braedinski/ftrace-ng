# ftrace-ng

The 'next-generation' of Ryan O'Neill's `ftrace`. `ftrace` is a user-land function tracer, which can be useful for gaining a quick insight into the control flow of a program, as well as the parameters passed to certain functions.

`ftrace-ng` is using `libelfmaster`, and will (hopefully) be easier to read and maintain. We plan to support multiple architectures and add on to the functionality of the original `ftrace`.

### Compiling
Firstly, install `libmasterelf`, then type `make` once you're in the project's root directory. `libelfmaster` currently installs itself into `/opt/elfmaster` and the `Makefile` reflects this. If your installation is different, you'll need to update `Makefile`.

### Features
* It currently only supports non-stripped binaries. We plan on supporting stripped binaries in the future.

