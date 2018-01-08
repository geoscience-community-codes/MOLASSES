#Makefile for compiling MOLASSES Modular Lava Simulator (monte carlo version)

#MODULES: Alter as needed. 
#Check docs/ for module codes and destriptions.
export driver            = LJC
export initialize        = LJC
export DEM_loader        = LJC
export array_initializer = LJC
export flow_initializer  = LJC
export pulse             = LJC
export distribute        = proportional2slope_LJC
export neighbor_ID       = 4_LJC
export output            = LJC
export newvent       = LJC
# export activate          = LJC

# Linking and compiling variables
# Alter as needed for your system.
export CC	= gcc
export INSTALLPATH	= .
export GDAL_INCLUDE_PATH = /usr/include/gdal
# export GDAL_LIB_PATH
export BINDIR	= $(INSTALLPATH)/bin

all clean check install uninstall molasses:
	$(MAKE) -C src $@

