/* ------------------------------------------------------------------   */
/*      item            : FlightGearObjectProp.cxx
        made by         : Rene' van Paassen
        date            : 100707
        category        : body file
        description     :
        changes         : 100707 first version
        language        : C++
*/

#include "FlightGearObjectProp.ixx"
#include "FGObjectFactory.hxx"
#include "comm-objects.h"

static auto FlightGearObjectProp_maker =
  new SubContractor<FGObjectTypeKey, FlightGearObjectProp<FGObjectMotion>>(
    "fgprops", "Moving FlightGear object, position controlled, property added");
