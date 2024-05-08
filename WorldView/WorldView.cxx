/* ------------------------------------------------------------------   */
/*      item            : WorldView.cxx
        made by         : rvanpaassen
        from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Fri May 15 15:33:50 2009
        category        : body file
        description     :
        changes         : Fri May 15 15:33:50 2009 first version
                          180902 RvP modernize to DUECA 2
        template changes: 030401 RvP Added template creation comment
        060512 RvP Modified token checking code
        language        : C++
*/

#include "ChannelDef.hxx"
#include "ChannelReadToken.hxx"
#include "CommObjectTraits.hxx"
#include "DataReader.hxx"
#include "SimulationState.hxx"
#define WorldView_cxx
// include the definition of the module class
#include "WorldView.hxx"
#include <dueca-version.h>
// include the debug writing header, by default, write warning and
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here
#include "WorldViewerBase.hxx"
#include <algorithm>
// the standard package for DUSIME, including template source
#include <Ticker.hxx>
#define DO_INSTANTIATE
#include <Entity.hxx>
#include <MemberCall2Way.hxx>
#include <dusime.h>

// X is a quite vicious header. include last
#include <X11/Xlib.h>

// class/module name
const char *const WorldView::classname = "world-view";

// Parameters to be inserted
const ParameterTable *WorldView::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing",
      new MemberCall<_ThisModule_, TimeSpec>(&_ThisModule_::setTimeSpec),
      set_timing_description },

    { "check-timing",
      new MemberCall<_ThisModule_, vector<int>>(&_ThisModule_::checkTiming),
      check_timing_description },

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string).

       Note that for efficiency, set_timing_description and
       check_timing_description are pointers to pre-defined strings,
       you can simply enter the descriptive strings in the table. */
    { "claim-thread",
      new VarProbe<_ThisModule_, bool>(&_ThisModule_::claim_thread),
      "occupy a thread and wait for gl swap" },

    { "restore-context",
      new VarProbe<_ThisModule_, bool>(&_ThisModule_::restore_context),
      "force a restore of the graphics context before drawing" },

    { "predict-dt",
      new VarProbe<_ThisModule_, double>(&_ThisModule_::t_predict),
      "prediction time span for the data extrapolation" },

    { "predict-dt-max",
      new VarProbe<_ThisModule_, double>(&_ThisModule_::max_predict),
      "maximum prediction to be attempted on old data" },

    { "initial-camera",
      new MemberCall<_ThisModule_, vector<float>>(&_ThisModule_::initialCamera),
      "Set initial camera position and orientation" },

    { "set-viewer",
      new MemberCall2Way<_ThisModule_, ScriptCreatable>(
        &_ThisModule_::setViewer),
      "add the toolkit-specific viewer to this module" },

    { "set-window-events-writing",
      new MemberCall<_ThisModule_, vector<string>>(
        &_ThisModule_::setWindowEventWriting),
      "Add event checking and writing to one of the windows. Only allowed\n"
      "when not claiming the thread. Arguments are strings; <window name>\n"
      "<entity> <class> <part>: if not given, use default\n"
      "<event type>: the event type arguments are optional, and can be\n"
      "\"keypress\", \"keyrelease\", \"buttonpress\", \"buttonrelease\",\n"
      " or \"motion\". If no event type argument is given, all events are\n"
      "generated" },

    { "config-per-cycle",
      new VarProbe<_ThisModule_, int>(&_ThisModule_::num_config_per_round),
      "For online configuration, number of configuration commands per\n"
      "draw cycle" },

    { "ego-motion-channel",
      new MemberCall<_ThisModule_, std::string>(
        &_ThisModule_::setEgoMotionChannel),
      "Override channel name for reading observer's motion" },

    { "add-world-information-channel",
      new MemberCall<_ThisModule_, std::vector<std::string>>(
        &_ThisModule_::addWorldInformationChannel),
      "Add one or more world info channels. If not used, only the default\n"
      "ObjectMotion://world channel is monitored, if used, and you need this\n"
      "channel, explicitly add it" },

    { "follow-dusime",
      new MemberCall<_ThisModule_, bool>(&_ThisModule_::followDUSIME),
      "Follow the DUSIME state changes, and inform viewers of run/hold" },

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL,
      "Out-of-the-window view presentation, possibly with multiple windows\n"
      "and views" }
  };

  return parameter_table;
}

