

#ifndef LORAWAN_ENDDEVICE_HELPER_H
#define LORAWAN_ENDDEVICE_HELPER_H

#include <string>

#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/net-device.h"
#include "ns3/application-container.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/lorawan-enddevice-application.h"

namespace ns3 {

class LoRaWANEndDeviceHelper 
{
public:
	LoRaWANEndDeviceHelper();

	void SetAttribute (std::string name, const AttributeValue &value);

	ApplicationContainer Install (Ptr<Node> node) const;

	ApplicationContainer Install (std::string nodeName) const;

	ApplicationContainer Install (NodeContainer c) const;

private:
	Ptr<Application> InstallPriv (Ptr<Node> node) const;

	ObjectFactory m_factory;
};

} // namespace ns3

#endif /* LORAWAN_ENDDEVICE_HELPER_H */