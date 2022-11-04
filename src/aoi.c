/* AOI.c */

#include "include/prototypes_LJC3.h"
#define DIAGONAL_DISTANCE(x,y) (sqrt(((x)*(x))+((y)*(y))))

int add_aoi_to_dem(
  DataCell **grid,
  double *geotransform, 
  AOI *aoi
  ) 
{
  int row, col, ct = 0;
  double x = 0, y = 0;
  double west = floor(aoi->easting - aoi->radius);
  double east = ceil(aoi->easting + aoi->radius);
  double south = floor(aoi->northing - aoi->radius);
  double north = ceil(aoi->northing + aoi->radius);
  
  double easting;
  double northing;
  FILE *OUT;
  
  OUT = fopen("aoi.out", "w"); /*open configuration file*/
	if (OUT == NULL) 
	{
		fprintf(stderr, 
					"\nERROR [INITIALIZE]: Cannot open aoi output file=[%s]:[%s]!\n", 
					"aoi.out", strerror(errno));
		return 0;
	}
  /* Find and tag DEM cells that are within the AOI */
  for(row = 0; row < geotransform[4]; row++) { 
		for(col = 0; col < geotransform[2]; col++) {
		  easting = geotransform[0] + (geotransform[1] * col);
      northing = geotransform[3] + (geotransform[5] * row);
      if (easting > west && easting < east) {
        if (northing > south && northing < north) {
          x = easting - aoi->easting;
          y = northing - aoi->northing;
          if (DIAGONAL_DISTANCE(x,y) < aoi->radius) {
            grid[row][col].aoi = 1;
            fprintf(OUT, "%lf %lf\n", easting,northing );
            ct++;
          }
          else grid[row][col].aoi = 0;
        }
      }
    }
  }
  fclose(OUT);
  return ct;
} 		  