// constructor
WorldView::WorldView(Entity *e, const char *part, const PrioritySpec &ps) :
  /* The following line initialises the SimulationModule base class.
     You always pass the pointer to the entity, give the classname and the
     part arguments.
     If you give a NULL pointer instead of the inco table, you will not be
     called for trim condition calculations, which is normal if you for
     example implement logging or a display.
     If you give 0 for the snapshot state, you will not be called to
     fill a snapshot, or to restore your state from a snapshot. Only
     applicable if you have no state. */
  Module(e, classname, part),

  // initialize the data you need in your simulation
  rendersurface(NULL),
  claim_thread(false),
  restore_context(false),
  do_init(true),
  run_until(0),
  max_predict(1.0),
  t_predict(0.01),
  num_config_per_round(1),
  freeze(false),

  // initialize the channel access tokens
  r_own(),
  m_others(),
  no_explicit_entity_watch(true),
  r_config(getId(), NameSet(getEntity(), "WorldViewConfig", ""),
           "WorldViewConfig", entry_any, Channel::Events,
           Channel::ZeroOrMoreEntries, Channel::ReadAllData),
  r_dusime(),

  // activity initialization
  cb1(this, &_ThisModule_::doCalculation),
  do_calc(getId(), "update view", &cb1, ps),
  myclock(),
  myalarm()
{
  // do the actions you need for the simulation
}

bool WorldView::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */

  // these are incompatible
  if (claim_thread && w_events.size()) {
    E_MOD("Cannot get key or pointer events when claiming thread");
    return false;
  }

  // connect the triggers for simulation
  if (claim_thread) {
    do_calc.setTrigger(myalarm);
    do_calc.switchOn(0);
  }
  else {
    restore_context = true;
    do_calc.setTrigger(myclock);
  }

  if (!rendersurface) {
    E_MOD("this module needs a render surface");
    return false;
  }

  if (!r_own) {
    // default channel opening
    r_own.reset(new ChannelReadToken(
      getId(), NameSet(getEntity(), "ObjectMotion", ""), "BaseObjectMotion", 0,
      Channel::Continuous, Channel::OnlyOneEntry, Channel::JumpToMatchTime,
      0.2));
  }

  // default case
  if (no_explicit_entity_watch) {
    m_others.push_back(boost::shared_ptr<ChannelWatcher>(
      new ChannelWatcher(NameSet("ObjectMotion://world"), true)));
  }

  return true;
}

// destructor
WorldView::~WorldView()
{
  //
}

// as an example, the setTimeSpec function
bool WorldView::setTimeSpec(const TimeSpec &ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0)
    return false;

  // specify the timespec to the activity
  myclock.changePeriodAndOffset(ts);

  // return true if everything is acceptable
  return true;
}

// and the checkTiming function
bool WorldView::checkTiming(const vector<int> &i)
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

bool WorldView::initialCamera(const vector<float> &i)
{
  if (i.size() != 3 && i.size() != 6 && i.size() != 9 && i.size() != 12) {
    E_CNF("Need 3 or 6 parameters for initial cam pos");
    return false;
  }
  if (i.size() >= 3)
    copy(i.begin(), i.begin() + 3, current_view.xyz.begin());
  if (i.size() >= 6)
    current_view.setquat(i[3], i[4], i[5]);
  if (i.size() >= 9)
    copy(i.begin() + 6, i.begin() + 9, current_view.uvw.begin());
  if (i.size() == 12)
    copy(i.begin() + 9, i.begin() + 12, current_view.omega.begin());
  return true;
}

