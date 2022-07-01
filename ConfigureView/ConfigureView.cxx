/* ------------------------------------------------------------------   */
/*      item            : ConfigureView.cxx
        made by         : repa
	from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Fri Jun 11 22:16:57 2010
	category        : body file
        description     :
	changes         : Fri Jun 11 22:16:57 2010 first version
	template changes: 030401 RvP Added template creation comment
	                  060512 RvP Modified token checking code
        language        : C++
*/

#define ConfigureView_cxx
// include the definition of the module class
#include "ConfigureView.hxx"

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
const char* const ConfigureView::classname = "configure-view";

// Parameters to be inserted
const ParameterTable* ConfigureView::getMyParameterTable()
{
	static const ParameterTable parameter_table[] =
	{
		{ "set-timing",
			new MemberCall<ConfigureView,TimeSpec>
			( &ConfigureView::setTimeSpec ), set_timing_description },

		{ "check-timing",
		  new MemberCall<ConfigureView,vector<int> >
		  ( &ConfigureView::checkTiming ), check_timing_description },

		/* You can extend this table with labels and MemberCall or
		   VarProbe pointers to perform calls or insert values into your
		   class objects. Please also add a description (c-style string).

		   Note that for efficiency, set_timing_description and
		   check_timing_description are pointers to pre-defined strings,
		   you can simply enter the descriptive strings in the table. */

		/* The table is closed off with NULL pointers for the variable
		   name and MemberCall/VarProbe object. The description is used to
		   give an overall description of the module. */
		{ NULL, NULL, "please give a description of this module"}
	};

	return parameter_table;
}

// constructor
ConfigureView::ConfigureView ( Entity* e, const char* part, const
                               PrioritySpec& ps ) :
		/* The following line initialises the SimulationModule base class.
		   You always pass the pointer to the entity, give the classname and the
		   part arguments.
		   If you give a NULL pointer instead of the inco table, you will not be
		   called for trim condition calculations, which is normal if you for
		   example implement logging or a display.
		   If you give 0 for the snapshot state, you will not be called to
		   fill a snapshot, or to restore your state from a snapshot. Only
		   applicable if you have no state. */
		Module ( e, classname, part ),

		// initialize the data you need in your simulation
		viewspec(),

		// initialize the data you need for the trim calculation

		// initialize the channel access tokens
		w_config ( getId(), NameSet ( getEntity(), "WorldViewConfig", part ), 
			   ChannelDistribution::NO_OPINION, Regular, NULL ),

		// activity initialization
		cb1 ( this, &ConfigureView::doCalculation ),
		do_calc ( getId(), "dunno", &cb1, ps )
{
	// do the actions you need for the simulation
	config.coordinates.resize ( 9 );
	config.filename.resize ( 1 );

	// connect the triggers for simulation
	// do_calc.setTrigger(/* fill in your triggering channels */);
}

bool ConfigureView::complete()
{
	/* All your parameters have been set. You may do extended
	   initialisation here. Return false if something is wrong. */

	static GladeCallbackTable controls[] =
	{
		{ "_add", "clicked",
			gtk_callback ( &ConfigureView::doAction ),
			gpointer ( WorldViewConfig::LoadObject ) },
		{ "_move", "clicked",
		  gtk_callback ( &ConfigureView::doAction ),
		  gpointer ( 4 ) },
		{ "_delete", "clicked",
		  gtk_callback ( &ConfigureView::doAction ), gpointer ( 1 ) },
		{ "_scene", "clicked",
		  gtk_callback ( &ConfigureView::doAction ), gpointer ( 2 ) },
		{ "_clear", "clicked",
		  gtk_callback ( &ConfigureView::doAction ), gpointer ( 0 ) },
		{ "_overlay", "clicked",
		  gtk_callback ( &ConfigureView::doAction ), gpointer ( 3 ) },
		{ NULL, NULL, NULL, NULL }
	};

#if GTK_CHECK_VERSION(3,0,0)
	viewcontrol.readGladeFile
	  ( "../../../../WorldView/ConfigureView/configureview-gtk3.ui",
	  "configureview", this, controls );
#elif GTK_CHECK_VERSION(2,0,0)
	viewcontrol.readGladeFile
	  ( "../../../../WorldView/ConfigureView/configureview.glade",
	  "configureview", this, controls );
#else
#error "No suitable GTK version found"
#endif
	viewcontrol.show();

	return true;
}

