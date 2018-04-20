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
