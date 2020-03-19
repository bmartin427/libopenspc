libOpenSPC
==========

This repo hosts libOpenSPC, which is a project I initially released via source
tarball on my website back in 2002.  It has unfortunately received little
attention since then.  Please see the README file for the original
documentation.

This repo is currently undergoing an overhaul, with the goal of serving as a
better example of how I would structure such a project today.  Additionally,
the commit history can serve as an example of how I would go about
restructuring such a legacy codebase.

Installation
------------

This project now depends on [meson](https://mesonbuild.com) 0.46 or higher.
In Ubuntu 19.04 or newer, this can be installed with a simple `sudo apt
install meson`.  In Ubuntu 18.04, you have the option of either:

 * Manually downloading [the .deb from
   19.04](https://packages.ubuntu.com/disco/meson) and installing it, or

 * Using `pip3 install meson` to get a current version.  (You may need to
   separately `sudo apt install ninja-build` if you go this route.)

Once a suitable version of meson is installed, use the following commands to
configure, build, and install:

 * `meson release`
 * `cd release`
 * `ninja`
 * `ninja install`
