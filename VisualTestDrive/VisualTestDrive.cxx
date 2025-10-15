/* ------------------------------------------------------------------   */
/*      item            : VisualTestDrive.cxx
        made by         : repa
        from template   : DusimeModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Tue Jun 30 20:07:16 2009
        category        : body file
        description     :
        changes         : Tue Jun 30 20:07:16 2009 first version
        template changes: 030401 RvP Added template creation comment
                          060512 RvP Modified token checking code
        language        : C++
*/

static const char c_id[] =
  "$Id: VisualTestDrive.cxx,v 1.8 2016/08/12 09:55:22 rvanpaassen Exp $";

#define VisualTestDrive_cxx
// include the definition of the module class
#include "VisualTestDrive.hxx"

// include the debug writing header, by default, write warning and
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dusime.h>

// class/module name
const char *const VisualTestDrive::classname = "visual-test-drive";

// initial condition/trim table
const IncoTable *VisualTestDrive::getMyIncoTable()
{
  static IncoTable inco_table[] = {
    // enter pairs of IncoVariable and VarProbe pointers (i.e.
    // objects made with new), in this table.
    // For example
//    {(new IncoVariable("example", 0.0, 1.0, 0.01))
//     ->forMode(FlightPath, Constraint)
//     ->forMode(Speed, Control),
//     new VarProbe<VisualTestDrive,double>
//       (REF_MEMBER(&VisualTestDrive::i_example))}

    // always close off with:
    { NULL, NULL }
  };

  return inco_table;
}

// parameters to be inserted
const ParameterTable *VisualTestDrive::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing",
      new MemberCall<VisualTestDrive, TimeSpec>(&VisualTestDrive::setTimeSpec),
      set_timing_description },

    { "check-timing",
      new MemberCall<VisualTestDrive, vector<int>>(
        &VisualTestDrive::checkTiming),
      check_timing_description },

    { "position",
      new MemberCall<VisualTestDrive, vector<float>>(
        &VisualTestDrive::setPosition),
      "Set initial position" },
    { "speed",
      new MemberCall<VisualTestDrive, vector<float>>(
        &VisualTestDrive::setSpeed),
      "Set initial speed" },
    { "orientation",
      new MemberCall<VisualTestDrive, vector<float>>(
        &VisualTestDrive::setOrientation),
      "Set initial orientation" },
    { "rotation",
      new MemberCall<VisualTestDrive, vector<float>>(
        &VisualTestDrive::setRotation),
      "Set initial rotation" },

    { "dt", new MemberCall<VisualTestDrive, double>(&VisualTestDrive::setDt),
      "set integration time step" },

    { "add-motion",
      new MemberCall<VisualTestDrive, string>(&VisualTestDrive::addMotion),
      "Add a new motion drive." },

    { "set-class",
      new MemberCall<VisualTestDrive, string>(&VisualTestDrive::setClass),
      "Specify the object class for a new motion drive" },
    { "add-fg-motion",
      new MemberCall<_ThisModule_, std::string>(&_ThisModule_::addFGMotion),
      "Create a flightgear enhanced aircraft entry, which moves various "
      "parts." },

    { "fg-motion-parameters",
      .probe = new MemberCall<_ThisModule_, std::vector<double>>(
        &_ThisModule_::supplyFGMotion),
      "Add coordinates for flightgear object, xyz, phi, theta, psi, uvw, "
      "omega, movement duration" },

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string).

       Note that for efficiency, set_timing_description and
       check_timing_description are pointers to pre-defined strings,
       you can simply enter the descriptive strings in the table. */

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL, "Test data for view module" }
  };

  return parameter_table;
}

