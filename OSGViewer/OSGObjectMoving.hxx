/* ------------------------------------------------------------------   */
/*      item            : OSGObjectMoving.hxx
        made by         : Rene van Paassen
        date            : 180903
	category        : header file 
        description     : 
	changes         : 180903 first version
        language        : C++
        copyright       : (c) 2018 TUDelft-AE-C&S
*/

#ifndef OSGObjectMoving_hxx
#define OSGObjectMoving_hxx

#include "OSGObject.hxx"
#include <boost/scoped_ptr.hpp>
#include <dueca/dueca.h>

/** OSG objects controlled with the "BaseObjectMotion" dco object. 
 */   
class OSGObjectMoving: public OSGObject
{
protected:
  /** Channel read token for motion input */
  boost::scoped_ptr<ChannelReadToken>     r_motion;
  
public:

  /** Constructor 

      @param spec   Object data, defines the name of the object and
                    the image file
  */  
  OSGObjectMoving(const WorldDataSpec& spec);

  /** Destructor */
  virtual ~OSGObjectMoving();

  /** Connect to a channel entry 
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion& base, double late);
};

#endif
