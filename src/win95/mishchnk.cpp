#include "chunk.hpp"
#include "chnktype.hpp"
#include "mishchnk.hpp"

#include "shpchunk.hpp"
#include "obchunk.hpp"



#include "huffman.hpp"


// Class Lockable_Chunk_With_Children functions

extern char * users_name;

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(mishchnk)

BOOL Lockable_Chunk_With_Children::lock_chunk(File_Chunk & fchunk)
{
	if (!fchunk.filename) return FALSE;

	if (local_lock) return FALSE; // you can't lock a chunk twice

	#if DisableLock
	if(external_lock) return FALSE;
	local_lock = TRUE;
	set_lock_user (users_name);
	return TRUE;
	#else
	if (!fchunk.check_file()) return FALSE;
	if (updated_outside || external_lock) return FALSE;

	HANDLE rif_file;
	unsigned long bytes_read;

	int tries = 0;
	
 	rif_file = CreateFile (fchunk.filename, GENERIC_WRITE + GENERIC_READ, 0, 0, OPEN_EXISTING, 
 					FILE_FLAG_RANDOM_ACCESS, 0);

	while(rif_file == INVALID_HANDLE_VALUE && tries<10)
	{
	 	rif_file = CreateFile (fchunk.filename, GENERIC_WRITE + GENERIC_READ, 0, 0, OPEN_EXISTING, 
	 					FILE_FLAG_RANDOM_ACCESS, 0);
	 	tries ++;
		clock_t ctime = clock();
		double secs	= (double)ctime / (double)CLOCKS_PER_SEC;
		double secsgone;
		do
		{
			ctime = clock();
			secsgone = (double)ctime / (double)CLOCKS_PER_SEC;
		}
		while( (secsgone-secs)<1);
	}	
		
	if (rif_file == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	
	SetFilePointer (rif_file,0,0,FILE_BEGIN);
	
	List<int> chfptrs;
	list_chunks_in_file(& chfptrs, rif_file, identifier);

	LIF<int> cfpl(&chfptrs);

	if (chfptrs.size()) {
		for (; !cfpl.done(); cfpl.next()) {

			SetFilePointer (rif_file, cfpl(),0,FILE_BEGIN);

			if (file_equals(rif_file)) break;
		}
	}

	if (!cfpl.done()) {

		// go to start of chunk
		SetFilePointer (rif_file,cfpl(),0,FILE_BEGIN);
		
		// get header list
		if (get_head_id()) 
		{
			List<int> obhead;
			list_chunks_in_file(& obhead, rif_file, get_head_id());

			assert (obhead.size() == 1);

			int flags;

			// go to lock status in header
			SetFilePointer(rif_file,obhead.first_entry() + 12,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &flags, 4, &bytes_read, 0);
			SetFilePointer(rif_file,-4,0,FILE_CURRENT);
			flags |= GENERAL_FLAG_LOCKED;
			WriteFile (rif_file, (long *) &flags, 4, &bytes_read, 0);
			WriteFile (rif_file, (long *) &users_name[0], 16, &bytes_read, 0);
		}

	}

	local_lock = TRUE;

	set_lock_user (users_name);

	CloseHandle (rif_file);
	return TRUE;
	#endif	
}	

BOOL Lockable_Chunk_With_Children::unlock_chunk (File_Chunk & fchunk, BOOL updateyn)
{
	if (updateyn)	{
		updated = TRUE;
	}
	#if DisableLock
	local_lock = FALSE;
	(void)fchunk;
	#else
	else
	{
		fchunk.check_file();
		if (updated_outside || external_lock) return FALSE;

		HANDLE rif_file;
		unsigned long bytes_read;
	
		rif_file = CreateFile (fchunk.filename, GENERIC_WRITE + GENERIC_READ, 0, 0, OPEN_EXISTING, 
							FILE_FLAG_RANDOM_ACCESS, 0);
		
		if (rif_file == INVALID_HANDLE_VALUE) {
			return FALSE;
		}
	
		SetFilePointer (rif_file,0,0,FILE_BEGIN);
		
		List<int> chfptrs;
		list_chunks_in_file(& chfptrs, rif_file, identifier);

		List<obinfile> obs;

		char name[50];

		LIF<int> ofpl(&chfptrs);

		if (chfptrs.size()) {
			for (; !ofpl.done(); ofpl.next()) {

				SetFilePointer (rif_file, ofpl(),0,FILE_BEGIN);

				if (file_equals(rif_file)) break;
			}
		}

		if (!ofpl.done()) {

			// go to start of chunk
			SetFilePointer (rif_file,ofpl(),0,FILE_BEGIN);
			
			// get header list
			if (get_head_id()) 
			{
				List<int> obhead;
				list_chunks_in_file(& obhead, rif_file, get_head_id());

				assert (obhead.size() == 1);

				int flags;

				// go to lock status in header
				SetFilePointer(rif_file,obhead.first_entry() + 12,0,FILE_BEGIN);
				ReadFile (rif_file, (long *) &flags, 4, &bytes_read, 0);
				SetFilePointer(rif_file,-4,0,FILE_CURRENT);
				flags &= ~GENERAL_FLAG_LOCKED;
				WriteFile (rif_file, (long *) &flags, 4, &bytes_read, 0);
				WriteFile (rif_file, (long *) &users_name[0], 16, &bytes_read, 0);
			}

		}
		
		local_lock = FALSE;

		CloseHandle (rif_file);
	}
	#endif


	return TRUE;

}


BOOL Lockable_Chunk_With_Children::update_chunk_in_file(HANDLE &rif_file)
{

	unsigned long bytes_read;
	int length = 0;

	const char * hd_id = get_head_id();

	if (!hd_id) return FALSE;

	SetFilePointer (rif_file,0,0,FILE_BEGIN);

	//twprintf("\nLooking for chunks in file\n");

	List<int> shpfptrs;
	list_chunks_in_file(& shpfptrs, rif_file, identifier);

	// look through chunks for the save of our current chunk

	//twprintf("Checking each chunk\n");

	LIF<int> sfpl(&shpfptrs);

	if (shpfptrs.size()) {
		for (; !sfpl.done(); sfpl.next()) {

			SetFilePointer (rif_file, sfpl()+8,0,FILE_BEGIN);

			ReadFile (rif_file, (long *) &(length), 4, &bytes_read, 0);

			SetFilePointer (rif_file, sfpl(),0,FILE_BEGIN);
			if (file_equals(rif_file)) break;
		}
	}

	// then load the file after that chunk into a buffer,
	// output the chunk and write the buffer
	// unless the chunk is the last one in the file

	//twprintf("Updating file\n");

	if (!sfpl.done())
	{

		int file_length = GetFileSize(rif_file,0);

		if (file_length > (sfpl() + length)) {
							
			SetFilePointer (rif_file, sfpl() + length,0,FILE_BEGIN);

			char * tempbuffer;
			tempbuffer = new char [file_length - (sfpl() + length)];

			ReadFile (rif_file, (long *) tempbuffer, (file_length - (sfpl() + length)), &bytes_read, 0);

			SetFilePointer (rif_file, sfpl() ,0,FILE_BEGIN);
			
			if (!deleted)
			{
				size_chunk();
				output_chunk(rif_file);
			}
			
			WriteFile (rif_file, (long *) tempbuffer, (file_length - (sfpl() + length)), &bytes_read, 0);

			delete [] tempbuffer;

			SetEndOfFile (rif_file);
		}
		else{

			SetFilePointer (rif_file, sfpl() ,0,FILE_BEGIN);
			if (!deleted)
			{
				size_chunk();
				output_chunk(rif_file);
			}
			SetEndOfFile (rif_file);
		}

	}
	else {

		SetFilePointer (rif_file,0 ,0,FILE_END);
		if (!deleted)
		{
			size_chunk();
			output_chunk(rif_file);
		}
		SetEndOfFile (rif_file);
	}

	local_lock = FALSE;

	updated = FALSE;
	
	int file_length = GetFileSize(rif_file,0);
	SetFilePointer (rif_file,8,0,FILE_BEGIN);

	WriteFile (rif_file, (long *) &file_length, 4, &bytes_read, 0);

	
	// DO NOT PUT ANY CODE AFTER THIS
	
	if (deleted) delete this;
	
	// OR ELSE !!!
	
	return TRUE;

}


size_t Lockable_Chunk_With_Children::size_chunk_for_process()
{
	if (output_chunk_for_process)
		return size_chunk();
	return(chunk_size = 0);
}


void Lockable_Chunk_With_Children::fill_data_block_for_process(char * data_start)
{
	if (output_chunk_for_process)
	{
		fill_data_block(data_start);
		output_chunk_for_process = FALSE;
	}
}




///////////////////////////////////////

// Class File_Chunk functions

/*
Children for File_Chunk :
"REBSHAPE"		Shape_Chunk
"RSPRITES"		AllSprites_Chunk
"RBOBJECT"		Object_Chunk
"RIFVERIN"		RIF_Version_Num_Chunk
"REBENVDT"		Environment_Data_Chunk
"REBENUMS"		Enum_Chunk
"OBJCHIER"		Object_Hierarchy_Chunk
"OBHALTSH"		Object_Hierarchy_Alternate_Shape_Set_Chunk
"HIDEGDIS"		Hierarchy_Degradation_Distance_Chunk
"INDSOUND"		Indexed_Sound_Chunk
"HSETCOLL"		Hierarchy_Shape_Set_Collection_Chunk
"DUMMYOBJ"		Dummy_Object_Chunk

*/

File_Chunk::File_Chunk(const char * file_name)
: Chunk_With_Children (NULL, "REBINFF2")
{
// Load in whole chunk and traverse
	char rifIsCompressed = FALSE;
	char *uncompressedData = NULL;
	FILE *rif_file;
	DWORD file_size;
	DWORD file_size_from_file;
	char * buffer;
	char * buffer_ptr;
	char id_buffer[9];

	Parent_File = this;

	error_code = 0;
	object_array_size=0;
	object_array=0;

	filename = new char [strlen(file_name) + 1];

	strcpy (filename, file_name);

	rif_file = OpenGameFile(file_name, FILEMODE_READONLY, FILETYPE_PERM);

	if (rif_file == NULL) {
		error_code = CHUNK_FAILED_ON_LOAD;
		return;
	}

	fseek(rif_file, 0, SEEK_END);
	file_size = ftell(rif_file);
	rewind(rif_file);


	if (fread(id_buffer, 1, 8, rif_file) != 8) {	
		error_code = CHUNK_FAILED_ON_LOAD;
		fclose(rif_file);
		return;
	}	

	/* KJL 16:46:14 19/09/98 - check for a compressed rif */
	if (!strncmp (id_buffer, COMPRESSED_RIF_IDENTIFIER, 8))
	{
		rifIsCompressed = TRUE;
	}	
	else if (strncmp (id_buffer, "REBINFF2", 8))
	{
		error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
		fclose(rif_file);
		return;
	}	

	buffer = new char [file_size];

	/* KJL 17:57:44 19/09/98 - if the rif is compressed, we must load the whole
	file in and then pass it to the decompression routine, which will return a
	pointer to the original data. */
	if (rifIsCompressed)
	{
		if (fread(buffer+8, 1, (file_size-8), rif_file) != (file_size-8)) {
			error_code = CHUNK_FAILED_ON_LOAD;
			fclose(rif_file);
			return;
		}	
		uncompressedData = HuffmanDecompress((HuffmanPackage*)buffer); 		
		file_size = ((HuffmanPackage*)buffer)->UncompressedDataSize;
		
		delete [] buffer; // kill the buffer the compressed file was loaded into

		buffer_ptr = buffer = uncompressedData+12; // skip header data
	}
	else // the normal uncompressed approach:
	{
		if (fread(&file_size_from_file, 1, 4, rif_file) != 4) {
			error_code = CHUNK_FAILED_ON_LOAD;
			fclose(rif_file);
			return;
		}	

		if (file_size != file_size_from_file) {
			error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			fclose(rif_file);
			return;
		}	

		if (fread(buffer, 1, (file_size-12), rif_file) != (file_size-12)) {
			error_code = CHUNK_FAILED_ON_LOAD;
			fclose(rif_file);
			return;
		}
		buffer_ptr = buffer;
	}

	fclose(rif_file);
	
	// Process the RIF
	// The start of the first chunk

	while ((buffer_ptr-buffer)< ((signed) file_size-12) && !error_code) {

		if ((*(int *)(buffer_ptr + 8)) + (buffer_ptr-buffer) > ((signed)file_size-12)) {
			error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		DynCreate(buffer_ptr);
		buffer_ptr += *(int *)(buffer_ptr + 8);
	}

	/* KJL 17:59:42 19/09/98 - release the memory holding the rif */
	if (rifIsCompressed)
	{
		free(uncompressedData);
	}
	else
	{
		delete [] buffer;
	}

	post_input_processing();
}

File_Chunk::File_Chunk()
: Chunk_With_Children (NULL, "REBINFF2")
{
// Empty File chunk
	new RIF_Version_Num_Chunk (this);
	filename = 0;

	object_array_size=0;
	object_array=0;
}


File_Chunk::~File_Chunk()
{
	if (filename)
		delete [] filename;

	if(object_array)
		free(object_array);
}

#define SAVE_USING_COMPRESSION 1

BOOL File_Chunk::write_file (const char * fname)
{
	if(!fname) return FALSE;
	//if a read_only file exists with this filename , then abort attempt to save
	DWORD attributes = GetFileAttributesA(fname);
	if (0xffffffff!=attributes)
	{
		if (attributes & FILE_ATTRIBUTE_READONLY)
		{
			return FALSE;
		}
	}
	
	
	
	HANDLE rif_file;

	if (filename) delete [] filename;

	filename = new char [strlen(fname) + 1];
	strcpy (filename, fname);
		
	//save under a temporary name in case a crash occurs during save;
	int filename_start_pos=0;
	int pos=0;
	while(fname[pos])
	{
		if(fname[pos]=='\\' || fname[pos]==':')
		{
			filename_start_pos=pos+1;
		}
		//go to next MBCS character in string
		pos+=_mbclen((unsigned const char*)&fname[pos]);
	}
	if(!fname[filename_start_pos]) return FALSE;
	
	char* temp_name=new char[strlen(fname)+7];
	strcpy(temp_name,fname);
	strcpy(&temp_name[filename_start_pos],"~temp~");
	strcpy(&temp_name[filename_start_pos+6],&fname[filename_start_pos]);

	prepare_for_output();

	
	#if SAVE_USING_COMPRESSION
	//create a block containing the uncompressed rif file
	unsigned char* uncompressedData = (unsigned char*) make_data_block_from_chunk();
	if(!uncompressedData) return FALSE;

	//do the compression thing
	HuffmanPackage *outPackage = HuffmanCompression(uncompressedData,chunk_size);
	delete [] uncompressedData;
	if(!outPackage) return FALSE;

	//and now try to write the file
	rif_file = CreateFileA (temp_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 
					FILE_FLAG_RANDOM_ACCESS, 0);

	if (rif_file == INVALID_HANDLE_VALUE) {
		delete [] temp_name;
		free (outPackage);
		return FALSE;
	}
		
	unsigned long junk;
	BOOL ok;

	ok = WriteFile (rif_file, (long *) outPackage,outPackage->CompressedDataSize+sizeof(HuffmanPackage), &junk, 0);

	CloseHandle (rif_file);

	free (outPackage);

	if(!ok) return FALSE;
	
	
	#else
	size_chunk();
	
	rif_file = CreateFileA (temp_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 
					FILE_FLAG_RANDOM_ACCESS, 0);

	if (rif_file == INVALID_HANDLE_VALUE) {
		delete [] temp_name;
		return FALSE;
	}

	if (!(this->output_chunk(rif_file)))
	{
		CloseHandle (rif_file);
		delete [] temp_name;
		return FALSE;
	}

	CloseHandle (rif_file);
	#endif

	//Delete the old file with this name (if it exists) , and rename the temprary file
	DeleteFileA(fname);
	MoveFileA(temp_name,fname);

	delete [] temp_name;

	return TRUE;
}	

// the file_chunk must link all of its shapes & objects together

void File_Chunk::post_input_processing()
{
	List<Shape_Chunk *> shplist;
	List<Object_Chunk *> objlist;

	List<Chunk *> child_lists;

	lookup_child("REBSHAPE",child_lists);

	while (child_lists.size()) {
		shplist.add_entry((Shape_Chunk *)child_lists.first_entry());
		child_lists.delete_first_entry();
	}

	lookup_child("RBOBJECT",child_lists);

	while (child_lists.size()) {
		objlist.add_entry((Object_Chunk *)child_lists.first_entry());
		child_lists.delete_first_entry();
	}

	LIF<Shape_Chunk *> sli(&shplist);
	for (; !sli.done(); sli.next())
	{
		Shape_Chunk::max_id = max (Shape_Chunk::max_id,sli()->get_header()->file_id_num);	
	}
	Shape_Chunk** shape_array=new Shape_Chunk*[Shape_Chunk::max_id+1];

	for(sli.restart();!sli.done();sli.next())
	{
		shape_array[sli()->get_header()->file_id_num]=sli();
	}
	
	for (LIF<Object_Chunk *> ol(&objlist); !ol.done(); ol.next())
	{
		ol()->assoc_with_shape(shape_array[ol()->get_header()->shape_id_no]);
	}

	delete [] shape_array;
	

	Chunk_With_Children::post_input_processing();	
}



BOOL File_Chunk::check_file()
{
	if (!filename) return TRUE;

	#if DisableLock
	return(TRUE);
	#else
	
	int flags;
	int v_no;
	char locker[17];


	HANDLE rif_file;
	unsigned long bytes_read;

	int tries = 0;

	rif_file = CreateFileA (filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 
						FILE_FLAG_RANDOM_ACCESS, 0);

	while(rif_file == INVALID_HANDLE_VALUE && tries<10)
	{
		rif_file = CreateFileA (filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 
						FILE_FLAG_RANDOM_ACCESS, 0);
	 	tries ++;
		clock_t ctime = clock();
		double secs	= (double)ctime / (double)CLOCKS_PER_SEC;
		double secsgone;
		do
		{
			ctime = clock();
			secsgone = (double)ctime / (double)CLOCKS_PER_SEC;
		}
		while( (secsgone-secs)<1);
	}	

	if (rif_file == INVALID_HANDLE_VALUE) {
		error_code = CHECK_FAILED_NOT_OPEN;
		return FALSE;
	}


	if (rif_file == INVALID_HANDLE_VALUE) {
		error_code = CHECK_FAILED_NOT_OPEN;
		return FALSE;
	}

	SetFilePointer (rif_file,0,0,FILE_BEGIN);

	List<int> obfptrs;
	list_chunks_in_file(& obfptrs, rif_file, "RBOBJECT");

// ok, go through the objects, first locate the header and find
// the associated object.

	if (obfptrs.size()) {

		for (LIF<int> obflst(&obfptrs); !obflst.done(); obflst.next()) {
			
			// go to start of chunk
			SetFilePointer (rif_file,obflst(),0,FILE_BEGIN);
			
			// get header list
			List<int> obhead;
			list_chunks_in_file(& obhead, rif_file, "OBJHEAD1");

			assert (obhead.size() == 1);

			// go to lock status in header
			SetFilePointer(rif_file,obhead.first_entry() + 12,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &flags, 4, &bytes_read, 0);
			ReadFile (rif_file, (long *) locker, 16, &bytes_read, 0);

			// go to version number
			SetFilePointer(rif_file,obhead.first_entry() + 88,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &v_no, 4, &bytes_read, 0);

			char name[50];

			// get object identifier
			SetFilePointer(rif_file,obhead.first_entry() + 96,0,FILE_BEGIN);
			int i = 0; 
			do ReadFile (rif_file, (long *) (name + i), 1, &bytes_read, 0);
			while (name[i++] != 0);

			Object_Chunk * tmpob;
			Object_Header_Chunk * tmpobh;
			List<Chunk *> obchs;
			lookup_child ("RBOBJECT",obchs);

			if (obchs.size()){
				for (LIF<Chunk *> obchls(&obchs); !obchls.done(); obchls.next()){
					tmpob = (Object_Chunk *)obchls();
					// if this is the same object
					if (!strcmp (name, tmpob->object_data.o_name)) break;
				}
				if (!obchls.done()) {
					tmpobh = tmpob->get_header();
					if (tmpobh) {
						if (tmpobh->version_no < v_no)
							tmpob->updated_outside = TRUE;
						if (flags & GENERAL_FLAG_LOCKED) {
							// do the lock check
							if (!tmpob->local_lock) {
								tmpob->external_lock = TRUE;
								strncpy(tmpobh->lock_user, locker,16);
								tmpobh->lock_user[16] = '\0';
							}
							else if (strncmp(tmpobh->lock_user, locker,16)) {
								tmpob->external_lock = TRUE;
								strncpy(tmpobh->lock_user, locker,16);
								tmpobh->lock_user[16] = '\0';
							}
						}
					}

				}
				
			}

		}

	}

	// shapes
	
	SetFilePointer (rif_file,0,0,FILE_BEGIN);

	List<int> shpfptrs;
	list_chunks_in_file(& shpfptrs, rif_file, "REBSHAPE");

	if (shpfptrs.size()) {

		for (LIF<int> shpflst(&shpfptrs); !shpflst.done(); shpflst.next()) {
			
			// go to start of chunk
			SetFilePointer (rif_file,shpflst(),0,FILE_BEGIN);
			
			// get header list
			List<int> shphead;
			list_chunks_in_file(& shphead, rif_file, "SHPHEAD1");

			assert (shphead.size() == 1);

			// go to lock status in header
			SetFilePointer(rif_file,shphead.first_entry() + 12,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &flags, 4, &bytes_read, 0);
			ReadFile (rif_file, (long *) locker, 16, &bytes_read, 0);

			int id;			
			// get shape identifier
			ReadFile (rif_file, (long *) &id, 4, &bytes_read, 0);


			// Here we update max_id
			Shape_Chunk::max_id = max (Shape_Chunk::max_id,id);	

			// go to version number
			SetFilePointer(rif_file,shphead.first_entry() + 100,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &v_no, 4, &bytes_read, 0);

			Shape_Chunk * tmpshp;
			Shape_Header_Chunk * tmpshph;
			List<Chunk *> shpchs;
			lookup_child ("REBSHAPE",shpchs);

			if (shpchs.size()){
				for (LIF<Chunk *> shpchls(&shpchs); !shpchls.done(); shpchls.next()){
					tmpshp = (Shape_Chunk *)shpchls();
					tmpshph = tmpshp->get_header();
					// if this is the same object
					if (tmpshph) 
						if (id == tmpshph->file_id_num) break;
				}
				if (!shpchls.done()) {
					if (tmpshph->version_no < v_no)
						tmpshp->updated_outside = TRUE;
					if (flags & GENERAL_FLAG_LOCKED) {
						// do the lock check
						if (!tmpshp->local_lock) {
							tmpshp->external_lock = TRUE;
							strncpy(tmpshph->lock_user, locker,16);
							tmpshph->lock_user[16] = '\0';
						}
						else if (strncmp(tmpshph->lock_user, locker,16)) {
							tmpshp->external_lock = TRUE;
							strncpy(tmpshph->lock_user, locker,16);
							tmpshph->lock_user[16] = '\0';
						}
					}
					
				}
				
			}

		}

	}

	// sprites
	
	SetFilePointer (rif_file,0,0,FILE_BEGIN);

	List<int> sprfptrs;
	list_chunks_in_file(& sprfptrs, rif_file, "RSPRITES");
	List<Chunk *> sprchlst;
	lookup_child("RSPRITES",sprchlst);

	AllSprites_Chunk * sprch;
	AllSprites_Header_Chunk * sprhead = 0;

	if (sprchlst.size())
	{
		sprch = (AllSprites_Chunk *)sprchlst.first_entry();
		sprhead = sprch->get_header();
	}

	if (sprhead)
	{
		if (sprfptrs.size())
		{
			SetFilePointer (rif_file,sprfptrs.first_entry(),0,FILE_BEGIN);

			// get header list
			List<int> sprchhl;
			list_chunks_in_file(& sprchhl, rif_file, "ASPRHEAD");

			assert (sprchhl.size() == 1);

			// go to lock status in header
			SetFilePointer(rif_file,sprchhl.first_entry() + 12,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &flags, 4, &bytes_read, 0);
			ReadFile (rif_file, (long *) locker, 16, &bytes_read, 0);

			// go to version number
			SetFilePointer(rif_file,sprchhl.first_entry() + 32,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &v_no, 4, &bytes_read, 0);

			if (sprhead->version_no < v_no)
			{
				sprch->updated_outside = TRUE;
			}
			if (flags & GENERAL_FLAG_LOCKED) 
			{
				if (!sprch->local_lock) {
					sprch->external_lock = TRUE;
					strncpy(sprhead->lock_user, locker,16);
					sprhead->lock_user[16] = '\0';
				}
				else if (strncmp(sprhead->lock_user, locker,16)) {
					sprch->external_lock = TRUE;
					strncpy(sprhead->lock_user, locker,16);
					sprhead->lock_user[16] = '\0';
				}
			}
		}
	}
	
	// environment data
	
	SetFilePointer (rif_file,0,0,FILE_BEGIN);

	List<int> edfptrs;
	list_chunks_in_file(& edfptrs, rif_file, "REBENVDT");

	Environment_Data_Chunk * ed;
	Environment_Data_Header_Chunk * edhead = 0;

	
	ed = (Environment_Data_Chunk *)lookup_single_child("REBENVDT");
	if (ed)
	{
		edhead = ed->get_header();
	}

	if (edhead)
	{
		if (edfptrs.size())
		{
			SetFilePointer (rif_file,edfptrs.first_entry(),0,FILE_BEGIN);

			// get header list
			List<int> edhl; list_chunks_in_file(& edhl, rif_file, "ENDTHEAD");

			assert (edhl.size() == 1);

			// go to lock status in header
			SetFilePointer(rif_file,edhl.first_entry() + 12,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &flags, 4, &bytes_read, 0);
			ReadFile (rif_file, (long *) locker, 16, &bytes_read, 0);

			// go to version number
			SetFilePointer(rif_file,edhl.first_entry() + 32,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &v_no, 4, &bytes_read, 0);

			if (edhead->version_no < v_no)
			{
				ed->updated_outside = TRUE;
			}
			if (flags & GENERAL_FLAG_LOCKED) 
			{
				if (!ed->local_lock) {
					ed->external_lock = TRUE;
					strncpy(edhead->lock_user, locker,16);
					edhead->lock_user[16] = '\0';
				}
				else if (strncmp(edhead->lock_user, locker,16)) {
					ed->external_lock = TRUE;
					strncpy(edhead->lock_user, locker,16);
					edhead->lock_user[16] = '\0';
				}
			}
		}
	}

	// check file for REBENUMS chunk

	SetFilePointer (rif_file,0,0,FILE_BEGIN);

	List<int> enumfptrs; list_chunks_in_file(& enumfptrs, rif_file, "REBENUMS");

	Enum_Chunk * enumch;
	Enum_Header_Chunk * enumhead = 0;

	enumch = (Enum_Chunk *)lookup_single_child("REBENUMS");
	if (enumch)
	{
		enumhead = enumch->get_header();
	}

	if (enumhead)
	{
		if (enumfptrs.size())
		{
			SetFilePointer (rif_file,enumfptrs.first_entry(),0,FILE_BEGIN);

			// get header list
			List<int> enumchhl; list_chunks_in_file(& enumchhl, rif_file, "ENUMHEAD");

			assert (enumchhl.size() == 1);

			// go to lock status in header
			SetFilePointer(rif_file,enumchhl.first_entry() + 12,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &flags, 4, &bytes_read, 0);
			ReadFile (rif_file, (long *) locker, 16, &bytes_read, 0);

			// go to version number
			SetFilePointer(rif_file,enumchhl.first_entry() + 32,0,FILE_BEGIN);
			ReadFile (rif_file, (long *) &v_no, 4, &bytes_read, 0);

			if (enumhead->version_no < v_no)
			{
				enumch->updated_outside = TRUE;
			}
			if (flags & GENERAL_FLAG_LOCKED) 
			{
				if (!enumch->local_lock) {
					enumch->external_lock = TRUE;
					strncpy(enumhead->lock_user, locker,16);
					enumhead->lock_user[16] = '\0';
				}
				else if (strncmp(enumhead->lock_user, locker,16)) {
					enumch->external_lock = TRUE;
					strncpy(enumhead->lock_user, locker,16);
					enumhead->lock_user[16] = '\0';
				}
			}
		}
	}

	CloseHandle (rif_file);

	return TRUE;
	#endif
}


BOOL File_Chunk::update_file()
{

	#if DisableLock
	if (!filename) return FALSE;

	char tempname [256];
	strcpy (tempname, filename);

	List<Shape_Chunk *> slist;
	list_shapes(&slist);
	List<Object_Chunk *> olist;
	list_objects(&olist);
	
	for (LIF<Shape_Chunk *> sli(&slist); !sli.done(); sli.next())
	{
		if (sli()->deleted)
		{
			delete sli();
		}
	}
	
	for (LIF<Object_Chunk *> oli(&olist); !oli.done(); oli.next())
	{
		if (oli()->deleted)
		{
			delete oli();
		}
	}
	
	
	return(write_file(tempname));
	
	#else

	if (!filename) return FALSE;

	twprintf("Updating %s\n",filename);

	check_file();

	HANDLE rif_file;
	unsigned long bytes_read;

	int tries = 0;

	//twprintf("Opening file\n");
	
	rif_file = CreateFileA (filename, GENERIC_WRITE + GENERIC_READ, 0, 0, OPEN_EXISTING, 
				FILE_FLAG_RANDOM_ACCESS, 0);

	if (rif_file == INVALID_HANDLE_VALUE)
	{
		DWORD error_num = GetLastError();
		switch (error_num)
		{
			case ERROR_SHARING_VIOLATION:
				twprintf("Sharing violation - retrying\n");
				break;
			case ERROR_ACCESS_DENIED:
				twprintf("File is Read Only\n");
				return FALSE;
			default:
				twprintf("Unknown error updating file, Error code %#08x\n",error_num);
				return FALSE;
		}
	}
				
	while(rif_file == INVALID_HANDLE_VALUE && tries<10)
	{
		twprintf("Again...\n");
	 	rif_file = CreateFileA (filename, GENERIC_WRITE + GENERIC_READ, 0, 0, OPEN_EXISTING, 
	 				FILE_FLAG_RANDOM_ACCESS, 0);
	 	tries ++;
		clock_t ctime = clock();
		double secs	= (double)ctime / (double)CLOCKS_PER_SEC;
		double secsgone;
		do
		{
			ctime = clock();
			secsgone = (double)ctime / (double)CLOCKS_PER_SEC;
		}
		while( (secsgone-secs)<1);
	}	

	if (rif_file == INVALID_HANDLE_VALUE) {
		error_code = CHECK_FAILED_NOT_OPEN;
		twprintf("ERROR - SHARING VIOLATION UNRESOLVED\n\n");
		return FALSE;
	}

	//twprintf("Opened\n\n");
	
	prepare_for_output();
	
	SetFilePointer (rif_file,0,0,FILE_BEGIN);

	//twprintf("Version info\n");
	
	List<int> verinf; list_chunks_in_file(& verinf, rif_file, "RIFVERIN");

// taking first entry of this list, if there are more - tough ha ha
// there shouldn't be

// Just increment it by one and reoutput
// check_file sets the internal copy of the chunk to the current file
// setting, so we need to increment that as well
	
	if (verinf.size()) {

		int f_version_num;
		SetFilePointer (rif_file,verinf.first_entry() + 12,0,FILE_BEGIN);
		ReadFile (rif_file, (long *) &f_version_num, 4, &bytes_read, 0);

		SetFilePointer (rif_file,verinf.first_entry() + 12,0,FILE_BEGIN);

		RIF_Version_Num_Chunk* rvnc=(RIF_Version_Num_Chunk*)lookup_single_child ("RIFVERIN");
		
		if (rvnc)
			rvnc->file_version_no++;
		
		f_version_num++;

		WriteFile (rif_file, (long *) &f_version_num, 4, &bytes_read, 0);

	}

//	go through the list of shape chunks looking for shape chunks to output
	//twprintf("\nShapes\n");
		
	List<Chunk *> shplst;
	lookup_child ("REBSHAPE",shplst);

	if (shplst.size())
		for (LIF<Chunk *> sli(&shplst); !sli.done(); sli.next()) {

			Shape_Chunk * tmpshpptr = ((Shape_Chunk *)sli());

			if (tmpshpptr->updated && 
				!(tmpshpptr->updated_outside || tmpshpptr->external_lock)) 
				tmpshpptr->update_chunk_in_file(rif_file);
		}


//	go through the list of object chunks looking for chunks to output
	//twprintf("\nObjects\n");
		
	List<Chunk *> oblst;
	lookup_child ("RBOBJECT",oblst);

	if (oblst.size()) 
		for (LIF<Chunk *> oli(&oblst); !oli.done(); oli.next()) {
			
			Object_Chunk * tmpobptr = ((Object_Chunk *)oli());
			
			if (tmpobptr->updated && !(tmpobptr->updated_outside || tmpobptr->external_lock)) 
				tmpobptr->update_chunk_in_file(rif_file);

		}


	//twprintf("\nSprites\n");

	List<Chunk *> sprfptrs;
	lookup_child ("RSPRITES",sprfptrs);
	AllSprites_Chunk * sprch;

	if (sprfptrs.size())
	{
		sprch = (AllSprites_Chunk *)sprfptrs.first_entry();
		if (sprch->updated && 
			!(sprch->updated_outside || sprch->external_lock)) 
			sprch->update_chunk_in_file(rif_file);
	}

	//twprintf("\nEnvironment data\n");

	Environment_Data_Chunk * ed;

	ed = (Environment_Data_Chunk *)lookup_single_child ("REBENVDT");
	if (ed)
	{
		if (ed->updated && 
			!(ed->updated_outside || ed->external_lock)) 
			ed->update_chunk_in_file(rif_file);
	}

	//twprintf("\nEnum data\n");

	List<Chunk *> enumfptrs;
	lookup_child ("REBENUMS",enumfptrs);
	Enum_Chunk * enumch;

	if (enumfptrs.size())
	{
		enumch = (Enum_Chunk *)enumfptrs.first_entry();
		if (enumch->updated && 
			!(enumch->updated_outside || enumch->external_lock)) 
			enumch->update_chunk_in_file(rif_file);
	}

	//

	int file_length = GetFileSize(rif_file,0);
	SetFilePointer (rif_file,8,0,FILE_BEGIN);

	WriteFile (rif_file, (long *) &file_length, 4, &bytes_read, 0);

	CloseHandle (rif_file);

	return TRUE;

	#endif //DisableLock
}

BOOL File_Chunk::update_chunks_from_file()
{
	#if DisableLock
	return(TRUE);
	#endif
	
	if (!filename) return FALSE;
	check_file();
	
	HANDLE rif_file;
	unsigned long bytes_read;
	
	rif_file = CreateFileA (filename, GENERIC_WRITE + GENERIC_READ, 0, 0, OPEN_EXISTING, 
					FILE_FLAG_RANDOM_ACCESS, 0);

	if (rif_file == INVALID_HANDLE_VALUE) {
		error_code = CHECK_FAILED_NOT_OPEN;
		return FALSE;
	}
	
	SetFilePointer (rif_file,0,0,FILE_BEGIN);
	
	List<int> verinf; list_chunks_in_file(& verinf, rif_file, "RIFVERIN");

	if (verinf.size()) {

		int f_version_num;
		SetFilePointer (rif_file,verinf.first_entry() + 12,0,FILE_BEGIN);
		ReadFile (rif_file, (long *) &f_version_num, 4, &bytes_read, 0);

		SetFilePointer (rif_file,verinf.first_entry() + 12,0,FILE_BEGIN);

		List<Chunk *> lverinf;
		lookup_child ("RIFVERIN",lverinf);

		if (lverinf.size())
			if (f_version_num == ((RIF_Version_Num_Chunk *)(lverinf.first_entry()))->file_version_no){
				CloseHandle (rif_file);
				return TRUE;
			}

	}

	//	go through the list of object chunks looking for chunks to input
		
	List<Chunk *> oblst;
	lookup_child ("RBOBJECT",oblst);

	if (oblst.size()) 
		for (LIF<Chunk *> oli(&oblst); !oli.done(); oli.next()) {
			
			Object_Chunk * tmpobptr = ((Object_Chunk *)oli());
			
			if (tmpobptr->updated_outside) {
			// find the chunk, then input it to a buffer and create a new object
			// from the buffer
				SetFilePointer (rif_file,0,0,FILE_BEGIN);
				
				List<int> obfptrs; list_chunks_in_file(& obfptrs, rif_file, "RBOBJECT");

				char name[50];

				LIF<int> ofpl(&obfptrs);

				if (obfptrs.size()) {
					for (; !ofpl.done(); ofpl.next()) {

						SetFilePointer (rif_file, ofpl(),0,FILE_BEGIN);
						// get header list
						List<int> obhead; list_chunks_in_file(& obhead, rif_file, "OBJHEAD1");

						assert (obhead.size() == 1);

						// get object identifier
						SetFilePointer(rif_file,obhead.first_entry() + 96,0,FILE_BEGIN);
						int i = 0; 
						do ReadFile (rif_file, (long *) (name + i), 1, &bytes_read, 0);
						while (name[i++] != 0);

						if (!strcmp(name, tmpobptr->object_data.o_name)) break;
					}
				}

				if (!ofpl.done()) {
	
					char * buffer;

					SetFilePointer (rif_file,ofpl()+8,0,FILE_BEGIN);
					int length;
					ReadFile(rif_file, (long *) &length, 4, &bytes_read, 0);
					buffer = new char [length];
					ReadFile(rif_file, (long *) buffer, length-12, &bytes_read, 0);
					new Object_Chunk (this, buffer, length-12);
					delete [] buffer;
					if (tmpobptr->get_header())
						if (tmpobptr->get_header()->associated_shape)
							tmpobptr->deassoc_with_shape(tmpobptr->get_header()->associated_shape);
					delete tmpobptr;

				}


			}

		}

//	go through the list of shape chunks looking for shape chunks to input
		
	List<Chunk *> shplst;
	lookup_child ("REBSHAPE",shplst);

	if (shplst.size())
		for (LIF<Chunk *> sli(&shplst); !sli.done(); sli.next()) {

			Shape_Chunk * tmpshpptr = ((Shape_Chunk *)sli());

			Shape_Header_Chunk * shhead = tmpshpptr->get_header();

			if (!shhead) continue;
			
			if (tmpshpptr->updated_outside) {
			// find the chunk, then input it to a buffer and create a new object
			// from the buffer
				SetFilePointer (rif_file,0,0,FILE_BEGIN);

				List<int> shfptrs; list_chunks_in_file(& shfptrs, rif_file, "REBSHAPE");

				LIF<int> sfpl(&shfptrs);

				if (shfptrs.size()) {
					for (sfpl.restart(); !sfpl.done(); sfpl.next()) {

						SetFilePointer (rif_file, sfpl(),0,FILE_BEGIN);
						// get header list
						List<int> shphead; list_chunks_in_file(& shphead, rif_file, "SHPHEAD1");

						assert (shphead.size() == 1);

						// get object identifier
						SetFilePointer(rif_file,shphead.first_entry() + 32,0,FILE_BEGIN);
						int sh_number; 
						ReadFile (rif_file, (long *) &sh_number, 4, &bytes_read, 0);

						if (sh_number == shhead->file_id_num) break;
					}
				}

				if (!sfpl.done()) {
	
					char * buffer;

					SetFilePointer (rif_file,sfpl()+8,0,FILE_BEGIN);
					int length;
					ReadFile(rif_file, (long *) &length, 4, &bytes_read, 0);
					buffer = new char [length];
					ReadFile(rif_file, (long *) buffer, length-12, &bytes_read, 0);
					new Shape_Chunk (this, buffer, length-12);
					delete [] buffer;
					// Associate with the new objects
					if ((shhead->associated_objects_store).size())
					{
						for (LIF<Object_Chunk *> aol(&(shhead->associated_objects_store)); !aol.done(); aol.next())
						{
							tmpshpptr->deassoc_with_object(aol());
						}
					}
					delete tmpshpptr;
				}
			}
		}

	post_input_processing();

	CloseHandle(rif_file);

	return TRUE;

}

void File_Chunk::list_objects(List<Object_Chunk *> * pList)
{
	Chunk * child_ptr = children;
	
	while (pList->size())
		pList->delete_first_entry();

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp ("RBOBJECT", child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				pList->add_entry((Object_Chunk *)child_ptr);
			}
			child_ptr = child_ptr->next;
		}

}

void File_Chunk::list_shapes(List<Shape_Chunk *> * pList)
{
	Chunk * child_ptr = children;

	while (pList->size())
		pList->delete_first_entry();

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp ("REBSHAPE", child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				pList->add_entry((Shape_Chunk *)child_ptr);
			}
			child_ptr = child_ptr->next;
		}

}

