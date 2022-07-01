/* ------------------------------------------------------------------   */
/*      item            : ConfigureView.hxx
        made by         : repa
	from template   : DuecaModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Fri Jun 11 22:16:57 2010
	category        : header file 
        description     : 
	changes         : Fri Jun 11 22:16:57 2010 first version
	template changes: 030401 RvP Added template creation comment
        language        : C++
*/

#ifndef ConfigureView_hxx
#define ConfigureView_hxx

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

    \verbinclude configure-view.scm
 */
class ConfigureView: public Module
{
private: // simulation data
  /** Window with controls */
  GtkGladeWindow                    viewcontrol;

  /** Current configuration */
  WorldDataSpec                     config;

  /** Current viewpsecs */
  ViewSpec	viewspec;
  
private: // channel access
  /** Write the configation */
  EventChannelWriteToken<WorldViewConfig>  w_config;

private: // activity allocation
  /** Callback object for simulation calculation. */
  Callback<ConfigureView>  cb1;

  /** Activity for simulation calculation. */
  ActivityCallback      do_calc;
  
public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char* const           classname;

  /** Return the parameter table. */
  static const ParameterTable*       getMyParameterTable();
  
public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  ConfigureView(Entity* e, const char* part, const PrioritySpec& ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */
  bool complete();

  /** Destructor. */
  ~ConfigureView();

  // add here the member functions you want to be called with further 
  // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here. 
  // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec& ts);

  /** Request check on the timing. */
  bool checkTiming(const vector<int>& i);

  void doAction(GtkToolButton* button, gpointer gp);

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
