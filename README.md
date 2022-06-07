RestrictPDF
===========

<img src="icon.svg" width="64" height="64">

A simple gui tool to set or modifiy user password, owner password and restrictions like printing, editing, extracting, accessibility, assemble, form filling. It uses [libqpdf](https://github.com/qpdf/qpdf) for the pdf processing and win32 API for the gui (basically just a file chooser).

Please be aware that setting the owner password and all the restrictions are only applied by compliant pdf readers. They are more or less just snake oil. RestrictPDF won't obey them and also allows to remove them.

__RestrictPDF is currently in alpha state. Some features are hard coded.__

<img src="screenshot.png" width="400">

Building
---------
Use CMake with MinGW-w64 GCC compiler. MSYS2 can provide all the necessary tools. Make sure that cmake can find qpdf headers and library. With no options cmake will look for `qpdf/bin/qpdf28.dll`, `qpdf/lib`, `qpdf/include` inside of the RestrictPDF source folder.

