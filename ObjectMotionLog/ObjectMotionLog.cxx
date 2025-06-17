/* ------------------------------------------------------------------   */
/*      item            : ObjectMotionLog.cxx
        made by         : repa
	from template   : DusimeModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Fri May 14 15:13:56 2010
	category        : body file
        description     :
	changes         : Fri May 14 15:13:56 2010 first version
	template changes: 030401 RvP Added template creation comment
	                  060512 RvP Modified token checking code
        language        : C++
*/

static const char c_id[] =
"$Id: ObjectMotionLog.cxx,v 1.5 2015/03/19 20:45:48 rvanpaassen Exp $";

#define ObjectMotionLog_cxx
// include the definition of the module class
#include "ObjectMotionLog.hxx"

// include the debug writing header, by default, write warning and
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here
#include <sys/time.h>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <Eigen/Dense>

typedef Eigen::Map<Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::ColMajor> > MatrixEc;
typedef Eigen::Map<Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> > MatrixE;
#define USING_EIGEN3

// guess on matio versions here
#if MATIO_MAJOR_VERSION == 1 && MATIO_MINOR_VERSION < 4
#define MAT_COMPRESSION_NONE COMPRESSION_NONE
#endif

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dusime.h>

ObjectMotionLog::LoggedData::LoggedData(int npoints) :
  count(0),
  npoints(npoints),
  check(0),
  data(NULL)
{
  assert(npoints > 0);
  //
}

ObjectMotionLog::LoggedData::LoggedData(const LoggedData& o) :
  count(0),
  npoints(o.npoints),
  check(0),
  data(NULL)
{
  assert(npoints > 0);
  //
}


bool ObjectMotionLog::LoggedData::addLine
(const ObjectMotion& o, const DataTimeSpec& dts, const TimeSpec& ts)
{
  if (data == NULL) data = new double[npoints*12];
  if (check && check + ts.getValiditySpan() != dts.getValidityStart()) {
    W_MOD("time anomaly data logging, have " << check << " new " << dts);
    if (check >= dts.getValidityStart()) return false;
  }
  check = dts.getValidityStart();
  data[12*count+0] = o.xyz[0];
  data[12*count+1] = o.xyz[1];
  data[12*count+2] = o.xyz[2];
  data[12*count+3] = o.uvw[0];
  data[12*count+4] = o.uvw[1];
  data[12*count+5] = o.uvw[2];
  data[12*count+6] = o.getPhi();
  data[12*count+7] = o.getTht();
  data[12*count+8] = o.getPsi();
  data[12*count+9] = o.omega[0];
  data[12*count+10] = o.omega[1];
  data[12*count+11] = o.omega[2];
  return (++count == npoints);
}

// class/module name
const char* const ObjectMotionLog::classname = "object-motion-log";

// initial condition/trim table
const IncoTable* ObjectMotionLog::getMyIncoTable()
{
  static IncoTable inco_table[] = {
    // enter pairs of IncoVariable and VarProbe pointers (i.e.
    // objects made with new), in this table.
    // For example
//    {(new IncoVariable("example", 0.0, 1.0, 0.01))
//     ->forMode(FlightPath, Constraint)
//     ->forMode(Speed, Control),
//     new VarProbe<ObjectMotionLog,double>
//       (REF_MEMBER(&ObjectMotionLog::i_example))}

    // always close off with:
    { NULL, NULL} };

  return inco_table;
}

// parameters to be inserted
const ParameterTable* ObjectMotionLog::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing",
      new MemberCall<ObjectMotionLog,TimeSpec>
        (&ObjectMotionLog::setTimeSpec), set_timing_description },

    { "check-timing",
      new MemberCall<ObjectMotionLog,vector<int> >
      (&ObjectMotionLog::checkTiming), check_timing_description },

    { "base-file-name",
      new VarProbe<ObjectMotionLog,string>
      (&ObjectMotionLog::filebase),
      "base name (without date/extension) for the logging" },

    { "points-per-file",
      new VarProbe<ObjectMotionLog,int>
      (&ObjectMotionLog::maxpts),
      "maximum size of the vectors in the log file" },

    { "readback",
      new VarProbe<ObjectMotionLog,int>
      (&ObjectMotionLog::readback),
      "how many granules should the logger look back in the channel\n"
      "default is 2x timing span, is overwritten by 'set-timing call" },

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string).

       Note that for efficiency, set_timing_description and
       check_timing_description are pointers to pre-defined strings,
       you can simply enter the descriptive strings in the table. */

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL, "please give a description of this module"} };

  return parameter_table;
}

