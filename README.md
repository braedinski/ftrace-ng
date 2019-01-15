# ftrace-ng

The 'next-generation' of Ryan O'Neill's `ftrace`. `ftrace-ng` will use `libelfmaster`, and will (hopefully) be easier to read and maintain. We plan to support multiple architectures and add on to the functionality of the original `ftrace`.

### Compiling
Firstly, install `libmasterelf`, then type `make` once you're in the project's root directory. `libelfmaster` currently installs itself into `/opt/elfmaster` and the `Makefile` reflects this. If your installation is different, you'll need to update `Makefile`.