void File_Chunk::list_dummy_objects(List<Dummy_Object_Chunk *> * pList){
	Chunk * child_ptr = children;
	
	while (pList->size())
		pList->delete_first_entry();

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp ("DUMMYOBJ", child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				pList->add_entry((Dummy_Object_Chunk *)child_ptr);
			}
			child_ptr = child_ptr->next;
		}

}

Environment_Data_Chunk * File_Chunk::get_env_data()
{
	List<Environment_Data_Chunk *> e_list;
	Chunk * child_ptr = children;

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp ("REBENVDT", child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				e_list.add_entry((Environment_Data_Chunk *)child_ptr);
			}
			child_ptr = child_ptr->next;
		}

	// There can be only ONE.
	assert (e_list.size() < 2);
	
	if (e_list.size())
		return e_list.first_entry();
	return 0;
}

void File_Chunk::build_object_array()
{
	List<Object_Chunk*> oblist;
	list_objects(&oblist);

	if(object_array)
	{
		free(object_array);
		object_array=0;
	}	
	object_array_size=0;

	LIF<Object_Chunk*> oblif(&oblist);

	//find the highest object index
	for(oblif.restart();!oblif.done();oblif.next())
	{
		object_array_size=max(object_array_size,oblif()->object_data.index_num+1);
	}

	if(object_array_size<=0) return;

	object_array = (Object_Chunk**) malloc(sizeof(Object_Chunk*)*object_array_size);
	for(int i=0;i<object_array_size;i++)
	{
		object_array[i]=0;
	}

	//now fill in object array

	for(oblif.restart();!oblif.done();oblif.next())
	{
		int index=oblif()->object_data.index_num;
		if(index>=0)
		{
			object_array[index]=oblif();
		}
	}
}

