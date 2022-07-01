/* ------------------------------------------------------------------   */
/*      item            : GLOverlay.hxx
        made by         : Rene van Paassen
        date            : 100210
	category        : header file 
        description     : 
	changes         : 100210 first version
        language        : C++
*/

#ifndef GLOverlay_hxx
#define GLOverlay_hxx

#include "PlibObject.hxx"
#include "comm-objects.h"
#include <GL/gl.h>
#include <vector>

/** Create a simple square overlay on the screen, with possibly a
    texture in it. */
class GLOverlay: public PlibObject
{
  std::string     filename;
  
  /** Pointer to the texture */
  GLuint           texture;

  /** Vertex points, x, y pairs */
  std::vector<double>  vpoints;

  /** View in which this overlay is drawn */
  int             viewno;

public:
  /** Constructor */
  GLOverlay(const WorldDataSpec& spec);

  /** Destructor */
  ~GLOverlay();

  /** Draw the timing information */
  void draw(int viewno);

  /** Init with plib root is not necessary */
  void init(ssgRoot* scene);

  /** Returns true if the object needs drawing post-access */
  bool requirePostDrawAccess() { return true; }

  /** Connect to a channel entry 
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc.
   */
  virtual void iterate(TimeTickType ts,
                       const BaseObjectMotion& base, double late);
  
};

#endif
