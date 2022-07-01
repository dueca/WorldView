/* ------------------------------------------------------------------   */
/*      item            : WorldView.hxx
        made by         : rvanpaassen
	from template   : DuecaModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Fri May 15 15:33:50 2009
	category        : header file 
        description     : 
	changes         : Fri May 15 15:33:50 2009 first version
	template changes: 030401 RvP Added template creation comment
        language        : C++
*/

#ifndef WorldView_hxx
#define WorldView_hxx

// include the dusime header
#include <dueca.h>
#include <AperiodicAlarm.hxx>
#include <dueca/ChannelWatcher.hxx>
USING_DUECA_NS;

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"

// include headers for functions/classes you need in the module
#include "WorldViewerBase.hxx"
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>

/** Out of the window view module
    
    This module can create a scene with a scene toolkit, currently
    Ogre, plib, OpenSceneGraph or FlightGear. 

    The module can generate multiple camera views on this same 
    scene. It is intended to be compatible with the HMI lab and thus
    provides fully configurable view specifications for the cameras. 

    \verbinclude world-view.scm

    The basic mode of operation is that one channel determines the view
    position and orientation on the world (by default the 
    ObjectMotion://<entity> channel), and one or more channels determine
    any moving objects or other information (hud display, e.g.).

    This module does the same whether it is in HoldCurrent or Advance,
    it simply calls for a re-draw of the scene. That is why it is a
    DUECA module, and not a DUSIME module. 

    Optionally, you can configure a channel for sending keyboard and
    cursor events from the window(s) opened. This currently only works
    with the PlibViewer.
 */
class WorldView: public Module
{
  /** self-define the module type, to ease writing the parameter table */
  typedef WorldView _ThisModule_;

private: // simulation data
  
  /** Object that provides the visualisation */
  WorldViewerBase    *rendersurface;

  /** Initial position and orientation */
  BaseObjectMotion current_view;

  /** Occupy a complete thread. If selected the viewer does not run on
      the simulation clock, but on the graphics card, waiting on swap
      buffers. */
  bool claim_thread;

  /** restore context */
  bool restore_context;
  
  /** Flag for initialisation from within the claimed thread. */
  bool do_init;

  /** Flag to maybe keep running */
  TimeTickType run_until;

  /** Maximum prediction time */
  double max_predict;

  /** Prediction correction */
  double t_predict;

  /** Number of configuration commands to accept per cycle. */
  int num_config_per_round;

private: // dueca configuration calls

  /** Function call that adds an object to the scene. */
  bool addObject(ScriptCreatable& ava, bool in);
  
  /** Function call that sets the viewer object. */
  bool setViewer(ScriptCreatable& ava, bool in);
  
  /** Set the initial camera position */
  bool initialCamera(const vector<float>& i);

private: // channel access
  /** Channel with position and rates. Position as quaternion */
  boost::scoped_ptr<ChannelReadToken>     r_own;

  /** Channel watcher list, monitors one or more channels for 
      compatible entries. */
  typedef std::list<boost::shared_ptr<ChannelWatcher> > watcher_list_t;
  
  /** Channel watcher for the other entities */
  watcher_list_t                          m_others;

  /** Flag to remember if the watcher list has been explicitly called */
  bool                                    no_explicit_entity_watch;
  
  /** Event feedback set */
  struct EventFeedback {
    /** Window name */
    std::string                           winname;
    /** Desired events */
    unsigned                              eventmask;
    /** channel token */
    boost::shared_ptr<ChannelWriteToken>  w_event;
  };

  /** Channels for sending the pointer and keyboard events */
  std::list<EventFeedback>                w_events;

  /** Channel for receiving on-line configuration events */
  ChannelReadToken                        r_config;

private: // activity allocation
  /** Callback object for simulation calculation. */
  Callback<WorldView>   cb1;

  /** Activity for simulation calculation. */
  ActivityCallback      do_calc;

  /** Clock */
  PeriodicAlarm         myclock;

  /** Clock */
  AperiodicAlarm        myalarm;
  
public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char* const           classname;

  /** Return the parameter table. */
  static const ParameterTable*       getMyParameterTable();
  
public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  WorldView(Entity* e, const char* part, const PrioritySpec& ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */
  bool complete();

  /** Destructor. */
  ~WorldView();

  // add here the member functions you want to be called with further 
  // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here. 
  // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec& ts);

  /** Request check on the timing. */
  bool checkTiming(const std::vector<int>& i);

  /** Add a key/pointer event writer */
  bool setWindowEventWriting(const std::vector<std::string>& s);

  /** Specify which channels need to be monitored for entries */
  bool addWorldInformationChannel(const std::vector<std::string>& s);
  
  /** Specify the own motion channel name */
  bool setEgoMotionChannel(const std::string& n);
  
public: // member functions for cooperation with DUECA
  /** indicate that everything is ready. */
  bool isPrepared();

  /** start responsiveness to input data. */
  void startModule(const TimeSpec &time);
  
  /** stop responsiveness to input data. */
  void stopModule(const TimeSpec &time);

public: // the member functions that are called for activities
  /** the method that implements the main calculation. */
  void doCalculation(const TimeSpec& ts);
};

#endif
