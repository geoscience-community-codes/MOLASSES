# CFLAGS = -Wall -pedantic -g Wno-long-long
CFLAGS = -Wall -O2
INCLUDES = -I$(GDAL_INCLUDE_PATH) -I../include -I./include
# LIBS = -L$(GDAL_LIB_PATH) -lgdal -L../lib -lgc -lran -lm
# If you set a specific path for your GDAL libraries, this will look for it!
ifndef GDAL_LIB_PATH
	LIBS = -lgdal -L../lib -lgc -lran -lm
else
	LIBS = -L$(GDAL_LIB_PATH) -lgdal -L../lib -lgc -lran -lm
endif

SRCS = driver_$(driver).c \
initialize_$(initialize).c \
demloader_$(DEM_loader).c \
arrayinit_$(array_initializer).c \
initflow_$(flow_initializer).c \
pulse_$(pulse).c \
distribute_$(distribute).c \
neighbor_$(neighbor_ID).c \
output_$(output).c \
set_flow_params.c \
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

$(OBJ): include/structs_LJC.h include/prototypes_LJC.h

.PHONY:	clean install

clean:
	$(RM) *.o *~ $(MAIN)

install:
	install -d ../bin
	install -m 0755 $(MAIN) ../bin

uninstall:
	-rm $(BINDIR)/$(MAIN) &>/dev/null