// destructor
ConfigureView::~ConfigureView()
{
	//
}

// as an example, the setTimeSpec function
bool ConfigureView::setTimeSpec ( const TimeSpec& ts )
{
	// a time span of 0 is not acceptable
	if ( ts.getValiditySpan() == 0 ) return false;

	// specify the timespec to the activity
	do_calc.setTimeSpec ( ts );

	// do whatever else you need to process this in your model
	// hint: ts.getDtInSeconds()

	// return true if everything is acceptable
	return true;
}

// and the checkTiming function
bool ConfigureView::checkTiming ( const vector<int>& i )
{
	if ( i.size() == 3 )
	{
		new TimingCheck ( do_calc, i[0], i[1], i[2] );
	}
	else if ( i.size() == 2 )
	{
		new TimingCheck ( do_calc, i[0], i[1] );
	}
	else
	{
		return false;
	}
	return true;
}

// tell DUECA you are prepared
bool ConfigureView::isPrepared()
{
	bool res = true;

	// Example checking a token:
	CHECK_TOKEN ( w_config );

	// Example checking anything
	// CHECK_CONDITION(myfile.good());

	// return result of checks
	return res;
}

// start the module
void ConfigureView::startModule ( const TimeSpec &time )
{
	do_calc.switchOn ( time );
}

// stop the module
void ConfigureView::stopModule ( const TimeSpec &time )
{
	do_calc.switchOff ( time );
}

// this routine contains the main simulation process of your module. You
// should read the input channels here, and calculate and write the
// appropriate output
void ConfigureView::doCalculation ( const TimeSpec& ts )
{
	// access the input
	// example:
	// try {
	//   StreamReader<MyInput> u(input_token, ts);
	//   throttle = u.data().throttle;
	//   de = u.data().de; ....
	// }
	// catch(Exception& e) {
	//   // strange, there is no input. Should I try to continue or not?
	// }
	/* The above piece of code shows a block in which you try to catch
	   error conditions (exceptions) to handle the case in which the input
	   data is lost. This is not always necessary, if you normally do not
	   foresee such a condition, and you don t mind being stopped when
	   it happens, forget about the try/catch blocks. */

	// do the simulation or other calculations, one step

	// DUECA applications are data-driven. From the time a module is switched
	// on, it should produce data, so that modules "downstreams" are
	// activated
	// access your output channel(s)
	// example
	// StreamWriter<MyOutput> y(output_token, ts);

	// write the output into the output channel, using the stream writer
	// y.data().var1 = something; ...
}

#if GTK_MAJOR_VERSION < 3
#define gtk_spin_button_get_value gtk_spin_button_get_value_as_float
#endif

void ConfigureView::doAction ( GtkToolButton* button, gpointer gp )
{
	union { gpointer udata; long idx;} conv = {gp};

	// read out controls
	static const char* spins[] =
	{
		"xpos", "ypos", "zpos",
		"phi", "theta", "psi",
		"scaleX", "scaleY", "scaleZ"
	};
	for ( int ii = 9; ii--; )
	{
		config.coordinates[ii] = gtk_spin_button_get_value
		  ( GTK_SPIN_BUTTON ( viewcontrol[ spins[ii] ] ) );
	}

	// get text
	config.name = gtk_entry_get_text ( GTK_ENTRY ( viewcontrol["object_name"] ) );
	config.type = gtk_entry_get_text ( GTK_ENTRY ( viewcontrol["object_type"] ) );
	config.filename[0] = gtk_entry_get_text ( GTK_ENTRY ( viewcontrol["files"] ) );

	viewspec.winname  = config.name;
	viewspec.name     = config.type;
	viewspec.overlay  = config.filename[0];

	// map command
	WorldViewConfig::ConfigCommand cmd = WorldViewConfig::ConfigCommand
	                                     ( uint8_t ( conv.idx ) );

	// write event
	EventWriter<WorldViewConfig> e ( w_config, TimeSpec ( SimTime::getTimeTick() ) );
	e.data().command = cmd;
	e.data().config = config;
	e.data().viewspecs = viewspec;
}

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the scheme-interpreting code, and enable the
// creation of modules of this type
static TypeCreator<ConfigureView> a ( ConfigureView::getMyParameterTable() );

