#include "../FlightGearViewer/MultiplayerEncode.hxx"

size_t FGBaseAircraftMotion::encodeProperties(XDR &xdr_data) const
{
  MultiplayerEncode::propertyEncode(xdr_data, 200, gear_compression_norm[0]);
  MultiplayerEncode::propertyEncode(xdr_data, 201, gear_extension_norm[0]);
  MultiplayerEncode::propertyEncode(xdr_data, 210, gear_compression_norm[1]);
  MultiplayerEncode::propertyEncode(xdr_data, 211, gear_extension_norm[1]);
  MultiplayerEncode::propertyEncode(xdr_data, 220, gear_compression_norm[2]);
  MultiplayerEncode::propertyEncode(xdr_data, 221, gear_extension_norm[2]);

  MultiplayerEncode::propertyEncode(xdr_data, 100, left_aileron_norm);
  MultiplayerEncode::propertyEncode(xdr_data, 101, right_aileron_norm);
  MultiplayerEncode::propertyEncode(xdr_data, 102, elevator_norm);
  MultiplayerEncode::propertyEncode(xdr_data, 103, rudder_norm);
  MultiplayerEncode::propertyEncode(xdr_data, 104, flaps_norm);
  MultiplayerEncode::propertyEncode(xdr_data, 105, speedbrake_norm);
  return 12;
}
