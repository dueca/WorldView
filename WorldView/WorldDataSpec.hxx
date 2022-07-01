/* ------------------------------------------------------------------   */
/*      item            : WorldDataSpec.hxx
        made by         : Rene van Paassen
        date            : 100610
	category        : header file 
        description     : 
	changes         : 100106 first version
	                  100610 split off from Specifications.hxx
        language        : C++
*/

#include "../comm-objects/WorldDataSpec.hxx"

#if 0

#ifndef WorldDataSpec_hxx
#define WorldDataSpec_hxx



#include <vector>
#include <string>
#include <AmorphStore.hxx>
#include <dueca_ns.h>
#include <iostream>

USING_DUECA_NS;

/** Defines an object/element in the world, not controlled by the
    simulation. Used in Plib interface. */
struct WorldDataSpec {
  
  /** For compatibility with newer channel code */
  static const char* classname;
	public:
  /** Name for the object */
  std::string name;

  /** Type, determines behavior of the object */
  std::string type;
  
  /** File names, for possibly a number of files to read the model,
      or any other specification caught in character strings */
  std::vector<std::string> filename;

  /** Position, x, y, z coordinates and/or Orientation, phi, theta,
      psi Euler angles, or anything else that the application might fancy */
  std::vector<double> coordinates;

  /** Constructor */
  WorldDataSpec();
  
  /** Constructor from data stream */
  WorldDataSpec(DUECA_NS ::AmorphReStore& r);

  /** new operator "new", which places objects not on a
  heap, but in one of the memory arenas. This may prevent
  problems with asymmetric allocation */
  static void* operator new(size_t size);

  	/** new operator "delete", to go with the new version
  of operator new. */
  static void operator delete(void* p);

  /** placement "new", needed for stl. */
  inline static void* operator new(size_t size, WorldDataSpec  *& o)
  { return reinterpret_cast<void*>(o); }

  /** placement "new", needed for other versions of the stl.*/
  inline static void* operator new(size_t size, void*& o)
  { return o; }
  
  /** Pack into a data stream */
  void packData(DUECA_NS ::AmorphStore& s) const;

  /** unpacks the WorldViewerEvent from an amorphous storage. */
  void unPackData(DUECA_NS ::AmorphReStore& s);

  /** prints the WorldDataSpec to a stream. */
  ostream & print(ostream& s) const;
  
  /** Check that this is sufficiently specified */
  bool sufficient();

  /** Test for equality. */
  bool operator == (const WorldDataSpec& o) const;
	
  /** For packability */
  bool operator != ( const WorldDataSpec& o ) const;

  WorldDataSpec& operator = (const WorldDataSpec& o);
};

/** pack the object into amorphous storage. */
inline void packData( DUECA_NS ::AmorphStore& s, const WorldDataSpec& o)
{ 
  o.packData(s); 
}

inline void unPackData(DUECA_NS::AmorphReStore& s, WorldDataSpec&o)
{ o.unPackData(s); }

PRINT_NS_START;

/** print to a stream. */
inline ostream & operator << (ostream& s, const WorldDataSpec& o)
{ 
  return s; //.print(s); 
}

PRINT_NS_END;

#endif
#endif
