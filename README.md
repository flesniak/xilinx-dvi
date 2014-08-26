xilinx-dvi
==========

This is an OVP (Open virtual platforms, http://www.ovpworld.org/) peripheral simulating the "Xilinx XPS TFT" controller.
The XPS TFT controller is synthesized into the FPGA and implements a 2MB framebuffer for 640x480 resulution.
Additionally, it supplies some configuration registers for memory-remapping and basic display configuration functions.
For further information, see http://www.xilinx.com/products/intellectual-property/xps_tft.htm

COMPILATION
===========

- be sure that your environment is properly set up to use Imperas OVP
- Edit "dvi-mem.h" to fit your requirements, e.g. may want to disable DLO support.
- "make"

RUN
===

You may run this peripheral in your own platform, though you may want to test the platform supplied by the project first.

- "cd platform"
- "./platform -d sdl ../application/rainbow.MICROBLAZE.elf"
- this should display a nice rainbow
- if you want to use a dlo device, just run the platform with "-d dlo"

NOTES
=====

- Despite this might just be useful on a MICROBLAZE architecture, it it independant and should be compatible with any CPU OVP supports.
- The peripheral implements some formals to configure its behaviour. See "pse/pse.tcl" to see which one are available.
- Test programs in "contrib" are not build automatically. Just run "make" inside the "contrib" directory.
