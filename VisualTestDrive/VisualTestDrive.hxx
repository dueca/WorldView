/* ------------------------------------------------------------------   */
/*      item            : VisualTestDrive.hxx
        made by         : repa
        from template   : DusimeModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Tue Jun 30 20:07:16 2009
        category        : header file
        description     :
        changes         : Tue Jun 30 20:07:16 2009 first version
        template changes: 030401 RvP Added template creation comment
        language        : C++
*/

#ifndef VisualTestDrive_hxx
#define VisualTestDrive_hxx

// include the dusime header
#include "ChannelWriteToken.hxx"
#include "WorldDataSpec.hxx"
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <dusime.h>
USING_DUECA_NS;

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"

// include headers for functions/classes you need in the module
#include <list>
#include <boost/scoped_ptr.hpp>

/** A module for generating test data for visual check-out.

    The instructions to create an module of this class from the Scheme
    script are:

    \verbinclude visual-test-drive.scm
*/
class VisualTestDrive : public SimulationModule
{
  typedef VisualTestDrive _ThisModule_;

private: // simulation data
  /** Combination of an initial and current motion description */
  struct MotionSet
  {
    /** Initial motion object */
    ObjectMotion initial;

    /** Current motion object, gets updated during simulation */
    ObjectMotion moving;

        /* channel token pointer for writing the results */
    ChannelWriteToken *w_entity;

    /** Constructor, with initial motion object and token pointer */
    MotionSet(const ObjectMotion &i,
              ChannelWriteToken *w_entity) :
      initial(i),
      moving(i),
      w_entity(w_entity)
    {}

    /** Constructor, with only motion object and token pointer */
    MotionSet(const ObjectMotion &i) :
      initial(i),
      moving(i)
    {}

     /** Empty constructor, placeholder */
    MotionSet() :
      w_entity(NULL){};
  };

  /** List of motion sets. */
  std::list<MotionSet> motion_sets;

  /** MotionSet to be filled by script */
  ObjectMotion temp;

  /** Ego motion set. */
  MotionSet ego;

  /** Data set for flightgear object */
  WorldDataSpec fgspec;

  /** New flightgear test set */
  struct FlightGearTestSet
  {
    /** Which part of the airplane are we trying to move */
    unsigned moving_part;

    /** Time counter */
    double tmove;

    /** Time scaler for movement */
    double tscale;

    FGBaseAircraftMotion initial;

    /** Moving and flapping aircraft motion */
    FGBaseAircraftMotion moving;

    /** Channel write token */
    boost::scoped_ptr<ChannelWriteToken> w_entry;

    /** Constructor, with only motion object and token pointer */
    FlightGearTestSet(const WorldDataSpec &i, Module* module);

    /** Advance step */
    void advance(const DataTimeSpec& ts, bool move);
  };

  /** Flightgear enabled test sets*/
  std::list<FlightGearTestSet>  fg_sets;

  /** Parameter callback, add the name of a motion device */
  bool addMotion(const std::string &name);

  /** Parameter callback, set the device class name */
  bool setClass(const std::string &klass);

  /** Parameter callback, adds an initial position */
  bool setPosition(const std::vector<float> &x);

  /** Parameter callback, sets speed */
  bool setSpeed(const std::vector<float> &x);

  /** Parameter callback, sets orientation */
  bool setOrientation(const std::vector<float> &x);

  /** Parameter callback, sets rotational speed */
  bool setRotation(const std::vector<float> &x);

  /** Parameter callback, set time step */
  bool setDt(const double &x);

  /** Flightgear motion object setting. */
  bool addFGMotion(const std::string& label);

  /** Flightgear motion parameters */
  bool supplyFGMotion(const std::vector<double>& coords);

private: // trim calculation data
  // declare the trim calculation data needed for your simulation
private: // snapshot data
  // declare, if you need, the room for placing snapshot data
private: // channel access
  // declare StreamChannelReadToken<Type>, StreamChannelWriteToken<Type>
  // EventChannelReadToken<Type>, EventChannelWriteToken<Type>
  // access tokens for all the channels you read and write
  // example
  ChannelWriteToken w_ego;

private: // activity allocation
  /** Callback object for simulation calculation. */
  Callback<VisualTestDrive> cb1;

  /** Activity for simulation calculation. */
  ActivityCallback do_calc;

  /** Clock */
  PeriodicAlarm myclock;

public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char *const classname;

  /** Return the initial condition table. */
  static const IncoTable *getMyIncoTable();

  /** Return the parameter table. */
  static const ParameterTable *getMyParameterTable();

public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  VisualTestDrive(Entity *e, const char *part, const PrioritySpec &ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */
  bool complete();

  /** Destructor. */
  ~VisualTestDrive();

  // add here the member functions you want to be called with further
   // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here.
   // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec &ts);

  /** Request check on the timing. */
  bool checkTiming(const vector<int> &i);

public: // member functions for cooperation with DUECA
  /** indicate that everything is ready. */
  bool isPrepared();

  /** start responsiveness to input data. */
  void startModule(const TimeSpec &time);

    /** stop responsiveness to input data. */
  void stopModule(const TimeSpec &time);

public: // the member functions that are called for activities
  /** the method that implements the main calculation. */
  void doCalculation(const TimeSpec &ts);

public: // member functions for cooperation with DUSIME
  /** For the Snapshot capability, fill the snapshot "snap" with the
      data saved at a point in your simulation (if from_trim is false)
      or with the state data calculated in the trim calculation (if
      from_trim is true). */
  void fillSnapshot(const TimeSpec &ts, Snapshot &snap, bool from_trim);

    /** Restoring the state of the simulation from a snapshot. */
  void loadSnapshot(const TimeSpec &t, const Snapshot &snap);

  /** Perform a trim calculation. Should NOT use current state
      uses event channels parallel to the stream data channels,
      calculates, based on the event channel input, the steady state
      output. */
  void trimCalculation(const TimeSpec &ts, const TrimMode &mode);
};

#endif
