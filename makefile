############################################################################
# MOLASSES (MOdular LAva Simulation Software for the Earth Sciences) 
# The MOLASSES model relies on a cellular automata algorithm to 
# estimate the area inundated by lava flows.MOLASSES 
#
#    Copyright (C) 2015-2020  
#    Laura Connor (lconnor@usf.edu)
#    Jacob Richardson 
#    Charles Connor
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
###########################################################################    

#Makefile for compiling MOLASSES Modular Lava Simulator (monte carlo version)

#MODULES: Alter as needed. 
#Check docs/ for module codes and destriptions.
export driver            = LJC
export initialize        = LJC
export DEM_loader        = LJC
export array_initializer = LJC
export flow_initializer  = LJC
export pulse             = LJC
export distribute        = proportional2slope2_LJC
export neighbor_ID       = 4.0_LJC
export output            = LJC
export newvent       = LJC
export check_vent	=
# export activate          = LJC

# Linking and compiling variables
# Alter as needed for your system.
export CC	= gcc
export INSTALLPATH	= .
#export GDAL_INCLUDE_PATH = /usr/include/gdal
export GDAL_INCLUDE_PATH = /opt/gdal-1.11.2/include
# export GDAL_LIB_PATH
export GDAL_LIB_PATH = /opt/gdal-1.11.2/lib
export BINDIR	= $(INSTALLPATH)/bin

all clean check install uninstall molasses:
	$(MAKE) -C src $@

