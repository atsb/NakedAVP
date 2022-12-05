#include <math.h>

#include "list_tem.hpp"

#include "zsp.hpp"

ZSP_Data::~ZSP_Data()
{
	while (zone_array.size())
	{
		zone_array.delete_first_entry();
	}
}

ZSP_Data::ZSP_Data (const char * zdata, size_t /*zsize*/)
{
	const char * ptr = zdata;

	num_x_cubes = *((int *) ptr);
	ptr += 4;

	num_y_cubes = *((int *) ptr);
	ptr += 4;

	num_z_cubes = *((int *) ptr);
	ptr += 4;

	cube_size = *((double *) ptr);
	ptr += 8;

	cube_radius = *((double *) ptr);
	ptr += 8;
	
	int i,j;
	
	int numzones = num_x_cubes*num_y_cubes*num_z_cubes;
	
	for (i=0; i<numzones; i++)
	{
		ZSP_zone tmpzone;
	
		tmpzone.num_z_polys = *((int *) ptr);
		ptr += 4;

		tmpzone.num_z_verts = *((int *) ptr);
		ptr += 4;
		
		if (tmpzone.num_z_polys)
			tmpzone.z_poly_list = new int [tmpzone.num_z_polys];
		
		for (j=0; j<tmpzone.num_z_polys; j++)
		{
			tmpzone.z_poly_list[j] = *((int *) ptr);
			ptr += 4;
		}
		
		if (tmpzone.num_z_verts)
			tmpzone.z_vert_list = new int [tmpzone.num_z_verts];
		
		for (j=0; j<tmpzone.num_z_verts; j++)
		{
			tmpzone.z_vert_list[j] = *((int *) ptr);
			ptr += 4;
		}
		
		zone_array.add_entry(tmpzone);
	}

}	


ZSP_zone::ZSP_zone ()
{
	num_z_polys = 0;
	z_poly_list = 0;
	num_z_verts = 0;
	z_vert_list = 0;
}	

ZSP_zone::~ZSP_zone ()
{
	if (num_z_polys)
		delete [] z_poly_list;
	if (num_z_verts)
		delete [] z_vert_list;
}	

ZSP_zone::ZSP_zone (const ZSP_zone &zz)
{
	if (zz.num_z_polys)	
	{
		num_z_polys = zz.num_z_polys;
		z_poly_list = new int [num_z_polys];
		
		int i;
		
		for (i=0; i<num_z_polys; i++)
		{
			z_poly_list[i] = zz.z_poly_list[i];
		}
	}
	else
	{
		z_poly_list = 0;
		num_z_polys = 0;
	}


	if (zz.num_z_verts)	
	{
		num_z_verts = zz.num_z_verts;
		z_vert_list = new int [num_z_verts];
		
		int i;
		
		for (i=0; i<num_z_verts; i++)
		{
			z_vert_list[i] = zz.z_vert_list[i];
		}
	}
	else
	{
		z_vert_list = 0;
		num_z_verts = 0;
	}
	
}

ZSP_zone & ZSP_zone::operator=(const ZSP_zone &zz)
{
	
	if (num_z_polys)
		delete [] z_poly_list;
	if (num_z_verts)
		delete [] z_vert_list;

	if (zz.num_z_polys)	
	{
		num_z_polys = zz.num_z_polys;
		z_poly_list = new int [num_z_polys];
		
		int i;
		
		for (i=0; i<num_z_polys; i++)
		{
			z_poly_list[i] = zz.z_poly_list[i];
		}
	}
	else
	{
		z_poly_list = 0;
		num_z_polys = 0;
	}


	if (zz.num_z_verts)	
	{
		num_z_verts = zz.num_z_verts;
		z_vert_list = new int [num_z_verts];
		
		int i;
		
		for (i=0; i<num_z_verts; i++)
		{
			z_vert_list[i] = zz.z_vert_list[i];
		}
	}
	else
	{
		z_vert_list = 0;
		num_z_verts = 0;
	}
	
	return(*this);
	
}
	

unsigned char operator==(const ZSP_zone &z1, const ZSP_zone &z2)
{
	return(&z1 == &z2);
}	

unsigned char operator!=(const ZSP_zone &z1, const ZSP_zone &z2)
{
	return(&z1 != &z2);
}	



/////////////////////////////////////////

// Class Shape_ZSP_Data_Chunk functions

size_t Shape_ZSP_Data_Chunk::size_chunk ()
{
	int sz = 12 + 12 + 16;

	ZSP_Data * zdata = (ZSP_Data *)(&zspdata);

	for (LIF<ZSP_zone> znl(&zdata->zone_array); !znl.done(); znl.next())
	{
		sz += 8 + (znl().num_z_polys * 4) + (znl().num_z_verts * 4);
	}

	return (chunk_size = sz);
}

void Shape_ZSP_Data_Chunk::fill_data_block ( char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = zspdata.num_x_cubes;
	data_start += 4;
	
	*((int *) data_start) = zspdata.num_y_cubes;
	data_start += 4;

	*((int *) data_start) = zspdata.num_z_cubes;
	data_start += 4;

	*((double *) data_start) = zspdata.cube_size;
	data_start += 8;

	*((double *) data_start) = zspdata.cube_radius;
	data_start += 8;

	ZSP_Data * zdata = (ZSP_Data *)(&zspdata);

	for (LIF<ZSP_zone> znl(&zdata->zone_array); !znl.done(); znl.next())
	{
		*((int *) data_start) = znl().num_z_polys;
		data_start += 4;

		*((int *) data_start) = znl().num_z_verts;
		data_start += 4;

		int i;
		for (i=0; i<znl().num_z_polys; i++)
		{
			*((int *) data_start) = znl().z_poly_list[i];
			data_start += 4;
		}

		for (i=0; i<znl().num_z_verts; i++)
		{
			*((int *) data_start) = znl().z_vert_list[i];
			data_start += 4;
		}
	}
}	
