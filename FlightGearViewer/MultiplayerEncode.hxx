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

#include "AxisTransform.hxx"
#include "comm-objects.h"
#include <TimeSpec.hxx>
#include <rpc/types.h>
#include <rpc/xdr.h>

class MultiplayerEncode
{
protected:
  /** xdr struct for the data */
  XDR xdr_data;

  /** xdr struct for the header */
  XDR xdr_header;

  /** Message buffer */
  char buffer[1200];

  /** Current buffer size */
  size_t bufsize;

  /** IPv4 receiver address */
  unsigned receiver;

  /** reply port */
  unsigned replyport;

  /** Axis for conversion */
  const FGAxis &axis;

  /** Radar range */
  float radarrange;

public:
  /** Constructor */
  MultiplayerEncode(const std::string &receiver, unsigned short replyport,
                    const FGAxis &ax, float radarrange = 100000.0f);

  /** Destructor */
  virtual ~MultiplayerEncode();

  /** return the current buffer */
  virtual const char *getBuffer() const { return buffer; }

  /** get the current buffer fill level */
  virtual inline size_t getBufferSize() const {return bufsize; }

  /** Encode an aircraft into this */
  virtual void encode(const BaseObjectMotion &motion,
                      const std::string &fgclass, const std::string &livery,
                      const std::string &name, double time, double lag);


  void dump(const char* buffer, size_t bufsize);
};

#endif
