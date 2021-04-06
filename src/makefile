/*############################################################################
# MOLASSES (MOdular LAva Simulation Software for the Earth Sciences) 
# The MOLASSES model relies on a cellular automata algorithm to 
# estimate the area inundated by lava flows.
#
#    Copyright (C) 2015-2021  
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
###########################################################################*/ 

# CFLAGS = -Wall -pedantic -g Wno-long-long
CFLAGS = -Wall -O2 -pthread
INCLUDES = -I$(GDAL_INCLUDE_PATH) -I../include -I./include
# If you compile the gc libraries rather than installing a precompiled version
# into the system, you may need to modify the LIBS variable to indicate
# where you put the -lgc library and include files. Notice the include and lib
# directories in the top level folder.
# If you set a specific path for your GDAL libraries, this will look for it!
ifndef GDAL_LIB_PATH
	LIBS = -lgdal -lgc -L../lib -lpthread -lran -lm
else
	LIBS = -L$(GDAL_LIB_PATH) -lpthread -lgc -lgdal -L../lib -lran -lm
endif

SRCS = driver_$(driver).c \
initialize_$(initialize).c \
demloader_$(DEM_loader).c \
arrayinit_$(arrayinit).c \
initflow_$(initflow).c \
pulse_$(pulse).c \
distribute_$(distribute).c \
neighbor_$(neighbor_ID).c \
output_$(output).c \
set_flow_params$(params).c \
choose_vent_$(newvent).c \
check_vent$(check_vent).c \
# activate_$(activate).c

OBJ = $(SRCS:.c=.o)

MAIN = molasses.ljc

all:	$(MAIN)
		@echo "*** $(MAIN) has been compiled. ***"

$(MAIN): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $+ $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

$(OBJ): include/structs_LJC2.h include/prototypes_LJC2.h

.PHONY:	clean install

clean:
	$(RM) *.o *~ $(MAIN)

install:
	install -d ../bin
	install -m 0755 $(MAIN) ../bin

uninstall:
	-rm $(BINDIR)/$(MAIN) &>/dev/null