bool WorldView::setViewer(ScriptCreatable &obj, bool in)
{
  // check direction
  if (!in)
    return false;

  // try a dynamic cast
  WorldViewerBase *viewer = dynamic_cast<WorldViewerBase *>(&obj);
  if (viewer == NULL) {
    E_CNF("must supply a WorldViewerBase compatible object");
    return false;
  }
  if (rendersurface != NULL) {
    E_CNF("2nd viewer supplied, ignoring");
    return false;
  }

#if DUECA_VERSION_NUM < DUECA_VERSION(2, 3, 0)
  // now make sure Scheme does not clean this helper from right
  // under our noses:
  getMyEntity()->scheme_id.addReferred(obj.scheme_id.getSCM());
#endif

  rendersurface = viewer;
  return true;
}

struct EventMapper
{
  unsigned event;
  const char *name;

  bool operator==(const EventMapper &o) const
  {
    return strcmp(this->name, o.name) == 0;
  }
};

bool WorldView::setWindowEventWriting(const vector<std::string> &s)
{
  EventFeedback efb;
  unsigned evmask = KeyReleaseMask | ButtonPressMask;

  if (s.size() < 1 || !rendersurface ||
      rendersurface->setEventMask(s[0], evmask) == false) {
    E_CNF("Need name of window");
    return false;
  }

  if (s.size() < 4) {
    E_CNF("Need three strings for entity/class/part");
    return false;
  }
  efb.winname = s[0];
  if (s.size() == 1) {
    efb.w_event.reset(new ChannelWriteToken(
      getId(), NameSet(getEntity(), "WorldViewerEvent", ""), "WorldViewerEvent",
      s[0], Channel::Events));
  }
  else {
    efb.w_event.reset(new ChannelWriteToken(
      getId(), NameSet(s[1].c_str(), s[2].c_str(), s[3].c_str()),
      "WorldViewerEvent", s[0], Channel::Events));
  }

  if (s.size() > 4) {
    evmask = 0;
    for (unsigned ii = 4; ii < s.size(); ii++) {
      static EventMapper evmap[7] = { { KeyPressMask, "keypress" },
                                      { KeyReleaseMask, "keyrelease" },
                                      { ButtonPressMask, "buttonpress" },
                                      { ButtonReleaseMask, "buttonrelease" },
                                      { PointerMotionMask, "move" },
                                      { ButtonMotionMask, "drag" },
                                      { 0, "" } };
      EventMapper test = { 0, s[ii].c_str() };
      EventMapper *found = find(&evmap[0], &evmap[6], test);
      if (!found->event)
        W_CNF("Cannot find event type " << s[ii]);
      evmask |= found->event;
    }
    rendersurface->setEventMask(s[0], evmask);
  }
  w_events.push_back(efb);

  return true;
}

bool WorldView::setEgoMotionChannel(const std::string &n)
{
  r_own.reset(new ChannelReadToken(getId(), NameSet(n), "BaseObjectMotion", 0,
                                   Channel::Continuous, Channel::OnlyOneEntry,
                                   Channel::JumpToMatchTime, 0.2));
  return true;
}

// tell DUECA you are prepared
bool WorldView::isPrepared()
{
  bool res = true;

  CHECK_TOKEN(*r_own);
  if (r_dusime)
    CHECK_TOKEN(*r_dusime);
  //  CHECK_TOKEN(r_config);

  // return result of checks
  return res;
}

// start the module
void WorldView::startModule(const TimeSpec &time)
{
  if (claim_thread) {
    run_until = MAX_TIMETICK;
    myalarm.requestAlarm(time.getValidityStart());
  }
  else {
    do_calc.switchOn(time);
  }
}

// stop the module
void WorldView::stopModule(const TimeSpec &time)
{
  if (claim_thread) {
    run_until = time.getValidityEnd();
  }
  else {
    do_calc.switchOff(time);
  }
}