// constructor
VisualTestDrive::VisualTestDrive(Entity *e, const char *part,
                                 const PrioritySpec &ps) :
  /* The following line initialises the SimulationModule base class.
     You always pass the pointer to the entity, give the classname and the
     part arguments.
     If you give a NULL pointer instead of the inco table, you will not be
     called for trim condition calculations, which is normal if you for
     example implement logging or a display.
     If you give 0 for the snapshot state, you will not be called to
     fill a snapshot, or to restore your state from a snapshot. Only
     applicable if you have no state. */
  SimulationModule(e, classname, part, getMyIncoTable()),

  // initialize the data you need in your simulation

  // initialize the data you need for the trim calculation

  // initialize the channel access tokens
  // example
  // my_token(getId(), NameSet(getEntity(), "MyData", part)),
  w_ego(getId(), NameSet(getEntity(), "ObjectMotion", part),
        ObjectMotion::classname, getEntity()),

  // activity initialization
  cb1(this, &VisualTestDrive::doCalculation),
  do_calc(getId(), "dummy data for visual", &cb1, ps)
{
  // connect the triggers for simulation
  do_calc.setTrigger(myclock);
  fgspec.coordinates.resize(13, 0.0);
  fgspec.coordinates[12] = 1.0;

  // connect the triggers for trim calculation. Leave this out if you
  // don not need input for trim calculation
  // trimCalculationCondition(/* fill in your trim triggering channels */);
}

bool VisualTestDrive::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  addMotion("");
  addFGMotion("");
  return true;
}

// destructor
VisualTestDrive::~VisualTestDrive()
{
  //
}

// as an example, the setTimeSpec function
bool VisualTestDrive::setTimeSpec(const TimeSpec &ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0)
    return false;

  // specify the timespec to the activity
  myclock.changePeriodAndOffset(ts);
  //  do_calc.setTimeSpec(ts);

  // do whatever else you need to process this in your model
  // hint: ts.getDtInSeconds()

  // return true if everything is acceptable
  return true;
}

// and the checkTiming function
bool VisualTestDrive::checkTiming(const vector<int> &i)
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

bool VisualTestDrive::addMotion(const std::string &name)
{
  if (temp.name.size()) {
    if (!ego.initial.name.size()) {
      ego = MotionSet(temp);
    }
    else {
      motion_sets.push_back(MotionSet(
        temp,
        new ChannelWriteToken(getId(), NameSet("world", "ObjectMotion", ""),
                              ObjectMotion::classname, temp.name.c_str(),
                              Channel::Continuous, Channel::OneOrMoreEntries)));
    }
  }

  temp.name = name;
  temp.klass = name;
  return true;
}

bool VisualTestDrive::addFGMotion(const std::string &name)
{
  if (fgspec.name.size()) {
    fg_sets.emplace_back(fgspec, this);
    fgspec = WorldDataSpec();
    fgspec.coordinates.resize(13, 0.0);
    fgspec.coordinates[12] = 1.0;
  }

  // copy the label
  fgspec.name = name;
  return true;
}

bool VisualTestDrive::supplyFGMotion(const std::vector<double> &coords)
{
  for (unsigned ii = std::max(coords.size(), size_t(13)); ii--;) {
    fgspec.coordinates[ii] = coords[ii];
  }
  return true;
}

VisualTestDrive::FlightGearTestSet::FlightGearTestSet(const WorldDataSpec &i,
                                                      Module *module) :
  moving_part(0),
  tmove(0.0),
  initial(),
  moving(),
  tscale(1.0/i.coordinates[12]),
  w_entry(new ChannelWriteToken(module->getId(),
                                NameSet("world", "ObjectMotion", ""),
                                getclassname<FGBaseAircraftMotion>(), i.name,
                                Channel::Continuous, Channel::OneOrMoreEntries))
{
  initial.xyz[0] = i.coordinates[0];
  initial.xyz[1] = i.coordinates[1];
  initial.xyz[2] = i.coordinates[2];
  initial.setquat(i.coordinates[3] * M_PI / 180.0,
                  i.coordinates[4] * M_PI / 180.0,
                  i.coordinates[5] * M_PI / 180.0);
  initial.uvw[0] = i.coordinates[6];
  initial.uvw[1] = i.coordinates[7];
  initial.uvw[2] = i.coordinates[8];
  initial.omega[0] = i.coordinates[9];
  initial.omega[1] = i.coordinates[10];
  initial.omega[2] = i.coordinates[11];
  initial.dt = 1.0;
  moving = initial;
}

