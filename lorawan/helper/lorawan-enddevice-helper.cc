
#include "ns3/names.h"
#include "lorawan-enddevice-helper.h"

namespace ns3 {

LoRaWANEndDeviceHelper::LoRaWANEndDeviceHelper (/*Address address*/)
{
	m_factory.SetTypeId ("ns3::LoRaWANEndDeviceApplication");
	//m_factory.Set ("Remote", AddressValue (address));
}

void
LoRaWANEndDeviceHelper::SetAttribute (std::string name, const AttributeValue &value)
{
	m_factory.Set (name, value);
}

ApplicationContainer 
LoRaWANEndDeviceHelper::Install (Ptr<Node> node) const
{
	return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LoRaWANEndDeviceHelper::Install (std::string nodeName) const
{
	Ptr<Node> node = Names::Find<Node> (nodeName);
	return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LoRaWANEndDeviceHelper::Install (NodeContainer c) const
{
	ApplicationContainer apps;
	for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
	{
		apps.Add (InstallPriv (*i));
	}

	return apps;
}

Ptr<Application>
LoRaWANEndDeviceHelper::InstallPriv (Ptr<Node> node) const
{
	Ptr<Application> app = m_factory.Create<Application> ();
	node->AddApplication (app);

	return app;
}

} // namespace ns3