// this routine contains the main simulation process of your module. You
// should read the input channels here, and calculate and write the
// appropriate output
void WorldView::doCalculation(const TimeSpec &ts)
{
  TimeTickType current_tick;

  if (do_init) {
    // initialize / open the window
    rendersurface->init(claim_thread);
    do_init = false;
  }

  do {

    // if other GL windows use this thread, the context might be clobbered
    if (!restore_context) {
      rendersurface->makeContextCurrent();
    }

    // any configuration events?
    for (int cc = num_config_per_round;
         cc-- && r_config.isValid() && r_config.getNumVisibleSets();) {
      DataReader<WorldViewConfig, VirtualJoin> c(r_config);

      // let the viewer process this
      rendersurface->adaptSceneGraph(c.data());
    }

    // updates in the simulation state?
    if (r_dusime && r_dusime->haveVisibleSets(current_tick)) {
      DataReader<SimulationState> rs(*r_dusime, current_tick);
      freeze = !(rs.data().t == SimulationState::Advance ||
                 rs.data().t == SimulationState::Replay);
    }

    // check addition and removal of other entities
    ChannelEntryInfo ei;
    for (auto &wchannel : m_others) {
      while (wchannel->checkChange(ei)) {
        if (ei.created) {
          // new entry
          rendersurface->addControllable(
            getId(), wchannel->getChannelName(), ei.entry_id, ei.creation_id,
            ei.data_class, ei.entry_label, ei.time_aspect);
        }
        else {
          // remove an entry
          rendersurface->removeControllable(wchannel->getChannelName(),
                                            ei.creation_id);
        }
      }
    }

    // if we claim a thread, wait for the swap
    if (claim_thread) {
      do_calc.logBlockingWait();
      rendersurface->waitSwap();
      do_calc.logBlockingWaitOver();
    }

    // read current position
    current_tick = SimTime::getTimeTick();
    int64_t frac = Ticker::single()->getUsecsSinceTick(current_tick);
    double late = 0.0;

    // read the viewpoint motion
    try {
      DataReader<BaseObjectMotion, MatchIntervalStartOrEarlier> r(*r_own);
      DataTimeSpec span(r.timeSpec().getValidityStart(), current_tick);
      late = span.getDtInSeconds() + frac * 1e-6 + t_predict;
      current_view = r.data();
      if (late > 0.0 && max_predict > 0.0) {
        current_view.extrapolate(min(late, max_predict));
      }

      D_MOD(classname << "tick=" << current_tick << " frac=" << frac
                      << " late:" << late);
    }
    catch (const NoDataAvailable &e) {
      // expected, no data yet
    }
    catch (const exception &e) {
      W_MOD(classname << ": caught exception " << e.what());
    }

    // put current viewpoint in place
    rendersurface->setBase(current_tick, current_view, late, freeze);

    if (!claim_thread && do_calc.numScheduledBehind()) {

      // early return, don't redraw
      if (do_calc.getCheck() != NULL) {
        do_calc.getCheck()->userReportsAnomaly();
      }
      return;
    }

    // redraw the scene
    rendersurface->redraw(claim_thread);

    // now check if there are events on the surface, and send these
    for (list<EventFeedback>::iterator ii = w_events.begin();
         ii != w_events.end(); ii++) {
      WorldViewerEvent ev;
      ev = rendersurface->getNextEvent(ii->winname);
      while (ev.type != WorldViewerEvent::WVNone && ii->w_event->isValid()) {
        DataWriter<WorldViewerEvent> w(*(ii->w_event), ts);
        w.data() = ev;
        ev = rendersurface->getNextEvent(ii->winname);
      }
    }
  }
  while (run_until >= SimTime::getTimeTick());
}

bool WorldView::addWorldInformationChannel(const std::vector<std::string> &ch)
{
  no_explicit_entity_watch = false;
  for (auto &chn : ch) {
    m_others.push_back(boost::shared_ptr<ChannelWatcher>(
      new ChannelWatcher(NameSet(chn), true)));
  }
  return true;
}

bool WorldView::followDUSIME(const bool &b)
{
  if (b) {
    r_dusime.reset(new ChannelReadToken(
      getId(), NameSet("dusime", getclassname<SimulationState>(), ""),
      getclassname<SimulationState>(), 0, Channel::Events));
    freeze = true;
  }
  else {
    r_dusime.reset();
    freeze = false;
  }
  return true;
}

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the scheme-interpreting code, and enable the
// creation of modules of this type
static TypeCreator<WorldView> a(WorldView::getMyParameterTable());