// constructor
ObjectMotionLog::ObjectMotionLog(Entity* e, const char* part, const
		       PrioritySpec& ps) :
  /* The following line initialises the SimulationModule base class.
     You always pass the pointer to the entity, give the classname and the
     part arguments.
     If you give a NULL pointer instead of the inco table, you will not be
     called for trim condition calculations, which is normal if you for
     example implement logging or a display.
     If you give 0 for the snapshot state, you will not be called to
     fill a snapshot, or to restore your state from a snapshot. Only
     applicable if you have no state. */
  SimulationModule(e, classname, part, getMyIncoTable(), 0),

  // initialize the data you need in your simulation
  file(NULL),
  filebase("objectmotion"),
  maxpts(1000),
  readback(40),
  startcount(0),
  stopcount(0),
  t_data(NULL),

  // initialize the data you need for the trim calculation

  // initialize the channel access tokens
  r_motion(getId(), NameSet(getEntity(), "ObjectMotion", ""), 2000),

  // activity initialization
  cb1(this, &ObjectMotionLog::doCalculation),
  do_calc(getId(), "log objectmotion", &cb1, ps),
  alarm()
{
  // do the actions you need for the simulation

  // connect the triggers for simulation
  do_calc.setTrigger(alarm);
}

bool ObjectMotionLog::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  if (maxpts < 1 || readback < 0) {
    E_CNF("wrong configuration");
    return false;
  }
  delete(t_data);
  t_data = new double[12*maxpts];

  return true;
}

// destructor
ObjectMotionLog::~ObjectMotionLog()
{
  //
}

// as an example, the setTimeSpec function
bool ObjectMotionLog::setTimeSpec(const TimeSpec& ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0) return false;

  // specify the timespec to the activity
  alarm.changePeriodAndOffset(ts);
  readback = (5*ts.getValiditySpan());

  // return true if everything is acceptable
  return true;
}

// and the checkTiming function
bool ObjectMotionLog::checkTiming(const vector<int>& i)
{
  if (i.size() == 3) {
    new TimingCheck(do_calc, i[0], i[1], i[2]);
  }
  else if (i.size() == 2) {
    new TimingCheck(do_calc, i[0], i[1]);
  }
  else {
    return false;
  }
  return true;
}

// tell DUECA you are prepared
bool ObjectMotionLog::isPrepared()
{
  bool res = true;

  // Example checking a token:
  CHECK_TOKEN(r_motion);

  // return result of checks
  return res;
}


void ObjectMotionLog::newMatfile()
{
  time_t tnow = time(NULL);
  struct tm tsplit; gmtime_r(&tnow, &tsplit);

  stringstream fname;
  fname << filebase << setfill('0') << setw(2) << tsplit.tm_year
	<< tsplit.tm_mon << tsplit.tm_mday << '_'
	<< tsplit.tm_hour << ':' << tsplit.tm_min << ':' << tsplit.tm_sec
	<< ".mat";
  file = Mat_Open(fname.str().c_str(), MAT_ACC_RDWR);
}

void ObjectMotionLog::writeMatfile()
{
  for (map<string32,LoggedData>::iterator lb = logbuffer.begin();
       lb != logbuffer.end(); lb++) {
    if (lb->second.count) {
#if MATIO_MAJOR_VERSION == 1 && MATIO_MINOR_VERSION < 4
      int dims[2] = {lb->second.count, 12};
#else
      size_t dims[2] = {size_t(lb->second.count), size_t(12)};
#endif
      // hackety hack. Have to transpose the thing
      MatrixEc t(t_data, dims[0], dims[1]);
      MatrixE d(lb->second.data, dims[0], dims[1]);
      t = d;
      matvar_t *matvar = Mat_VarCreate
	(lb->first.c_str(), MAT_C_DOUBLE, MAT_T_DOUBLE,
	 2, dims, t_data, 0);
      Mat_VarWrite( file, matvar, MAT_COMPRESSION_NONE);
      lb->second.count = 0;
      Mat_VarFree(matvar);
    }
  }
  Mat_Close(file);
  file = NULL;
}

