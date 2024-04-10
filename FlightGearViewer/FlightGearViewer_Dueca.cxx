/* ------------------------------------------------------------------   */
/*      item            : FlightGearViewer_Dueca.cxx
        made by         : rvanpaassen
        from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Tue Jan 26 13:49:33 2010
        category        : body file
        description     :
        changes         : Tue Jan 26 13:49:33 2010 first version
        template changes: 030401 RvP Added template creation comment
        language        : C++
*/

static const char c_id[] =
  "$Id: FlightGearViewer_Dueca.cxx,v 1.5 2018/08/01 10:19:54 fltsim Exp $";

#define FlightGearViewer_Dueca_cxx
// include the definition of the helper class
#include "FlightGearViewer_Dueca.hxx"

// include the debug writing header, by default, write warning and
 // error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here

#define DO_INSTANTIATE
#include <CoreCreator.hxx>
#include <MemberCall.hxx>
#include <MemberCall2Way.hxx>
#include <VarProbe.hxx>
USING_DUECA_NS;

// Parameters to be inserted
const ParameterTable *FlightGearViewer_Dueca::getParameterTable()
{
  static const ParameterTable parameter_table[] = {

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string). */
    { "receiver",
      new VarProbe<FlightGearViewer_Dueca, string>(
        &FlightGearViewer_Dueca::receiver),
      "IP address for the flightgear process" },

    { "own_interface",
      new VarProbe<FlightGearViewer_Dueca, string>(
        &FlightGearViewer_Dueca::own_interface),
      "IP address of own interface, empty to bind to all interfaces" },

    { "port",
      new VarProbe<FlightGearViewer_Dueca, int>(&FlightGearViewer_Dueca::port),
      "Port to be used" },

    { "lat-lon-psi0",
      new MemberCall<FlightGearViewer_Dueca, vector<double>>(
        &FlightGearViewer_Dueca::setLatLonPsi0),
      "call with three parameters to set local coordinate system" },

    { "lat-lon-alt-psi0",
      new MemberCall<FlightGearViewer_Dueca, vector<double>>(
        &FlightGearViewer_Dueca::setLatLonPsi0),
      "call with four parameters to set local coordinate system" },

    { "binary-packets",
      new VarProbe<FlightGearViewer_Dueca, bool>(
        &FlightGearViewer_Dueca::binary_packets),
      "Use binary packets for sending and receiving" },

    { "mp-interface",
      new VarProbe<FlightGearViewer_Dueca, string>(
        &FlightGearViewer_Dueca::mp_interface),
      "Interface address to listen on for the multiplayer server" },

    { "mp-port",
      new VarProbe<FlightGearViewer_Dueca, int>(
        &FlightGearViewer_Dueca::mp_port),
      "Port number of the multiplayer server, enables if set" },

    { "mp-radarrange",
      new VarProbe<FlightGearViewer_Dueca, float>(
        &FlightGearViewer_Dueca::mp_radarrange),
      "Radar range/visibility for the multiplayer connection." },

    { "model-table",
      new MemberCall<FlightGearViewer_Dueca, std::vector<std::string>>(
        &FlightGearViewer_Dueca::modelTableEntry),
      "Set the link between simulation class name, and FlightGear, for\n"
      "sending player data to a multiplayer server. Each object needs a name\n"
      "and a model class, and optionally a livery.\n"
      " - matchstring: Normally a string matching data class name and entry\n"
      "   label. Examples:\n"
      "   * \"BaseObjectMotion:PH-ANH\",to match label PH-ANH as name\n"
      "   * \"BaseObjectMotion:c172\", would match label c172|PH-ANH\n"
      "   See also the documentation on SpecificationBase class.\n"
      " - Flightgear model, e.g. \"/Aircraft/c172p/Models/c172p.xml\", see\n"
      "   flightgear data folder\n"
      " - Livery, optional" },

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL,
      "FlightGear interconnection. Can send own aircraft reports to a\n"
      "FlightGear process and other aircraft reports to a multiplayer\n"
      "server" }
  };

  return parameter_table;
}

// constructor
FlightGearViewer_Dueca::FlightGearViewer_Dueca() :
  ScriptCreatable(),
  FlightGearViewer()
{}

bool FlightGearViewer_Dueca::complete() { return FlightGearViewer::complete(); }

// destructor
FlightGearViewer_Dueca::~FlightGearViewer_Dueca()
{
  //
}

// script access macro
SCM_FEATURES_NOIMPINH(FlightGearViewer_Dueca, ScriptCreatable,
                      "flight-gear-viewer");

// Make a CoreCreator object for this module, the CoreCreator
// will check in with the scheme-interpreting code, and enable the
// creation of objects of this type

#ifdef SCRIPT_PYTHON
static CoreCreator<FlightGearViewer_Dueca>
  a(FlightGearViewer_Dueca::getParameterTable(), "FlightGearViewer");
#else
static CoreCreator<FlightGearViewer_Dueca>
  a(FlightGearViewer_Dueca::getParameterTable());
#endif
