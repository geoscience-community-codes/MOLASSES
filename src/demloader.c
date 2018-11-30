#include "include/prototypes_LJC.h"

DataCell **DEM_LOADER(
char *DEMfilename,
double *DEMGeoTransform,
DataCell **grid, 
char *modeltype) {
/*
MODULE: DEM_LOADER_GDAL
Accepts a file name and a null data grid
Checks for validity of raster file
Load raster file metadata into array (DEMGeoTransform)
Load Raster Data into DataCell grid array depending on Raster Type:
TOPOG: DEMgrid.dem_elev (elevation)
T_UNC: DEMgrid.elev_uncert (grid cell uncertainty)
RESID: DEMgrid.residual (modal flow residual)
	
RETURN:
DataCell **grid, or NULL on error
	

DEMGeoTransform[0] lower left x
DEMGeoTransform[1] w-e pixel resolution (positive value)
DEMGeoTransform[2] number of cols, assigned manually in this module 
DEMGeoTransform[3] lower left y
DEMGeoTransform[4] number of rows, assigned manually in this module
DEMGeoTransform[5] n-s pixel resolution (negative value) 
	
*/
	
	GDALDatasetH    DEMDataset;/*The raster file*/
	GDALDriverH     DEMDriver; /*essentially the raster File Type*/
	GDALRasterBandH DEMBand;
    
	float *pafScanline;
	int type = -1;
	int YOff;
	int i,j;
	float k;
	DataCell **local_grid;	

	GDALAllRegister();
	DEMDataset = GDALOpen( DEMfilename, GA_ReadOnly ); /* Open file */
	if(DEMDataset == NULL){
		fprintf(stderr, "ERROR [DEM_LOADER]: File=[%s] could not be opened!\n", DEMfilename);
		return NULL;
	}
	
	/*Make sure Projection metadata is valid (that the raster is a valid raster)*/
	if( GDALGetProjectionRef( DEMDataset ) == NULL ){
		fprintf(stderr, "ERROR [DEM_LOADER]: File=[%s] could not be read!\n", DEMfilename);
		return NULL;
	}
	
	/*Read DEM raster metadata into DEMGeoTransform*/
	if( GDALGetGeoTransform( DEMDataset, DEMGeoTransform ) != CE_None )
	{
		fprintf(stderr, "ERROR [DEM_LOADER]: Data from [%s]could not be loaded!\n",DEMfilename);
		return NULL;
	}
	
	DEMDriver = GDALGetDatasetDriver( DEMDataset ); 	/* Find out raster type (e.g. tiff, netcdf)*/
	DEMGeoTransform[5] = -1 * DEMGeoTransform[5]; /*row height*/
	DEMGeoTransform[4] = GDALGetRasterYSize( DEMDataset );
	DEMGeoTransform[2] = GDALGetRasterXSize( DEMDataset );
	DEMGeoTransform[3] -= (DEMGeoTransform[5] * DEMGeoTransform[4]);
	
	fprintf(stdout, "\nDEM Information [%s]:\n", GDALGetDriverLongName(DEMDriver)); 
	fprintf(stdout, "  File:              %s\n", DEMfilename);
	fprintf(stdout, "  Lower Left Origin: (%.6f,%.6f)\n", DEMGeoTransform[0], DEMGeoTransform[3]);
	fprintf(stdout, "  GMT Range Code:    -R%.3f/%.3f/%.3f/%.3f\n",
	        DEMGeoTransform[0],
	       (DEMGeoTransform[0]+((DEMGeoTransform[2]-1)*DEMGeoTransform[1])),
	        DEMGeoTransform[3],
	       (DEMGeoTransform[3]+((DEMGeoTransform[4]-1)*DEMGeoTransform[5])));
	fprintf(stdout, "  Pixel Size:        (%.6f,%.6f)\n",
	       DEMGeoTransform[5], DEMGeoTransform[1]);
	fprintf(stdout, "  Grid Size:         (%d,%d)\n",
	       (int)DEMGeoTransform[4], (int)DEMGeoTransform[2]);
	
	if(!strcmp(modeltype,"TOPOG")) {
		type = Topog;
		fprintf(stdout, "              Creating ELEVATION Grid...\n");
		local_grid = GLOBALDATA_INIT(DEMGeoTransform[4], DEMGeoTransform[2]);
        if (local_grid == NULL) return NULL;
        else grid = local_grid;
	} 
	else if(!strcmp(modeltype,"RESID")) {
		fprintf(stdout, "              Creating RESIDUAL Grid...\n");
		type = Resid;
	}
	else if(!strcmp(modeltype, "T_UNC")) {
		fprintf(stdout, "              Creating ELEVATION UNCERTAINTY Grid...\n");
		type = T_unc;
	}
		
	DEMBand = GDALGetRasterBand(DEMDataset, 1); /* 1 band in raster */
	pafScanline = (float *) CPLMalloc(sizeof(float) * DEMGeoTransform[2]); /* Allocate for 1 row of data */
	 	
	for(i=0; i < DEMGeoTransform[4]; i++) { /*For each row*/
        YOff = (DEMGeoTransform[4]-1) - i; /* bottom row is read in first*/
		if((GDALRasterIO(DEMBand, /*Read elevation data from row in input raster*/
            GF_Read, 0, YOff, DEMGeoTransform[2], 1, pafScanline, DEMGeoTransform[2], 1, GDT_Float32, 0, 0)) != CE_None) {
            fprintf(stderr, 
                "\nERROR [DEM_LOADER]: DEM file [%s] could not be read!\n", DEMfilename);
            return NULL;
		}
	
		if((i % 50) == 0) { /*Command line Status Bar*/
			k = 0;
			fprintf(stdout, "\r");
			while(k < (DEMGeoTransform[4] - 1)){		
				if (k < i) fprintf(stdout, "=");
				else if((k - ((DEMGeoTransform[4] - 1) / 60)) < i) fprintf(stdout, ">");
				else fprintf(stdout, " ");
				k += DEMGeoTransform[4] / 60;
			} fprintf(stdout, "| %3d%%",(int) (100 * i / (DEMGeoTransform[4] -1)));
		}
	
		for (j = 0; j < DEMGeoTransform[2]; j++) { /*Write elevation column by column into 2D array*/
			if (type == Topog) {
				(grid)[i][j].dem_elev = pafScanline[j];
				(grid)[i][j].eff_elev = (grid)[i][j].dem_elev;
				(grid)[i][j].parentcode = 0;
				(grid)[i][j].active = -1;
				(grid)[i][j].hit_count = 0; /* Initialize hit count */
			} 
			else if (type == Resid) 
				(grid)[i][j].residual = pafScanline[j];
			else if (type == T_unc) 
				(grid)[i][j].elev_uncert = pafScanline[j];
		}
	}
	fprintf(stdout, "\n DEM Loaded.\n\n");
	CPLFree(pafScanline);
	fflush(stdout);
	return(grid);
}
