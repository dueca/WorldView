/* ------------------------------------------------------------------   */
/*      item            : ViewpointHack.cxx
        made by         : repa
	from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Wed Sep 11 14:37:24 2013
	category        : body file 
        description     : 
	changes         : Wed Sep 11 14:37:24 2013 first version
	template changes: 030401 RvP Added template creation comment
	                  060512 RvP Modified token checking code
        language        : C++
*/

#define ViewpointHack_cxx
// include the definition of the module class
#include "ViewpointHack.hxx"

// include the debug writing header, by default, write warning and 
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dueca.h>

// class/module name
const char* const ViewpointHack::classname = "viewpoint-hack";

float viewpoint_xyz_pqr[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

// Parameters to be inserted
const ParameterTable* ViewpointHack::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing", 
      new MemberCall<ViewpointHack,TimeSpec>
        (&ViewpointHack::setTimeSpec), set_timing_description },

    { "check-timing", 
      new MemberCall<ViewpointHack,vector<int> >
      (&ViewpointHack::checkTiming), check_timing_description },

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
ViewpointHack::ViewpointHack(Entity* e, const char* part, const
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

  // activity initialization
  myclock(),
  cb1(this, &ViewpointHack::doCalculation),
  do_calc(getId(), "dummy update viewpointhack", &cb1, ps)
{
  // do the actions you need for the simulation

  // connect the triggers for simulation
  do_calc.setTrigger(myclock);
}

static GladeCallbackTable controls[] = {
  { "position1", "value_changed", 
    gtk_callback(&ViewpointHack::changePositionOrientation), gpointer(0) },
  { "position2", "value_changed", 
    gtk_callback(&ViewpointHack::changePositionOrientation), gpointer(1) },
  { "position3", "value_changed", 
    gtk_callback(&ViewpointHack::changePositionOrientation), gpointer(2) },
  { "orientation1", "value_changed", 
    gtk_callback(&ViewpointHack::changePositionOrientation), gpointer(3) },
  { "orientation2", "value_changed", 
    gtk_callback(&ViewpointHack::changePositionOrientation), gpointer(4) },
  { "orientation3", "value_changed", 
    gtk_callback(&ViewpointHack::changePositionOrientation), gpointer(5) },
  { NULL, NULL, NULL, NULL }
};

bool ViewpointHack::complete()
{

#if GTK_CHECK_VERSION(3,0,0)
  viewcontrol.readGladeFile
    ("../../../../WorldView/ViewpointHack/viewcontrol-gtk3.ui",
     "viewcontrol", this, controls);
#elif GTK_CHECK_VERSION(2,0,0)
  // open the interface
  viewcontrol.readGladeFile
    ("../../../../WorldView/ViewpointHack/viewcontrol.glade",
     "viewcontrol", this, controls);
#else
#error "No suitable GTK version found"
#endif
  
  viewcontrol.show();
  return true;
}

// destructor
ViewpointHack::~ViewpointHack()
{
  //
}

// as an example, the setTimeSpec function
bool ViewpointHack::setTimeSpec(const TimeSpec& ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0) return false;

  // specify the timespec to the activity
  do_calc.setTimeSpec(ts);
  // or do this with the clock if you have it (don't do both!)
  // myclock.changePeriodAndOffset(ts);

  // do whatever else you need to process this in your model
  // hint: ts.getDtInSeconds()

  // return true if everything is acceptable
  return true;
}

// and the checkTiming function
bool ViewpointHack::checkTiming(const vector<int>& i)
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
bool ViewpointHack::isPrepared()
{
  bool res = true;

  // Example checking a token:
  // CHECK_TOKEN(w_somedata);

  // Example checking anything
  // CHECK_CONDITION(myfile.good());

  // return result of checks
  return res;
}

// start the module
void ViewpointHack::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void ViewpointHack::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// this routine contains the main simulation process of your module. You 
// should read the input channels here, and calculate and write the 
// appropriate output
void ViewpointHack::doCalculation(const TimeSpec& ts)
{
  //
} 

#if GTK_MAJOR_VERSION < 3
#define gtk_spin_button_get_value gtk_spin_button_get_value_as_float
#endif

void ViewpointHack::changePositionOrientation(GtkSpinButton *widget,
					      gpointer udata)
{
  union {
    gpointer udata;
    long     idx;
  } conv = {udata};
  viewpoint_xyz_pqr[conv.idx] = gtk_spin_button_get_value(widget);
}


// Make a TypeCreator object for this module, the TypeCreator
// will check in with the scheme-interpreting code, and enable the
// creation of modules of this type
static TypeCreator<ViewpointHack> a(ViewpointHack::getMyParameterTable());

