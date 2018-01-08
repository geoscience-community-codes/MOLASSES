#### NAME
**MOLASSES**

MOLASSES stands for *MO*dular *LA*va *S*imulation *S*oftware for *E*arth *S*cience.
 
#### DESCRIPTION

MOLASSES is an in-progress fluid flow simulator, written in C. 

#### REQUIREMENTS

MOLASSES requires the GDAL C libraries and a C compiler. To get the GDAL libraries, install gdal-devel on your UNIX machine. We have tested this program on computers that use the C compiler gcc.

#### INSTALLATION

Before compilation of MOLASSES modify the Makefile in this directory for your use. 

1) Check to make sure that the path to GDAL and gcc are properly set. 

2) The module names at the top of the Makefile may be changed to accomadate an additional model algorithm. This name refers to a new C-code file in the src directory.

To compile and install MOLASSES execute the following commands:

		make
		make install
