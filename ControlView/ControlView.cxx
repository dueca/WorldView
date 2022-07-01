/* ------------------------------------------------------------------   */
/*      item            : ControlView.cxx
        made by         : rvanpaassen
	from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Fri Jan 29 13:51:31 2010
	category        : body file 
        description     : 
	changes         : Fri Jan 29 13:51:31 2010 first version
	template changes: 030401 RvP Added template creation comment
	                  060512 RvP Modified token checking code
        language        : C++
*/

static const char c_id[] =
"$Id: ControlView.cxx,v 1.11 2020/02/24 19:01:22 rvanpaassen Exp $";

#define ControlView_cxx
// include the definition of the module class
#include "ControlView.hxx"
#include "../comm-objects/RvPQuat.hxx"

// include the debug writing header, by default, write warning and 
// error messages
#define W_MOD
#define E_MOD
#define I_MOD
#include <debug.h>
#include <cstring>

// include additional files needed for your calculation here

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dueca.h>

// class/module name
const char* const ControlView::classname = "control-view";

// Parameters to be inserted
const ParameterTable* ControlView::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing", 
      new MemberCall<ControlView,TimeSpec>
        (&ControlView::setTimeSpec), set_timing_description },

    { "check-timing", 
      new MemberCall<ControlView,vector<int> >
      (&ControlView::checkTiming), check_timing_description },

    { "position", 
      new MemberCall<ControlView,vector<float> >
      (&ControlView::setPosition),
      "Set initial position" },
    
    { "orientation", 
      new MemberCall<ControlView,vector<float> >
      (&ControlView::setOrientation),
      "Set initial orientation" },

    { "viewer-event-reader",
      new MemberCall<ControlView,vector<string> >
      (&ControlView::addWindowEventReader), 
      "Add reading a channel for the window interaction events. \n"
      "supply two strings for the entity and part name" },

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
ControlView::ControlView(Entity* e, const char* part, const
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
  Module(e, classname, part),

  // initialize the data you need in your simulation

  // initialize the data you need for the trim calculation

  // initialize the channel access tokens
  w_entity(getId(), NameSet(getEntity(), "ObjectMotion", part)),
  w_config(getId(), NameSet(getEntity(), "WorldViewConfig", part), 
	   ChannelDistribution::JOIN_MASTER),
  // activity initialization
  cb1(this, &ControlView::doCalculation),
  do_calc(getId(), "new view", &cb1, ps)
{
  // do the actions you need for the simulation
  memset(current, 0, sizeof(current));

  // connect the triggers for simulation
  do_calc.setTrigger(myclock);
}

static GladeCallbackTable controls[] = {
  { "position1", "value_changed", 
    gtk_callback(&ControlView::changePositionOrientation), gpointer(0) },
  { "position2", "value_changed", 
    gtk_callback(&ControlView::changePositionOrientation), gpointer(1) },
  { "position3", "value_changed", 
    gtk_callback(&ControlView::changePositionOrientation), gpointer(2) },
  { "orientation1", "value_changed", 
    gtk_callback(&ControlView::changePositionOrientation), gpointer(3) },
  { "orientation2", "value_changed", 
    gtk_callback(&ControlView::changePositionOrientation), gpointer(4) },
  { "orientation3", "value_changed", 
    gtk_callback(&ControlView::changePositionOrientation), gpointer(5) },
  { NULL, NULL, NULL, NULL }
};

bool ControlView::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  object.name = getEntity();
  I_MOD(classname<<": opening viewcontrol interface");
#if GTK_CHECK_VERSION(3,0,0)
  viewcontrol.readGladeFile
    ("../../../../WorldView/ControlView/viewcontrol-gtk3.ui",
     "viewcontrol", this, controls);
#elif GTK_CHECK_VERSION(2,0,0)
  viewcontrol.readGladeFile
    ("../../../../WorldView/ControlView/viewcontrol.glade",
     "viewcontrol", this, controls);
#else
#error "No suitable GTK version found"
#endif
  viewcontrol.show();

  return true;
}

// destructor
ControlView::~ControlView()
{
  //
}

