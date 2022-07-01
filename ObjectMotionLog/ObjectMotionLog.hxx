/* ------------------------------------------------------------------   */
/*      item            : ObjectMotionLog.hxx
        made by         : repa
	from template   : DusimeModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Fri May 14 15:13:56 2010
	category        : header file 
        description     : 
	changes         : Fri May 14 15:13:56 2010 first version
	template changes: 030401 RvP Added template creation comment
        language        : C++
*/

#ifndef ObjectMotionLog_hxx
#define ObjectMotionLog_hxx

// include the dusime header
#include <dusime.h>
USING_DUECA_NS;

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"

// include headers for functions/classes you need in the module
#include <matio.h>

/** Log a multi-stream channel with ObjectMotion stuff.

    This logs to one or more matlab files. The ("world",
    "ObjectMotion", "") channel is opened and read. Each of the
    entries in that channel is logged in a Matlab file, with a
    variable under its own name. A maximum number of logpoints must be
    supplied. If the maximum number of logpoints is exceeded, new
    files are created to handle the rest of the data.

    The files are tagged with a UTC time stamp. 

    Also, if the simulation is reset to holdcurrent, the log is
    finished and a new log will be started when switching to advance. 

    The logging uses the matio library. 
    
    The instructions to create an module of this class from the Scheme
    script are:

    \verbinclude object-motion-log.scm
*/
class ObjectMotionLog: public SimulationModule
{
  struct LoggedData 
  {
    /** Counter with current point */
    int count;

    /** size */
    int npoints;

    /** Remember until what time this variable has been logged */
    TimeTickType check;
    
    /** Pointer to the data */
    double *data;
    
    /** Add a line, returns true when full */
    bool addLine(const ObjectMotion& o, const DataTimeSpec& dts, 
		 const TimeSpec& ts);

    /** Constructor. */
    LoggedData(int npoints = 1000);

    /** copy constructor. */
    LoggedData(const LoggedData& o);
  };

private: // simulation data
  /** file */
  mat_t                        *file;
  
  /** Base name for the file. */
  std::string                   filebase;

  /** Default maximum data size */
  int                           maxpts;

  /** Readback for the channel. Take a safe value, to get the proper data */
  int                           readback;

  /** Controls to keep the logging lagging by readback */
  int                           startcount, stopcount;

  /** map with data to be saved */
  std::map<string32,LoggedData> logbuffer;

  /** need to transpose the data before logging. */
  double                       *t_data;

private: // trim calculation data
  // declare the trim calculation data needed for your simulation

private: // snapshot data
  // declare, if you need, the room for placing snapshot data

private: // channel access
  MultiStreamReadToken<ObjectMotion>   r_motion;

private: // activity allocation
  /** Callback object for simulation calculation. */
  Callback<ObjectMotionLog>  cb1;

  /** Activity for simulation calculation. */
  ActivityCallback      do_calc;

  /** Clock for triggering, multistream channel does not do that for
      me */
  PeriodicAlarm          alarm;
  
public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char* const           classname;

  /** Return the initial condition table. */
  static const IncoTable*            getMyIncoTable();

  /** Return the parameter table. */
  static const ParameterTable*       getMyParameterTable();
  
public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  ObjectMotionLog(Entity* e, const char* part, const PrioritySpec& ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */
  bool complete();

  /** Destructor. */
  ~ObjectMotionLog();

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

public: // the member functions that are called for activities
  /** the method that implements the main calculation. */
  void doCalculation(const TimeSpec& ts);

public: // member functions for cooperation with DUSIME
  /** For the Snapshot capability, fill the snapshot "snap" with the 
      data saved at a point in your simulation (if from_trim is false)
      or with the state data calculated in the trim calculation (if
      from_trim is true). */
  void fillSnapshot(const TimeSpec& ts, 
		    Snapshot& snap, bool from_trim);
  
  /** Restoring the state of the simulation from a snapshot. */
  void loadSnapshot(const TimeSpec& t, const Snapshot& snap);

  /** Perform a trim calculation. Should NOT use current state 
      uses event channels parallel to the stream data channels,
      calculates, based on the event channel input, the steady state
      output. */
  void trimCalculation(const TimeSpec& ts, const TrimMode& mode);

private:
  /** Helper, open an new mat file */
  void newMatfile();
  
  /** Helper, write the current mat file */
  void writeMatfile();
};

#endif
