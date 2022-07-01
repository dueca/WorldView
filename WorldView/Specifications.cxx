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
#include "Specifications.hxx"
#include <iostream>
#include "Arena.hxx"
#include "ArenaPool.hxx"
#include <cassert>

const char* ViewSpec::classname = "ViewSpec";

ViewSpec::ViewSpec() :
  name ( "" ),
  winname ( "" ),
  overlay ( "" ),
  portcoords(),
  eye_pos ( 6, 0.0f ),
  frustum_data ( 3 ),
  render_shadows ( false ),
  shadow_technique ( "stencil" ),
  shadow_colour ( 3 ),
  shadow_farDistance ( 100.0 )
{
  frustum_data[0] = 1.0;
  frustum_data[1] = 1000.0, frustum_data[2] = 0.8;
  shadow_colour[0] = 0.2;
  shadow_colour[1] = 0.2;
  shadow_colour[2] = 0.2;
}

ViewSpec::ViewSpec ( DUECA_NS ::AmorphReStore& r ) :
  name ( r ),
  winname ( r ),
  overlay ( r ),
  render_shadows ( r ),
  shadow_technique ( r ),
  shadow_farDistance ( r )
{
  uint8_t n_portcoords ( r );
  portcoords.resize ( n_portcoords );
  for ( unsigned int ii = n_portcoords; ii--; ) 
    ::unPackData ( r, portcoords[ii] );

  uint8_t n_eye_pos ( r );
  eye_pos.resize ( n_eye_pos );
  for ( unsigned int ii = n_eye_pos; ii--; )
    ::unPackData ( r, eye_pos[ii] );
	
  uint8_t n_frustum_data ( r );
  frustum_data.resize ( n_frustum_data );
  for ( unsigned int ii = n_frustum_data; ii--; ) 
    ::unPackData ( r, frustum_data[ii] );
	
  uint8_t n_shadow_colour ( r );
  shadow_colour.resize ( n_shadow_colour );
  for ( unsigned int ii = n_shadow_colour; ii--; ) 
    ::unPackData ( r, shadow_colour[ii] );
}

bool ViewSpec::operator == ( const ViewSpec& o ) const
{
  if ( this->name != o.name ) return false;
  if ( this->winname != o.winname ) return false;
  if ( this->overlay != o.overlay ) return false;
  if ( this->render_shadows != o.render_shadows ) return false;
  if ( this->shadow_technique != o.shadow_technique ) return false;
  if ( this->shadow_farDistance != o.shadow_farDistance ) return false;
  if ( this->portcoords != o.portcoords ) return false;
  if ( this->eye_pos != o.eye_pos ) return false;
  if ( this->frustum_data != o.frustum_data ) return false;
  if ( this->shadow_colour != o.shadow_colour ) return false;
  return true;
}

ViewSpec& ViewSpec::operator= ( const ViewSpec & other )
{
  if ( this == &other ) return *this;
  this->name = other.name;
  this->winname = other.winname;
  this->overlay = other.overlay;
  this->render_shadows = other.render_shadows;
  this->shadow_technique = other.shadow_technique;
  this->shadow_farDistance = other.shadow_farDistance;
  this->portcoords = other.portcoords;
  this->eye_pos = other.eye_pos;
  this->frustum_data = other.frustum_data;
  this->shadow_colour = other.shadow_colour;
  return *this;
}

bool ViewSpec::operator != ( const ViewSpec& o ) const
{
  return name != o.name ||
    winname != o.winname ||
    overlay != o.overlay ||
    portcoords != o.portcoords ||
    eye_pos != o.eye_pos ||
    frustum_data != o.frustum_data ||
    render_shadows != o.render_shadows ||
    shadow_technique != o.shadow_technique ||
    shadow_colour != o.shadow_colour ||
    shadow_farDistance != o.shadow_farDistance;
}

void* ViewSpec::operator new ( size_t size )
{
  assert ( size == sizeof ( ViewSpec ) );
  static Arena* my_arena = arena_pool.findArena
    ( sizeof ( ViewSpec ) );
  return my_arena->alloc ( size );
}

void ViewSpec::operator delete ( void* v )
{
  static Arena* my_arena = arena_pool.findArena
    ( sizeof ( ViewSpec ) );
  my_arena->free ( v );
}

void ViewSpec::packData ( DUECA_NS ::AmorphStore& s ) const
{
  ::packData ( s, name );
  ::packData ( s, winname );
  ::packData ( s, overlay );
  ::packData ( s, render_shadows );
  ::packData ( s, shadow_technique );
  ::packData ( s, shadow_farDistance );

  assert(portcoords.size() < 256);
  uint8_t n_coords = portcoords.size();
  ::packData ( s, n_coords );
  for ( unsigned int ii = n_coords; ii--; ) 
    ::packData ( s, portcoords[ii] );

  assert(eye_pos.size() < 256);
  uint8_t n_eye_pos = eye_pos.size();
  ::packData ( s, n_eye_pos );
  for ( unsigned int ii = n_eye_pos; ii--; ) 
    ::packData ( s, eye_pos[ii] );

  assert(frustum_data.size() < 256);
  uint8_t n_frustum_data = frustum_data.size();
  ::packData ( s, n_frustum_data );
  for ( unsigned int ii = n_frustum_data; ii--; ) 
    ::packData ( s, frustum_data[ii] );

  assert(shadow_colour.size() < 256);
  uint8_t n_shadow_colour = shadow_colour.size();
  ::packData ( s, n_shadow_colour );
  for ( unsigned int ii = n_shadow_colour; ii--; ) 
    ::packData ( s, shadow_colour[ii] );
}

void ViewSpec::unPackData ( DUECA_NS ::AmorphReStore& s )
{
  ::unPackData ( s, name );
  ::unPackData ( s, winname );
  ::unPackData ( s, overlay );
  ::unPackData ( s, render_shadows );
  ::unPackData ( s, shadow_technique );
  ::unPackData ( s, shadow_farDistance );

  uint8_t n_coords(s);
  portcoords.resize(n_coords);
  for ( unsigned int ii = n_coords; ii--; ) 
    ::unPackData ( s, portcoords[ii] );

  uint8_t n_eye_pos(s);
  eye_pos.resize(n_eye_pos);
  for ( unsigned int ii = n_eye_pos; ii--; ) 
    ::unPackData ( s, eye_pos[ii] );

  uint8_t n_frustum_data(s);
  frustum_data.resize(n_frustum_data);
  for ( unsigned int ii = n_frustum_data; ii--; ) 
    ::unPackData ( s, frustum_data[ii] );

  uint8_t n_shadow_colour(s);
  shadow_colour.resize(n_shadow_colour );
  for ( unsigned int ii = n_shadow_colour; ii--; ) 
    ::unPackData ( s, shadow_colour[ii] );
}

bool ViewSpec::sufficient()
{
  return ( name.size() != 0 ) &&
    ( portcoords.size() == 0 || portcoords.size() == 4 ) &&
    ( frustum_data.size() == 3 || frustum_data.size() == 6 ) &&
    ( shadow_colour.size() == 3 );
}

#endif
