#### NAME
**MOLASSES**

MOLASSES stands for *MO*dular *LA*va *S*imulation *S*oftware for *E*arth *S*cience.
 
#### DESCRIPTION

MOLASSES is an in-progress fluid flow simulator, written in C. 
The MOLASSES model relies on a cellular automata algorithm to
estimate the area inundated by lava flows.

#### CODE REQUIREMENTS

1) MOLASSES requires the GDAL C libraries and a C compiler. GDAL libraries and development files are available for many systems (http://trac.osgeo.org/gdal/wiki/DownloadingGdalBinaries); install both the gdal library and the header (development) files. We have tested this program on computers that use the C compiler gcc.

2) MOLASSES requires a DEM (Digital Elevation Model) in a format recognized by the GDAL code library. The DEM should extend beyond the boundaries of the lava flow(s). 

3) MOLASSES requires a configuration file that is specified on the command line when executing the code. 

#### LICENSE

    Copyright (C) 2015-2020  
    Laura Connor (lconnor@usf.edu)
    Jacob Richardson 
    Charles Connor

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 

#### COMPILING and INSTALING THE CODE

Before compilation of MOLASSES modify the makefile in the top-level directory for your use. 

1) Check to make sure that the path to GDAL and gcc are properly set. 

2) The module names at the top of the Makefile may be changed to accomadate alternative model algorithms. This name refers to a new C-code file (for the alternative algorithm) in the src directory.

To compile and install MOLASSES execute the following commands:

		make
		make install

'make' compiles the code; 'make install' copies the code to a 'bin' (program) directory specified in the top-level makefile.

#### PROGRAM EXECUTION

To run molasses type:

	$PATH_TO_MOLASSES/$molasses $config_file

where $PATH_TO_MOLASSES indicates where the executable code is located, $molasses indicates the exact name of the compiled code, and $config_file indicates the name of the configuration file. It is most convenient if the configuration file resides in your working directory. 

MOLASSES can be executed from the 'run_code_here' directory. Within this directory are perl scripts that will execute the code, plot the output using GMT (Generic Mapping Tools), and write out 2 text files: (1) 'logfile' and (2) 'output', to help indicate errors or problems if any should occur. The perl scripts begin with the word 'run' and have a '.pl' extension. These perl 'run' scripts work with a specific configuration file, and a specific plotting file that contains the list of GMT commands needed to plot the resulting lava flow on the DEM specified in the configuration file. For this case, it is convenient to convert the DEM to a '.grd' (netCDF) file so the GMT commands can use the same DEM file when plotting the lava flow. There is a directory called DEMs where it is convenient to store the various DEM files used. Some example files have been provided in this directory. To execute this perl 'run' script type:
	
	perl run_xxx.pl

where _xxx indicates any name modification to the script. Run this example in the 'run_code_here' directory:
	
	perl run_SE-dip.pl

	
