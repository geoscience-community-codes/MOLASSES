/*############################################################################
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
###########################################################################*/    

#include "include/prototypes_LJC.h"

int CHECK_VENT_LOCATION(
VentArr *vent, 
double *gridInfo,
DataCell **grid)
{
	int i=1, ventRow, ventCol;
  
	if((ventRow = (int) ( ( vent->northing - gridInfo[3]) / gridInfo[5]) ) <= 0) 
	{
		fprintf(stderr, "[CHECK_VENT]: Vent not within region covered by DEM! (SOUTH of region)\n");
		fprintf(stderr, " Vent #%d at cell: [%d][%d].\n",
							(i), ventRow, (int) (( vent->easting - gridInfo[0]) / gridInfo[1]) );
		return 1;
	}
	else if (ventRow >= gridInfo[4]) 
	{
		fprintf(stderr, "[CHECK_VENT]: Vent not within region covered by DEM! (NORTH of region)\n");
		fprintf(stderr, " Vent #%d at cell: [%d][%d].\n",
							(i), ventRow, (int) (( vent->easting - gridInfo[0]) / gridInfo[1]) );
		return 1;
	}
	else if((ventCol = (int) ((vent->easting - gridInfo[0]) / gridInfo[1]) ) <= 0) 
	{
		fprintf(stderr, "[CHECK_VENT]: Vent not within region covered by DEM! (WEST of region)\n");
		fprintf(stderr, " Vent #%d at cell: [%d][%d].\n",
							(i), ventRow, ventCol);
		return 1;
	}
	else if(ventCol >= gridInfo[2]) 
	{
		fprintf(stderr, "[CHECK_VENT]: Vent not within region covered by DEM! (EAST of region)\n");
		fprintf(stderr, " Vent #%d at cell: [%d][%d].\n",
							(i), ventRow, ventCol);
		return 1;
	}
	else if ( grid[ventRow][ventCol].dem_elev < 0)
	{
		fprintf(stderr, "[CHECK_VENT]: Vent not within region covered by DEM! (below sea-level)\n");
		fprintf(stderr, " Vent #%d at cell: [%d][%d].\n",
							(i), ventRow, ventCol);
		return 1;
}

	return 0;
}
