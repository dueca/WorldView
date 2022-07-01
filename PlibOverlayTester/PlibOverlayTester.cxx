/* ------------------------------------------------------------------   */
/*      item            : PlibOverlayTester.cxx
        made by         : rvanpaassen
	from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Fri Mar 19 16:25:26 2010
	category        : body file 
        description     : 
	changes         : Fri Mar 19 16:25:26 2010 first version
	template changes: 030401 RvP Added template creation comment
	                  060512 RvP Modified token checking code
        language        : C++
*/

static const char c_id[] =
"$Id: PlibOverlayTester.cxx,v 1.3 2010/05/14 17:08:04 rvanpaassen Exp $";

#define PlibOverlayTester_cxx
// include the definition of the module class
#include "PlibOverlayTester.hxx"

// only for hmi lib configs
#ifdef DUECA_CONFIG_HMI

// include the debug writing header, by default, write warning and 
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dueca.h>
#include <Entity.hxx>
#include <MemberCall2Way.hxx>

// class/module name
const char* const PlibOverlayTester::classname = "plib-overlay-tester";

// Parameters to be inserted
const ParameterTable* PlibOverlayTester::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing", 
      new MemberCall<PlibOverlayTester,TimeSpec>
        (&PlibOverlayTester::setTimeSpec), set_timing_description },

    { "check-timing", 
      new MemberCall<PlibOverlayTester,vector<int> >
      (&PlibOverlayTester::checkTiming), check_timing_description },

    { "set-overlay", 
      new MemberCall2Way<PlibOverlayTester, ScriptCreatable >
      (&PlibOverlayTester::setOverlay), 
      "specify an overlay to communicate with the display" },

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
PlibOverlayTester::PlibOverlayTester(Entity* e, const char* part, const
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
  overlay(NULL),
  
  // initialize the data you need for the trim calculation

  // activity initialization
  myclock(),
  cb1(this, &PlibOverlayTester::doCalculation),
  do_calc(getId(), "dummy data for overlay", &cb1, ps)
{
  // connect the triggers for simulation
  do_calc.setTrigger(myclock);
}

bool PlibOverlayTester::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  if (!overlay) {
    E_CNF("gimme an overlay!");
    return false;
  }

  return true;
}

// destructor
PlibOverlayTester::~PlibOverlayTester()
{
  //
}

// as an example, the setTimeSpec function
bool PlibOverlayTester::setTimeSpec(const TimeSpec& ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0) return false;

  // specify the timespec to the activity
  myclock.changePeriodAndOffset(ts);

  // return true if everything is acceptable
  return true;
}

// and the checkTiming function
bool PlibOverlayTester::checkTiming(const vector<int>& i)
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

bool PlibOverlayTester::setOverlay(ScriptCreatable &obj, bool in)
{
  // check direction
  if (!in) return false;

  if (overlay) {
    E_CNF("can only specify one overlay");
    return false;
  }

  // try a dynamic cast
  overlay = dynamic_cast<PlibTestOverlay*> (&obj);
  if (overlay == NULL) {
    E_CNF("must supply a PlibTestOverlay for the tester");
    return false;
  }

  // the overlay was created in scheme, and we need to make sure it
  // does not get cleaned up by garbage disposal. 
  getMyEntity()->scheme_id.addReferred(overlay->scheme_id.getSCM());

  return true;
}

// tell DUECA you are prepared
bool PlibOverlayTester::isPrepared()
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
void PlibOverlayTester::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void PlibOverlayTester::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// this routine contains the main simulation process of your module. You 
// should read the input channels here, and calculate and write the 
// appropriate output
void PlibOverlayTester::doCalculation(const TimeSpec& ts)
{
  overlay->nextMessage();
} 

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the scheme-interpreting code, and enable the
// creation of modules of this type
static TypeCreator<PlibOverlayTester> a(PlibOverlayTester::getMyParameterTable());

#endif