Object_Chunk* File_Chunk::get_object_by_index(int index)
{
	if(!object_array) build_object_array();
	if(index<0 || index>=object_array_size)return 0;
	return object_array[index];
}

void File_Chunk::assign_index_to_object(Object_Chunk* object)
{
	assert(object);

	if(!object_array) build_object_array();
	//see if there is a free index

	for(int i=0;i<object_array_size;i++)
	{
		if(!object_array[i])
		{
			object->object_data_store->index_num=i;
			object_array[i]=object;
			return;
		}
	}
	
	//add a new entry on the end of the array
	object_array_size++;
		
	object_array=(Object_Chunk**) realloc(object_array,sizeof(Object_Chunk*)*object_array_size);
	

	object->object_data_store->index_num=object_array_size-1;;
	object_array[object_array_size-1]=object;
}

/////////////////////////////////////////

// Class GodFather_Chunk functions

/*
Children for GodFather_Chunk :
"REBSHAPE"		Shape_Chunk
"RSPRITES"		AllSprites_Chunk
"RBOBJECT"		Object_Chunk
"RIFVERIN"		RIF_Version_Num_Chunk
"REBENVDT"		Environment_Data_Chunk
"REBENUMS"		Enum_Chunk
"OBJCHIER"		Object_Hierarchy_Chunk
"OBHALTSH"		Object_Hierarchy_Alternate_Shape_Set_Chunk

*/