static double zigzag(double t, double n, double p1)
{
  if (t <= 0.0 || t >= 1.0) {
    return n;
  }
  else if (t <= 0.5) {
    return n + 2.0 * t * (p1 - n);
  }
  else {
    return 2 * p1 - n + 2.0 * t * (n - p1);
  }
}

static double zigzag(double t, double n, double p1, double p2)
{
  if (t <= 0.5) {
    return zigzag(2 * t, n, p1);
  }
  else {
    return zigzag(2 * t - 1.0, n, p2);
  }
}

void VisualTestDrive::FlightGearTestSet::advance(const DataTimeSpec &ts,
                                                 bool move)
{
  if (move) {
    tmove += tscale * ts.getDtInSeconds();
    if (tmove > 1.0) {
      moving_part++;
      if (moving_part == 16) moving_part = 0;
      tmove = 0.0;
    }
    switch (moving_part) {
    case 0:
    case 1:
    case 2:
      moving.gear_extension_norm[moving_part] = zigzag(tmove, 0.0, 1.0);
      break;
    case 3:
    case 4:
    case 5:
      moving.gear_compression_norm[moving_part - 3] = zigzag(tmove, 0.0, 1.0);
      break;
    case 6:
      moving.left_aileron_norm = zigzag(tmove, 0.0, -1.0, 1.0);
      break;
    case 7:
      moving.right_aileron_norm = zigzag(tmove, 0.0, -1.0, 1.0);
      break;
    case 8:
      moving.elevator_norm = zigzag(tmove, 0.0, -1.0, 1.0);
      break;
    case 9:
      moving.rudder_norm = zigzag(tmove, 0.0, -1.0, 1.0);
      break;
    case 10:
      moving.flaps_norm = zigzag(tmove, 0.0, 1.0);
      break;
    case 11:
      moving.speedbrake_norm = zigzag(tmove, 0.0, 1.0);
      break;
    case 12:
    case 13:
    case 14:
    case 15:
      moving.engine_rpm[moving_part-12] = zigzag(tmove, 0.0, 1200.0);
    }

    // and follow speed/rotation
    moving.extrapolate(ts.getDtInSeconds());
  }

  // anyhow write the data
  DataWriter<FGBaseAircraftMotion> wm(*w_entry, ts);
  wm.data() = moving;
}

bool VisualTestDrive::setClass(const std::string &name)
{
  temp.klass = name;
  return true;
}

// temporary hack, to survive the transition to newer channel objects
#if GENCODEGEN >= 110
#define FIXVECBEGIN .begin()
#else
#define FIXVECBEGIN
#endif

bool VisualTestDrive::setPosition(const std::vector<float> &x)
{
  if (x.size() != 3) {
    E_CNF("setPosition needs 3 parameters");
    return false;
  }
  copy(x.begin(), x.end(), temp.xyz FIXVECBEGIN);
  return true;
}

bool VisualTestDrive::setSpeed(const std::vector<float> &x)
{
  if (x.size() != 3) {
    E_CNF("setSpeed needs 3 parameters");
    return false;
  }
  copy(x.begin(), x.end(), temp.uvw FIXVECBEGIN);
  return true;
}

bool VisualTestDrive::setOrientation(const std::vector<float> &x)
{
  if (x.size() != 3) {
    E_CNF("setOrientation needs 3 parameters");
    return false;
  }
  temp.setquat(x[0] * M_PI / 180.0, x[1] * M_PI / 180.0, x[2] * M_PI / 180.0);
  return true;
}

bool VisualTestDrive::setRotation(const std::vector<float> &x)
{
  if (x.size() != 3) {
    E_CNF("setRotation needs 3 parameters");
    return false;
  }
  copy(x.begin(), x.end(), temp.omega FIXVECBEGIN);
  return true;
}

bool VisualTestDrive::setDt(const double &dt)
{
  temp.dt = dt;
  return true;
}

