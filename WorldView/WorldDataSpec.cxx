/* ------------------------------------------------------------------   */
/*      item            : WorldDataSpec.cxx
        made by         : Rene van Paassen
        date            : 100610
	category        : header file
        description     :
	changes         : 100610 first version
        language        : C++
*/

#if 0

#include "WorldDataSpec.hxx"
#include <iostream>
#include "Arena.hxx"
#include "ArenaPool.hxx"
#include <cassert>

const char* WorldDataSpec::classname = "WorldDataSpec";

WorldDataSpec::WorldDataSpec() :
  name(""),
  type("General")
{
  filename.empty();
  coordinates.empty();
}

WorldDataSpec::WorldDataSpec ( DUECA_NS ::AmorphReStore& r ) :
  name ( r ),
  type ( r )
{
  uint16_t n_files ( r );
  filename.resize ( n_files );
  for ( unsigned int ii = n_files; ii--; ) 
    ::unPackData ( r, filename[ii] );
	
  uint16_t n_coords ( r );
  coordinates.resize ( n_coords );
  for ( unsigned int ii = n_coords; ii--; ) 
    ::unPackData ( r, coordinates[ii] );
}

bool WorldDataSpec::operator == ( const WorldDataSpec& o ) const
{
  if ( this->name != o.name ) return false;
  if ( this->type != o.type ) return false;
  if ( this->filename != o.filename ) return false;
  if ( this->coordinates != o.coordinates ) return false;
  return true;
}

WorldDataSpec& WorldDataSpec::operator= ( const WorldDataSpec & other )
{
  if ( this == &other ) return *this;
  this->name = other.name;
  this->type = other.type;
  this->filename = other.filename;
  this->coordinates = other.coordinates;
  return *this;
}

bool WorldDataSpec::operator != ( const WorldDataSpec& o ) const
{
  return name != o.name ||
    type != o.type ||
    filename != o.filename ||
    coordinates != o.coordinates;
}

void* WorldDataSpec::operator new ( size_t size )
{
  assert ( size == sizeof ( WorldDataSpec ) );
  static Arena* my_arena = arena_pool.findArena
    ( sizeof ( WorldDataSpec ) );
  return my_arena->alloc ( size );
}

void WorldDataSpec::operator delete ( void* v )
{
  static Arena* my_arena = arena_pool.findArena
    ( sizeof ( WorldDataSpec ) );
  my_arena->free ( v );
}


void WorldDataSpec::packData ( DUECA_NS ::AmorphStore& s ) const
{
  ::packData ( s, name );
  ::packData ( s, type );
  assert(filename.size() < 0x10000);
  uint16_t n_files = filename.size();
  ::packData ( s, n_files );
  for ( unsigned int ii = n_files; ii--; ) ::packData ( s, filename[ii] );
  
  assert(coordinates.size() < 0x10000);
  uint16_t n_coords = coordinates.size();
  ::packData (s, n_coords );
  for ( unsigned int ii = n_coords; ii--; ) ::packData ( s, coordinates[ii] );
}

void WorldDataSpec::unPackData ( DUECA_NS ::AmorphReStore& s )
{
  ::unPackData ( s, name );
  ::unPackData ( s, type );
  uint16_t n_files(s);
  filename.resize(n_files );
  for ( unsigned int ii = n_files; ii--; ) ::unPackData ( s, filename[ii] );
  
  uint16_t n_coords(s);
  coordinates.resize(n_coords );
  for ( unsigned int ii = n_coords; ii--; ) ::unPackData ( s, coordinates[ii] );
}

bool WorldDataSpec::sufficient()
{
  return name.size() != 0 && type.size() != 0;
}
#endif
