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

#Makefile for compiling MOLASSES Modular Lava Simulator

#MODULES: Alter as needed. 
export driver      = LJC2
export initialize  = LJC2
export DEM_loader  = LJC
export arrayinit   = LJC
export initflow    = 2LJC2
export pulse       = 2LJC2
export distribute  = proportional2slope4_LJC2
export neighbor_ID = 8
export output      = 2LJC2
export newvent     = LJC2
export check_vent	 = 2
export params      = 2
# export activate  = LJC

# Linking and compiling variables
# Alter as needed for your system.
export CC	= gcc
export INSTALLPATH	= .
export GDAL_INCLUDE_PATH = /usr/include/gdal
# export GDAL_LIB_PATH =
export BINDIR	= $(INSTALLPATH)/bin

all clean check install uninstall molasses:
	$(MAKE) -C src $@

