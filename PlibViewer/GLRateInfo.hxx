/* ------------------------------------------------------------------   */
/*      item            : GLRateInfo.hxx
        made by         : Rene van Paassen
        date            : 100209
	category        : header file 
        description     : 
	changes         : 100209 first version
        language        : C++
*/

#ifndef GLRateInfo_hxx
#define GLRateInfo_hxx

#include "PlibObject.hxx"
#include "comm-objects.h"
#include <sys/time.h>
#include <list>

/** This is an example of an object collaborating with the PlibViewer
    and drawing its own GL code. It checks the time at which it is
    called, and every second updates the largest time span it found,
    and the update rate of the display. 

    This object should be useful by itself for testing, but you can
    also use it as an example of how to extend the factory of graphics
    objects used by PlibViewer. Look in the .cxx file for that. 
*/
class GLRateInfo: public PlibObject
{
  /** previous time */
  struct timeval previous;
  
  /** Draw rate previous second */
  unsigned int rate;

  /** Largest gap in usecs */
  unsigned int gap;

  /** Number of draw calls */
  unsigned int currentrate;

  /** Current second largest gap */
  int currentgap;

  /** List of windows for which to draw */
  std::list<int> drawviews;

public:
  /** Constructor 
      \param  spec   Most of this is ignored. The list of coordinates
      is remembered, if a coordinate is equal to a view number
      (starting from 0, in order of creation), the info is used and
      drawn. */
  GLRateInfo(const WorldDataSpec& spec);

  /** Destructor */
  ~GLRateInfo();

  /** Draw the timing information */
  void draw(int viewno);

  /** Init with plib root is not necessary */
  void init(ssgRoot* scene) { }

  /** Returns true if the object needs drawing post-access */
  bool requirePostDrawAccess() { return true; }
};

#endif
