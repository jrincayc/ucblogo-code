# UCBLogo

## Berkeley Logo interpreter

This is a free (both senses) interpreter for the Logo programming language.

This program is free software: you can redistribute it and/or modify 
it under the terms of the GNU General Public License as published by 
the Free Software Foundation, either version 3 of the License, or 
(at your option) any later version. 
 
This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
GNU General Public License for more details. 
 
You should have received a copy of the GNU General Public License 
along with this program.  If not, see https://www.gnu.org/licenses/. 

## About this project

This repository created for further UCBLogo development.
The current version is 6.1

Changes for this release:
* The license has been changed to GPL3.
* The wxWidgets version updated to compile and run with version 3.0.
* Adjustments made to compile and run on newer OSX versions.
* Variety fixes like fixing implicit function definitions, fixing a line cursor behaviour, etc.

Here is an overview of the repository:
* csls - Programs form [Brian Havrvey's trilogy "Computer Science Logo Style"](https://people.eecs.berkeley.edu/~bh/).
* docs - Usermanual in variety formats and brief manual page.
* helpfiles - files for UCBLogo interactive help.
* test - Unit tests.
* / - program source code and [a Program Logic Manual](/plm).

The executable file in this distribution includes libraries from the
[wxWidgets project](http://wxwidgets.org/).  The Microsoft Windows version
is distributed with runtime library mingwm10.dll, from the [MinGW project](http://www.mingw.org/).  
Those libraries are also covered by the Gnu GPL.  We
do not distribute their source code; you can download it from their
respective web sites.

## Usage

To build Logo under *nix, do this:
```	
	./configure
	make
```

## Previous versions

For getting UCBLogo previous versions such as version 6.0 if you're running wxWidgets or 5.4 if not, please visit [Brian Harvey's UCBLogo GitHub repository](https://github.com/brianharvey/UCBLogo).

The distribution for frozen platforms includes version 5.3 for DOS and Mac Classic (pre-OS X) still may be reached at [Brian Harvey's personal page](https://people.eecs.berkeley.edu/~bh/).

