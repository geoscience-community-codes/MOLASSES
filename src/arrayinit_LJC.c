/*############################################################################
# MOLASSES (MOdular LAva Simulation Software for the Earth Sciences) 
# The MOLASSES model relies on a cellular automata algorithm to 
# estimate the area inundated by lava flows.MOLASSES 
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
#include "include/prototypes_LJC2.h"

/*Reserves memory for active list with size [CAListSize] With the typedef of ActiveList. */
ActiveList *ACTIVELIST_INIT(
unsigned int CAListSize)
{
	ActiveList *m = NULL;
	
	/*Allocate active list*/
	m = (ActiveList*) GC_MALLOC( (size_t)(CAListSize) * sizeof(ActiveList) );
	if (m == NULL) 
	{
		fprintf(stderr, "[ACTIVELIST_INIT]\n");
		fprintf(stderr, "   NO MORE MEMORY: Tried to allocate memory Active Lists!! Program stopped!\n");
		return NULL;
	}
	return (m);
}

/*Reserves memory for matrix of size [rows]x[cols] With the typedef of DataCell.*/
DataCell **GLOBALDATA_INIT(
int rows, 
int cols)
{
	int i;
	DataCell **m = NULL;
	
	/*Allocate row pointers*/
	if((m = (DataCell**) GC_MALLOC((size_t)(rows) * sizeof(DataCell*) )) == NULL)
	{
		fprintf(stderr, "[GLOBALDATA_INIT]\n");
		fprintf(stderr, "   NO MORE MEMORY: Tried to allocate memory for %d Rows!! Program stopped!\n", rows);
		return NULL;
	}
	/*allocate cols & set previously allocated row pointers to point to these*/
	for (i = 0; i < rows; i++) 
	{
		if((m[i] = (DataCell*) GC_MALLOC((size_t)(cols) * sizeof(DataCell) )) == NULL)
		{
			fprintf(stderr, "[GLOBALDATA_INIT]\n");
			fprintf(stderr, "   NO MORE MEMORY: Tried to allocate memory for %d cols in %d rows!! Program stopped!", cols,rows);
			return NULL;
		}
	}
	return m; /*return array */
}