// as an example, the setTimeSpec function
bool ControlView::setTimeSpec(const TimeSpec& ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0) return false;

  // specify the timespec to the activity
  myclock.changePeriodAndOffset(ts);
  //  do_calc.setTimeSpec(ts);

  // do whatever else you need to process this in your model
  // hint: ts.getDtInSeconds()

  // return true if everything is acceptable
  return true;
}

bool ControlView::setPosition(const std::vector<float>& x)
{
  if (x.size() != 3) {
    E_CNF("setPosition needs 3 parameters");
    return false;
  }
  copy(x.begin(), x.end(), current);
  return true;
}

bool ControlView::setOrientation(const std::vector<float>& x)
{
  if (x.size() != 3) {
    E_CNF("setOrientation needs 3 parameters");
    return false;
  }
  copy(x.begin(), x.end(), &current[3]);
  return true;
}

// and the checkTiming function
bool ControlView::checkTiming(const vector<int>& i)
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

ControlView::WindowEventSet::WindowEventSet(const GlobalId& master_id, 
					    std::vector<std::string> names) :
  name(names)
{
  name = names;
  r_event = new EventChannelReadToken<WorldViewerEvent> 
    (master_id, NameSet(names[0].c_str(), "WorldViewerEvent", 
			names[1].c_str()));
}

bool ControlView::addWindowEventReader(const std::vector<std::string>& n)
{
  if (n.size() != 2) {
    E_CNF("Need 2 strings as argument; entity, part");
    return false;
  }

  wvwindow_events.push_back(WindowEventSet(getId(), n));
  return true;
}

// tell DUECA you are prepared
bool ControlView::isPrepared()
{
  bool res = true;
  
  CHECK_TOKEN(w_entity);
  CHECK_TOKEN(w_config);
  
  for (list<WindowEventSet>::iterator ii = wvwindow_events.begin();
       ii != wvwindow_events.end(); ii++) {
    CHECK_TOKEN(*(ii->r_event));
  }

  // return result of checks
  return res;
}

// start the module
void ControlView::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(viewcontrol["position1"]),
			    current[0]);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(viewcontrol["position2"]),
			    current[1]);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(viewcontrol["position3"]),
			    current[2]);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(viewcontrol["orientation1"]),
			    current[3]);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(viewcontrol["orientation2"]),
			    current[4]);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(viewcontrol["orientation3"]),
			    current[5]);
}

// stop the module
void ControlView::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// this routine contains the main simulation process of your module. You 
// should read the input channels here, and calculate and write the 
// appropriate output
void ControlView::doCalculation(const TimeSpec& ts)
{
  for (list<WindowEventSet>::iterator ii = wvwindow_events.begin();
       ii != wvwindow_events.end(); ii++) {
    
    while (ii->r_event->getNumWaitingEvents(ts)) {
      EventReader<WorldViewerEvent> r(*(ii->r_event), ts);
      
      cout << ii->name[0] << '/' << ii->name[1]  
	   << r.data() << endl;
    }
  }
  
  StreamWriter<ObjectMotion> w(w_entity, ts);
  w.data() = object;
}

#if GTK_MAJOR_VERSION > 2
#define gtk_spin_button_get_value_as_float gtk_spin_button_get_value
#endif


// Make a TypeCreator object for this module, the TypeCreator
// will check in with the scheme-interpreting code, and enable the
// creation of modules of this type
static TypeCreator<ControlView> a(ControlView::getMyParameterTable());

#if GTK_MAJOR_VERSION >= 3
#define gtk_spin_button_get_value_as_float gtk_spin_button_get_value
#endif

void ControlView::changePositionOrientation(GtkSpinButton *widget,
					    gpointer udata)
{
  union {
    gpointer udata;
    long     idx;
  } conv = {udata};
  current[conv.idx] = gtk_spin_button_get_value_as_float(widget);

  if (w_entity.isValid()) {
    object.setquat(current[3]*M_PI/180.0, 
		   current[4]*M_PI/180.0, 
		   current[5]*M_PI/180.0);
    object.xyz[0] = current[0];
    object.xyz[1] = current[1];
    object.xyz[2] = current[2];

    
  }
}
