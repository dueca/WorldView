/* ------------------------------------------------------------------   */
/*      item            : Specifications.hxx
        made by         : Rene van Paassen
        date            : 100126
	category        : header file
        description     :
	changes         : 100126 first version
        language        : C++
*/

#ifndef Specifications_hxx
#define Specifications_hxx

#include "../comm-objects/ViewSpec.hxx"
#include "../comm-objects/WinSpec.hxx"

#if 0

#include <vector>
#include <string>
#include <AmorphStore.hxx>
#include <dueca_ns.h>
#include <iostream>

USING_DUECA_NS;

/** Helper struct that defines the parameters for a view. */
struct ViewSpec
{
  static const char* classname;

	/** Name for this specification */
	std::string name;

	/** Name for the window it should be in. Empty for default */
	std::string winname;

	/** Optionally, specify (by OGRE name) a display overlay with mask */
	std::string overlay;

	/** Coordinates for the viewport. If empty, the complete window is assumed
	    left, top, width, height*/
	std::vector<int> portcoords;

	/** Eye position and optionally rotation, offset from a base (cg or
	    otherwise) position */
	std::vector<float> eye_pos;

	/** Frustrum data, containing, in this order
	    near plane distance
	    far plane distance
	    and then, option 1: field of view y
	    option 2: left, right, bottom, top, of near plane in eye coord */
	std::vector<float> frustum_data;

	/** Flag to turn rendering of shadows in the window on (TRUE) or off
	    (FALSE) Tested for OGRE only so far and only
	    SHADOWTYPE_STENCIL_MODULATIVE technique.  Make sure that in your
	    scene the groundplane and large structures you don't want to
	    cast shadows you specify the castShadows parameter of an entity
	    is set to "false".<br> Added by Mark Mulder 17/08/2010 */
	bool render_shadows;

	std::string shadow_technique;

	std::vector<float> shadow_colour;

	float shadow_farDistance;

	/** Constructor */
	ViewSpec();

	/** Constructor from data stream */
	ViewSpec ( DUECA_NS ::AmorphReStore& r );

	  /** new operator "new", which places objects not on a
	heap, but in one of the memory arenas. This may prevent
	problems with asymmetric allocation */
	static void* operator new(size_t size);

  	/** new operator "delete", to go with the new version
	of operator new. */
	static void operator delete(void* p);

	/** placement "new", needed for stl. */
	inline static void* operator new(size_t size, ViewSpec  *& o)
	{ return reinterpret_cast<void*>(o); }

	/** placement "new", needed for other versions of the stl.*/
	inline static void* operator new(size_t size, void*& o)
	{ return o; }
	
	/** Pack into a data stream */
	void packData ( DUECA_NS ::AmorphStore& s ) const;

	/** unpacks the WorldViewerEvent from an amorphous storage. */
	void unPackData ( DUECA_NS ::AmorphReStore& s );

	/** prints the ViewSpec to a stream. */
	ostream & print(ostream& s) const;
	
	/** Test for equality. */
	bool operator == (const ViewSpec& o) const;
	
	/** For packability */
	bool operator != ( const ViewSpec& o ) const;

	ViewSpec& operator = (const ViewSpec& o);
	
	/** Check that this is sufficiently specified */
	bool sufficient();
};


/** Helper struct that defines the parameters for a window */
struct WinSpec
{
  static const char* classname;

	/** Name for the window */
	std::string name;

	/** Window size, or window size and position */
	std::vector<int> size_and_position;

	/** Display name */
	std::string display;

	/** Event map */
	unsigned long mask;

	/** Constructor */
	WinSpec() : display ( "" ), mask ( 0 ) { }

	/** Check that this is sufficiently specified */
	bool sufficient()
	{
		return name.size() != 0 &&
		       ( size_and_position.size() == 2 || size_and_position.size() == 4 );
	}

};

/** pack the object into amorphous storage. */
inline void packData( DUECA_NS ::AmorphStore& s, const ViewSpec& o)
{ 
	o.packData(s); 
}

inline void unPackData(DUECA_NS::AmorphReStore& s, ViewSpec& o)
{ o.unPackData(s); }

PRINT_NS_START;

/** print to a stream. */
inline ostream & operator << (ostream& s, const ViewSpec& o)
{ 
	return s; //.print(s); 
}

PRINT_NS_END;
#endif

#endif
