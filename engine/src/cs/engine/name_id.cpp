// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/name_id.hpp"
#include "cs/containers/hash_table.hpp"

#include <cstring>
#include <cassert>

uint32_t crc32(const char *string, size_t length) 
{
    if(length == (size_t) -1)
    {
        length = strlen(string);    
    } 

	uint32_t crc = 0xFFFFFFFF;
	
	for(uint32 i = 0; i < length; i++)
    {
		char ch = string[i];
		for (int32 j = 0; j < 8; j++)
        {
			uint32_t b = (ch ^ crc) & 1;
			crc >>= 1;
			if (b)
            {            
                crc=crc^0xEDB88320;
            }

			ch >>= 1;
		}
	}
	
	return ~crc;
}

static Hash_Table<const char*> *string_hash_table;

uint32 get_hash(const char* str, size_t _size)
{
    return crc32(str, _size);
}

const char* get_hashed_string(uint32 hash)
{
    return (*string_hash_table)[hash];
}

uint32 operator ""_hashed(const char* str, size_t size)
{
    if (string_hash_table == nullptr)
    {
         string_hash_table = new Hash_Table<const char*>(2048);
    }

    const uint32 hash = get_hash(str, size);
    (*string_hash_table)[hash] = strdup(str);

    return hash;
}

Name_Id Name_Id::Empty = Name_Id("");

Name_Id::Name_Id()
    :id(0), str(nullptr)
{
}

Name_Id::Name_Id(const char* string)
    :id(get_hash(string)), str(strdup(string))
{
}

/*
template<>
Archive& operator<<(Archive& archive, Name_Id& data)
{
    archive << data.id; //data.str;

    int32 str_len;
    if (archive.is_writing())
    {
        str_len = (int32) strlen(data.str) + 1;
        archive << str_len;
        archive.write((void*) data.str, sizeof(char), (size_t) str_len);
    }
    else
    {
        archive << str_len;
        char* buf = new char[str_len];
        archive.read((void*) buf, sizeof(char), (size_t) str_len);
        
        Name_Id read_name(buf);
        CS_ASSERT(read_name.id == data.id);
        data = read_name;
    }

    return archive;
}
*/