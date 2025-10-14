/* ------------------------------------------------------------------   */
/*      item            : ControlView.hxx
        made by         : rvanpaassen
        from template   : DuecaModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Fri Jan 29 13:51:31 2010
        category        : header file
        description     :
        changes         : Fri Jan 29 13:51:31 2010 first version
        template changes: 030401 RvP Added template creation comment
        language        : C++
*/

#ifndef ControlView_hxx
#define ControlView_hxx

// include the dusime header
#include <dueca.h>
USING_DUECA_NS;

#include <GtkGladeWindow.hxx>

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"

// include headers for functions/classes you need in the module
#include <list>
#include <string>

/** This is a test module for the WorldView class. It generate an
    "own" aircraft controllable by a GTK interface, as well as a
    variable number of other aircraft driven by their initial speed.

    In addition, reading and printing the stream of interface events
    (implemented currently only for the plib interface) is added to
    this class.

    The instructions to create an module of this class from the Scheme
    script are:

    \verbinclude control-view.scm
 */
class ControlView : public Module
{
private: // simulation data
  /** Position and orientation */
  double current[6];

  /** Object sent over channels */
  ObjectMotion object;

  /** Window with controls */
  GtkGladeWindow viewcontrol;

private: // channel access
  StreamChannelWriteToken<ObjectMotion> w_entity;
  EventChannelWriteToken<WorldViewConfig> w_config;

  /** struct to keep window event tokens */
  struct WindowEventSet
  {
    /** Nameset of the read token */
    std::vector<std::string> name;
    /** Read token itself */
    EventChannelReadToken<WorldViewerEvent> *r_event;

    /** Constructor */
    WindowEventSet(const GlobalId &master_id, std::vector<std::string> names);
  };

  /** List of event tokens for reading stuff from windows. */
  std::list<WindowEventSet> wvwindow_events;

private: // activity allocation
  /** Periodic clock */
  PeriodicAlarm myclock;

  /** Callback object for simulation calculation. */
  Callback<ControlView> cb1;

  /** Activity for simulation calculation. */
  ActivityCallback do_calc;

public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char *const classname;

  /** Return the parameter table. */
  static const ParameterTable *getMyParameterTable();

public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  ControlView(Entity *e, const char *part, const PrioritySpec &ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */
  bool complete();

  /** Destructor. */
  ~ControlView();

  // add here the member functions you want to be called with further
  // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here.
  // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec &ts);

  /** Request check on the timing. */
  bool checkTiming(const vector<int> &i);

  /** Parameter callback, adds an initial position */
  bool setPosition(const std::vector<float> &x);

  /** Viewer events reading */
  bool addWindowEventReader(const std::vector<std::string> &n);

  /** Parameter callback, sets orientation */
  bool setOrientation(const std::vector<float> &x);

public: // member functions for cooperation with DUECA
  /** indicate that everything is ready. */
  bool isPrepared();

  /** start responsiveness to input data. */
  void startModule(const TimeSpec &time);

  /** stop responsiveness to input data. */
  void stopModule(const TimeSpec &time);

  /** position change */
  void changePositionOrientation(GtkSpinButton *widget, gpointer udata);

public: // the member functions that are called for activities
  /** the method that implements the main calculation. */
  void doCalculation(const TimeSpec &ts);
};

#endif
