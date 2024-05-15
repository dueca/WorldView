/* ------------------------------------------------------------------   */
/*      item            : FlightGearViewer_Dueca.hxx
        made by         : rvanpaassen
        from template   : DuecaHelperTemplate.hxx
        template made by: Rene van Paassen
        date            : Tue Jan 26 13:49:33 2010
        category        : header file
        description     :
        changes         : Tue Jan 26 13:49:33 2010 first version
        template changes: 050825 RvP Added template creation comment
        language        : C++
*/

#ifndef FlightGearViewer_Dueca_hxx
#define FlightGearViewer_Dueca_hxx

// include the dueca header
#include <ParameterTable.hxx>
#include <ScriptCreatable.hxx>
#include <dueca_ns.h>
#include <stringoptions.h>

#include "FlightGearViewer.hxx"

USING_DUECA_NS;

/** This helper class can communicate position reports to a FlightGear
    viewer.

    Configure FlightGear to use an "external" fdm and read the
    position data from a socket.

    \verbatim
    fgfs --config=emptypanel.xml \
         --generic=socket,dir,hz,machine,port,style,duecavis-ascii \
    \endverbatim
    The variables are:
    <ul>
    <li> emptypanel.xml: a file with generic configuration
    <li> dir: direction of io, must be 'in'
    <li> hz: rate of input
    <li> machine: ip address or machine name
    <li> port: port to be used
    <li> style: must be 'udp' for this communication
    <li> duecavis-ascii: the specified protocol, in a file
    duecavis-ascii.xml. Install this file in /usr/share/FlightGear/Protocol
    another option is duecavis; a binary protocol
    </ul>

    You can add a --fov=?? flag to match the field of view to the fov
    of your set-up

    This class has been derived from the ScriptCreatable base class,
    and has a (scheme) script command to create it and optionally add
    parameters.

    The instructions to create an object of this class from the Scheme
    script are:

    \verbinclude flight-gear-viewer.scm
 */
class FlightGearViewer_Dueca : public ScriptCreatable, public FlightGearViewer
{
private: // simulation data
  // define for internal reference
  typedef FlightGearViewer_Dueca _ThisModule_;

public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  FlightGearViewer_Dueca();

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. */
  bool complete();

  /** Destructor. */
  ~FlightGearViewer_Dueca();

  /** Obtain a pointer to the parameter table. */
  static const ParameterTable *getParameterTable();

public:
  /** Default script linkage. */
  SCM_FEATURES_DEF;
};

#endif
