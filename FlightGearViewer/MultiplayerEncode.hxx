/* ------------------------------------------------------------------   */
/*      item            : MultiplayerEncode.hxx
        made by         : Rene van Paassen
        date            : 100623
	category        : header file 
        description     : 
	changes         : 100623 first version
        language        : C++
*/

#ifndef MultiplayerEncode_hxx
#define MultiplayerEncode_hxx

#include <rpc/types.h>
#include <rpc/xdr.h>
#include "comm-objects.h"
#include "AxisTransform.hxx"
#include <TimeSpec.hxx>

class MultiplayerEncode
{
protected:
  /** xdr struct for the data */
  XDR xdr_data;

  /** xdr struct for the header */
  XDR xdr_header;

  /** Message buffer */
  char buffer[1200];

  /** IPv4 receiver address */
  unsigned receiver;

  /** reply port */
  unsigned replyport;

  /** Axis for conversion */
  const FGAxis& axis;

public:
  /** Constructor */
  MultiplayerEncode(const std::string& receiver, unsigned short replyport, 
		    const FGAxis &ax);

  /** Destructor */
  virtual ~MultiplayerEncode();

  /** return the current buffer */
  virtual const char* getBuffer() const {return buffer;}

  /** get the current buffer fill level */
  virtual unsigned getBufferSize();

  /** Encode an aircraft into this */
  virtual void encode(const BaseObjectMotion& motion,
		      const std::string& fgclass,
		      const std::string& livery,
                      const std::string& name,
		      double time, double lag);
};

#endif
