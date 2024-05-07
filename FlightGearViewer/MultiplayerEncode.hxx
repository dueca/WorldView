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
#include <rapidjson/document.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <string>

/** Encoder for FlightGear multiplayer messages.

    Creates a buffer for encoding xdr-based multiplayer message
*/
class MultiplayerEncode
{
public:
  /** base class for encoding additional properties. */
  struct PropertyEncoderBase
  {

    virtual size_t operator()(XDR &xdr_data) const { return 0; }
  };

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
                    const FGAxis &ax, float radarrange = 100.0f);

  /** Destructor */
  virtual ~MultiplayerEncode();

  /** return the current buffer */
  virtual const char *getBuffer() const { return buffer; }

  /** get the current buffer fill level */
  virtual inline size_t getBufferSize() const { return bufsize; }

  /** Encode an aircraft into this */
  virtual void encode(const BaseObjectMotion &motion,
                      const std::string &fgclass, const std::string &name, double time, double lag,
                      const PropertyEncoderBase *coder = NULL);

  /** Log print of a received or created buffer. */
  void dump(const char *buffer, size_t bufsize);
};

/** Encode fixed properties available in a JSON file */
class PropertyEncoderJSON : public MultiplayerEncode::PropertyEncoderBase
{
  /** JSON doc with data */
  rapidjson::Document doc;

  /** Remember file name for error reporting. */
  std::string fname;

public:
  /** Contructor

      @param jfile  File with JSON array containing "code", "value" structs.
  */
  PropertyEncoderJSON(const std::string &jfile);

  /** Destructor
   */
  virtual ~PropertyEncoderJSON();

  /** Encode the JSON data into XDR

      @param xdr_data XDR data pointer.
      @returns        Number of coded members.
  */
  virtual size_t operator()(XDR &xdr_data) const override;
};

#endif