// start the module
void ObjectMotionLog::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void ObjectMotionLog::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// fill a snapshot with state data. You may remove this method (and the
// declaration) if you specified to the SimulationModule that the size of
// state snapshots is zero
void ObjectMotionLog::fillSnapshot(const TimeSpec& ts,
                              Snapshot& snap, bool from_trim)
{
  // The most efficient way of filling a snapshot is with an AmorphStore
  // object.
  AmorphStore s(snap.accessData(), snap.getDataSize());

  if (from_trim) {
    // use packData(s, trim_state_variable1); ... to pack your state into
    // the snapshot
  }
  else {
    // this is a snapshot from the running simulation. Dusime takes care
    // that no other snapshot is taken at the same time, so you can safely
    // pack the data you copied into (or left into) the snapshot state
    // variables in here
    // use packData(s, snapshot_state_variable1); ...
  }
}

// reload from a snapshot. You may remove this method (and the
// declaration) if you specified to the SimulationModule that the size of
// state snapshots is zero
void ObjectMotionLog::loadSnapshot(const TimeSpec& t, const Snapshot& snap)
{
  // access the data in the snapshot with an AmorphReStore object
  AmorphReStore s(snap.data, snap.data_size);

  // use unPackData(s, real_state_variable1 ); ... to unpack the data
  // from the snapshot.
  // You can safely do this, while snapshot loading is going on the
  // simulation is in HoldCurrent or the activity is stopped.
}

// this routine contains the main simulation process of your module. You
// should read the input channels here, and calculate and write the
// appropriate output
void ObjectMotionLog::doCalculation(const TimeSpec& ts)
{
  // check the state we are supposed to be in
  switch (getAndCheckState(ts)) {
  case SimulationState::HoldCurrent: {

    startcount = readback;
    if (stopcount <= 0) return;
    stopcount -= ts.getValiditySpan();

    break;
  }

  case SimulationState::Replay:
  case SimulationState::Advance: {
    stopcount = readback;

    startcount -= ts.getValiditySpan();
    if (startcount > 0) return;
  }

    break;
  default:
    // other states should never be entered for a SimulationModule,
    // HardwareModules on the other hand have more states. Throw an
    // exception if we get here,
    throw CannotHandleState(getId(),GlobalId(), "state unhandled");
  }

  // for the first time logging after an advance
  if (file == NULL) newMatfile();

  bool full = false;
  for (r_motion.selectFirstEntry(); r_motion.haveEntry();
       r_motion.selectNextEntry()) {

    MultiStreamReader<ObjectMotion> r(r_motion, ts - readback);
    map<string32,LoggedData>::iterator ii = logbuffer.find(r.data().name);
    if (ii == logbuffer.end()) {
      logbuffer[r.data().name] = LoggedData(maxpts);
      ii = logbuffer.find(r.data().name);
    }
    full = full ||
      ii->second.addLine(r.data(), r.timeSpec(), ts);
  }

  // last round in a holdcurrent state, write out the mat file
  // or when the file is full
  if (stopcount <= 0 || full) {
    writeMatfile();
  }

}

void ObjectMotionLog::trimCalculation(const TimeSpec& ts, const TrimMode& mode)
{
  // read the event equivalent of the input data
  // example
  // EventReader u(i_input_token, ts);

  // using the input, and the data put into your trim variables,
  // calculate the derivative of the state. DO NOT use the state
  // vector of the normal simulation here, because it might be that
  // this is done while the simulation runs!
  // Some elements in this state derivative are needed as target, copy
  // these out again into trim variables (see you TrimTable

  // trim calculation
  switch(mode) {
  case FlightPath: {
    // one type of trim calculation, find a power setting and attitude
    // belonging to a flight path angle and speed
  }
  break;

  case Speed: {
    // find a flightpath belonging to a speed and power setting (also
    // nice for gliders)
  }
  break;

  case Ground: {
    // find an altitude/attitude belonging to standing still on the
    // ground, power/speed 0
  }
  break;

  default:
    W_MOD(getId() << " cannot calculate inco mode " << mode);
  break;
  }

  // This works just like a normal calculation, only you provide the
  // steady state value (if your system is stable anyhow). So, if you
  // have other modules normally depending on your output, you should
  // also produce the equivalent output here.
  // EventWriter<MyOutput> y(output_token, ts);

  // write the output into the output channel, using the EventWriter
  // N.B. you may also use:
  // output_token.sendEvent(ts, new MyOutput(arguments));
  // this is slightly more efficient. However, the EventWriter makes
  // it easier, because the code will look much more like your normal
  // calculation code.

  // now return. The real results from the trim calculation, as you
  // specified them in the TrimTable, will now be collected and sent
  // off for processing.
}

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the scheme-interpreting code, and enable the
// creation of modules of this type
static TypeCreator<ObjectMotionLog> a(ObjectMotionLog::getMyParameterTable());
