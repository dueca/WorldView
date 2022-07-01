/* ------------------------------------------------------------------   */
/*      item            : ViewpointHack.hxx
        made by         : repa
	from template   : DuecaModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Wed Sep 11 14:37:24 2013
	category        : header file 
        description     : 
	changes         : Wed Sep 11 14:37:24 2013 first version
	template changes: 030401 RvP Added template creation comment
        language        : C++
*/

#ifndef ViewpointHack_hxx
#define ViewpointHack_hxx

// include the dusime header
#include <dueca.h>
USING_DUECA_NS;

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"

// include headers for functions/classes you need in the module
#include <GtkGladeWindow.hxx>


/** A simulation module.
    
    The instructions to create an module of this class from the Scheme
    script are:

    \verbinclude viewpoint-hack.scm
 */
class ViewpointHack: public Module
{
private: // simulation data
  /** Window with controls */
  GtkGladeWindow                    viewcontrol;

private: // channel access
  // no channel access

private: // activity allocation
  /** You might also need a clock. Don't mis-use this, because it is
      generally better to trigger on the incoming channels */
  PeriodicAlarm        myclock;

  /** Callback object for simulation calculation. */
  Callback<ViewpointHack>  cb1;

  /** Activity for simulation calculation. */
  ActivityCallback      do_calc;
  
public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char* const           classname;

  /** Return the parameter table. */
  static const ParameterTable*       getMyParameterTable();
  
public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  ViewpointHack(Entity* e, const char* part, const PrioritySpec& ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */
  bool complete();

  /** Destructor. */
  ~ViewpointHack();

  // add here the member functions you want to be called with further 
  // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here. 
  // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec& ts);

  /** Request check on the timing. */
  bool checkTiming(const vector<int>& i);

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
  void doCalculation(const TimeSpec& ts);
};

#endif
