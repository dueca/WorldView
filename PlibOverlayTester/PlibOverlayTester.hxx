/* ------------------------------------------------------------------   */
/*      item            : PlibOverlayTester.hxx
        made by         : rvanpaassen
	from template   : DuecaModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Fri Mar 19 16:25:26 2010
	category        : header file 
        description     : 
	changes         : Fri Mar 19 16:25:26 2010 first version
	template changes: 030401 RvP Added template creation comment
        language        : C++
*/

#ifndef PlibOverlayTester_hxx
#define PlibOverlayTester_hxx

// include the dusime header
#include <dueca.h>
USING_DUECA_NS;

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"

// include headers for functions/classes you need in the module
#include "PlibTestOverlay.hxx"

/** The PlibOverlayTester is a dueca module that has a direct
    communication to a PlibTestOverlay object that can be created in
    the dueca start-up script. It was written as a proof-of-concept
    for controlling an overlay (or basically, anything else you want
    controlled) that is handled by the PlibViewer object. 
    
    Adapt and extend to -- for example -- create a HUD. 
    
    The instructions to create an module of this class from the Scheme
    script are:

    \verbinclude plib-overlay-tester.scm
 */
class PlibOverlayTester: public Module
{
private: // simulation data
  // declare the data you need in your simulation
  PlibTestOverlay* overlay;

private: // channel access
  // No channels used here, but extend if you want

private: // activity allocation
  /** clock */
  PeriodicAlarm         myclock;
  
  /** Callback object for simulation calculation. */
  Callback<PlibOverlayTester>  cb1;

  /** Activity for simulation calculation. */
  ActivityCallback      do_calc;
  
public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char* const           classname;

  /** Return the parameter table. */
  static const ParameterTable*       getMyParameterTable();
  
public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  PlibOverlayTester(Entity* e, const char* part, const PrioritySpec& ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */
  bool complete();

  /** Destructor. */
  ~PlibOverlayTester();

  // add here the member functions you want to be called with further 
  // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here. 
  // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec& ts);

  /** Request check on the timing. */
  bool checkTiming(const vector<int>& i);

  /** Add the overlay */
  bool setOverlay(ScriptCreatable &obj, bool in);

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
