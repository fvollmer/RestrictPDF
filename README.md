RestrictPDF
===========

<img src="icon.svg" width="64" height="64">

A simple gui tool to set or modifiy user password, owner password and restrictions like printing, editing, extracting, accessibility, assemble, form filling. It uses [libqpdf](https://github.com/qpdf/qpdf) for the pdf processing and win32 API for the gui (basically just a file chooser).

Please be aware that all setings except the user password are only usefull if compliant pdf reader is used. The owner password and all other restrictions are more or less just snake oil. RestrictPDF won't obey them and also allows to remove them.

<img src="https://user-images.githubusercontent.com/16699443/182047900-1608c4b9-62fb-48dc-8e33-09c07be9f965.png" width="400">

Building
---------
Use CMake with MinGW-w64 GCC compiler. MSYS2 can provide all the necessary tools. Make sure that cmake can find qpdf headers and library. With no options cmake will look for `qpdf/bin/qpdf28.dll`, `qpdf/lib`, `qpdf/include` inside of the RestrictPDF source folder.