GodFather_Chunk::GodFather_Chunk(char * buffer, size_t size)
: Chunk_With_Children (NULL, "REBINFF2")
{
	Parent_File = this;

	char * buffer_ptr = buffer;

	// The start of the first chunk

	while ((buffer_ptr-buffer)< ((signed)size-12) && !error_code) {

		if ((*(int *)(buffer_ptr + 8)) + (buffer_ptr-buffer) > ((signed)size-12)) {
			error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		DynCreate(buffer_ptr);
		buffer_ptr += *(int *)(buffer_ptr + 8);

	}

}

/////////////////////////////////////////

// Class RIF_Version_Num_Chunk functions

RIF_IMPLEMENT_DYNCREATE("RIFVERIN",RIF_Version_Num_Chunk)

void RIF_Version_Num_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = file_version_no;

}


/////////////////////////////////////////

// Class RIF_Name_Chunk functions
RIF_IMPLEMENT_DYNCREATE("RIFFNAME",RIF_Name_Chunk)

RIF_Name_Chunk::RIF_Name_Chunk (Chunk_With_Children * parent, const char * rname)
: Chunk (parent, "RIFFNAME")
{
	rif_name = new char [strlen(rname)+1];
	strcpy (rif_name, rname);
}

RIF_Name_Chunk::RIF_Name_Chunk (Chunk_With_Children * parent, const char * rdata, size_t /*rsize*/)
: Chunk (parent, "RIFFNAME")
{
	rif_name = new char [strlen(rdata)+1];
	strcpy (rif_name, rdata);
}