// tell DUECA you are prepared
bool VisualTestDrive::isPrepared()
{
  bool res = true;

  // Example checking a token:
  CHECK_TOKEN(w_ego);

  for (list<MotionSet>::iterator ii = motion_sets.begin();
       ii != motion_sets.end(); ii++) {
    CHECK_TOKEN(*(ii->w_entity));
  }

  for (auto const &fgt : fg_sets) {
    CHECK_TOKEN(*(fgt.w_entry));
  }

  // return result of checks
  return res;
}

// start the module
void VisualTestDrive::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void VisualTestDrive::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// fill a snapshot with state data. You may remove this method (and the
// declaration) if you specified to the SimulationModule that the size of
// state snapshots is zero
void VisualTestDrive::fillSnapshot(const TimeSpec &ts, Snapshot &snap,
                                   bool from_trim)
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
void VisualTestDrive::loadSnapshot(const TimeSpec &t, const Snapshot &snap)
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
void VisualTestDrive::doCalculation(const TimeSpec &ts)
{
  // cout << ts << endl;
  //  check the state we are supposed to be in
  switch (getAndCheckState(ts)) {
  case SimulationState::HoldCurrent: {
    // only repeat the output, do not change the model state
    ego.moving = ego.initial;
    ego.moving.dt = 0.0;

    for (list<MotionSet>::iterator ii = motion_sets.begin();
         ii != motion_sets.end(); ii++) {
      ii->moving = ii->initial;
      ii->moving.dt = 0.0;
    }

    for (auto &fgt : fg_sets) {
      fgt.advance(ts, false);
    }

    break;
  }

  case SimulationState::Replay:
  case SimulationState::Advance: {

    ego.moving.dt = ego.initial.dt;
    ego.moving.extrapolate(ts.getDtInSeconds());

    for (list<MotionSet>::iterator ii = motion_sets.begin();
         ii != motion_sets.end(); ii++) {
      ii->moving.dt = ii->initial.dt;
      ii->moving.extrapolate(ts.getDtInSeconds());
    }

    for (auto &fgt : fg_sets) {
      fgt.advance(ts, true);
    }
#if 0
    cout << "xyz "
	 << ego.xyz[0] << ' ' << ego.xyz[1] << ' ' << ego.xyz[2]
	 << " ptp "
	 << ego.getPhi() << ' ' << ego.getTht() << ' ' << ego.getPsi() << endl;
#endif

    break;
  }
  default:
    // other states should never be entered for a SimulationModule,
    // HardwareModules on the other hand have more states. Throw an
    // exception if we get here,
    throw CannotHandleState(getId(), GlobalId(), "state unhandled");
  }

  DataWriter<ObjectMotion> w(w_ego, ts);
  w.data() = ego.moving;

  for (list<MotionSet>::iterator ii = motion_sets.begin();
       ii != motion_sets.end(); ii++) {
    DataWriter<ObjectMotion> w(*(ii->w_entity), ts);
    w.data() = ii->moving;
  }

  // DUECA applications are data-driven. From the time a module is switched
  // on, it should produce data, so that modules "downstreams" are
  // activated
  // access your output channel(s)
  // example
  // StreamWriter<MyOutput> y(output_token, ts);

  // write the output into the output channel, using the stream writer
  // y.data().var1 = something; ...

  if (snapshotNow()) {
    // keep a copy of the model state. Snapshot sending is done in the
    // sendSnapshot routine, later, and possibly at lower priority
    // e.g.
    // snapshot_state_variable1 = state_variable1; ...
    // (or maybe if your state is very large, there is a cleverer way ...)
  }
}

void VisualTestDrive::trimCalculation(const TimeSpec &ts, const TrimMode &mode)
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
  switch (mode) {
  case FlightPath: {
    // one type of trim calculation, find a power setting and attitude
    // belonging to a flight path angle and speed
  } break;

  case Speed: {
    // find a flightpath belonging to a speed and power setting (also
    // nice for gliders)
  } break;

  case Ground: {
    // find an altitude/attitude belonging to standing still on the
    // ground, power/speed 0
  } break;

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
static TypeCreator<VisualTestDrive> a(VisualTestDrive::getMyParameterTable());