RIF_Name_Chunk::~RIF_Name_Chunk ()
{
	if (rif_name)
		delete [] rif_name;
}


void RIF_Name_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	strcpy (data_start, rif_name);

}


///////////////////////////////////////

/*
Children for RIF_File_Chunk :
"REBSHAPE"		Shape_Chunk
"RSPRITES"		AllSprites_Chunk
"RBOBJECT"		Object_Chunk
"RIFVERIN"		RIF_Version_Num_Chunk
"REBENVDT"		Environment_Data_Chunk
"OBJCHIER"		Object_Hierarchy_Chunk
"OBHALTSH"		Object_Hierarchy_Alternate_Shape_Set_Chunk

*/


RIF_File_Chunk::RIF_File_Chunk (Chunk_With_Children * parent, const char * file_name)
: Chunk_With_Children (parent, "SUBRIFFL")
{
	char rifIsCompressed = FALSE;
	char *uncompressedData = NULL;
	FILE *rif_file;
	DWORD file_size;
	DWORD file_size_from_file;
	char * buffer;
	char * buffer_ptr;
	char id_buffer[9];


	Chunk * ParentFileStore = Parent_File;
	
	Parent_File = this;
	
	error_code = 0;
	
	rif_file = OpenGameFile(file_name, FILEMODE_READONLY, FILETYPE_PERM);

	if (rif_file == NULL) {
		error_code = CHUNK_FAILED_ON_LOAD;
		Parent_File = ParentFileStore;
		return;
	}

	fseek(rif_file, 0, SEEK_END);
	file_size = ftell(rif_file);
	rewind(rif_file);

	if (fread(id_buffer, 1, 8, rif_file) != 8) {
		error_code = CHUNK_FAILED_ON_LOAD;
		fclose(rif_file);
		Parent_File = ParentFileStore;
		return;
	}	

	//check for compressed rif
	if (!strncmp (id_buffer, COMPRESSED_RIF_IDENTIFIER, 8))
	{
		rifIsCompressed = TRUE;
	}	
	else if (strncmp (id_buffer, "REBINFF2", 8)) {
		error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
		fclose(rif_file);
		Parent_File = ParentFileStore;
		return;
	}	

	buffer = new char [file_size];
	
	/* KJL 17:57:44 19/09/98 - if the rif is compressed, we must load the whole
	file in and then pass it to the decompression routine, which will return a
	pointer to the original data. */
	if (rifIsCompressed)
	{
		if (fread(buffer+8, 1, (file_size-8), rif_file) != (file_size-8)) {
			error_code = CHUNK_FAILED_ON_LOAD;
			fclose(rif_file);
			Parent_File = ParentFileStore;
			delete [] buffer;
			return;
		}	
		uncompressedData = HuffmanDecompress((HuffmanPackage*)buffer); 		
		file_size = ((HuffmanPackage*)buffer)->UncompressedDataSize;
		
		delete [] buffer; // kill the buffer the compressed file was loaded into

		buffer_ptr = buffer = uncompressedData+12; // skip header data
	}
	else // the normal uncompressed approach:
	{
		//get the file size stored in the rif file
		if (fread(&file_size_from_file, 1, 4, rif_file) != 4) {
			error_code = CHUNK_FAILED_ON_LOAD;
			fclose(rif_file);
			Parent_File = ParentFileStore;
			delete [] buffer;
			return;
		}	

		//and compare with the actual file size
		if (file_size != file_size_from_file) {
			error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			fclose(rif_file);
			Parent_File = ParentFileStore;
			delete [] buffer;
			return;
		}	

		//read the rest of the file into the buffer
		if (fread(buffer, 1, (file_size-12), rif_file) != (file_size-12)) {
			error_code = CHUNK_FAILED_ON_LOAD;
			fclose(rif_file);
			Parent_File = ParentFileStore;
			delete [] buffer;
			return;
		}
		buffer_ptr = buffer;
	}
	
	fclose(rif_file);
	

	// Process the RIF

	// The start of the first chunk

	while ((buffer_ptr-buffer)< ((signed) file_size-12) && !error_code) {

		if ((*(int *)(buffer_ptr + 8)) + (buffer_ptr-buffer) > ((signed)file_size-12)) {
			error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		DynCreate(buffer_ptr);
		buffer_ptr += *(int *)(buffer_ptr + 8);
	}

	/* KJL 17:59:42 19/09/98 - release the memory holding the rif */
	if (rifIsCompressed)
	{
		free(uncompressedData);
	}
	else
	{
		delete [] buffer;
	}

	post_input_processing();

	Parent_File = ParentFileStore;
}

void RIF_File_Chunk::post_input_processing()
{
	List<Shape_Chunk *> shplist;
	List<Object_Chunk *> objlist;

	List<Chunk *> child_lists;

	lookup_child("REBSHAPE",child_lists);

	while (child_lists.size()) {
		shplist.add_entry((Shape_Chunk *)child_lists.first_entry());
		child_lists.delete_first_entry();
	}

	lookup_child("RBOBJECT",child_lists);


	while (child_lists.size()) {
		objlist.add_entry((Object_Chunk *)child_lists.first_entry());
		child_lists.delete_first_entry();
	}

	for (LIF<Object_Chunk *> ol(&objlist); !ol.done(); ol.next()) {
		
		if (ol()->get_header()) {
			
			for (LIF<Shape_Chunk *> sl(&shplist); 
					 !sl.done(); sl.next()) {
				if (sl()->get_header())
					if (sl()->get_header()->file_id_num == ol()->get_header()->shape_id_no){
						ol()->assoc_with_shape(sl());
						break;
					}
			}
		}

	}	

	for (LIF<Shape_Chunk *> sli(&shplist); !sli.done(); sli.next())
	{
		Shape_Chunk::max_id = max (Shape_Chunk::max_id,sli()->get_header()->file_id_num);	
	}

	Chunk_With_Children::post_input_processing();	
}


void RIF_File_Chunk::list_objects(List<Object_Chunk *> * pList)
{
	Chunk * child_ptr = children;
	
	while (pList->size())
		pList->delete_first_entry();

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp ("RBOBJECT", child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				pList->add_entry((Object_Chunk *)child_ptr);
			}
			child_ptr = child_ptr->next;
		}
}

void RIF_File_Chunk::list_shapes(List<Shape_Chunk *> * pList)
{
	Chunk * child_ptr = children;

	while (pList->size())
		pList->delete_first_entry();

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp ("REBSHAPE", child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				pList->add_entry((Shape_Chunk *)child_ptr);
			}
			child_ptr = child_ptr->next;
		}

}

Environment_Data_Chunk * RIF_File_Chunk::get_env_data()
{
	List<Environment_Data_Chunk *> e_list;
	Chunk * child_ptr = children;

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp ("REBENVDT", child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				e_list.add_entry((Environment_Data_Chunk *)child_ptr);
			}
			child_ptr = child_ptr->next;
		}

	// There can be only ONE.
	assert (e_list.size() < 2);
	
	if (e_list.size())
		return e_list.first_entry();
	return 0;
}
