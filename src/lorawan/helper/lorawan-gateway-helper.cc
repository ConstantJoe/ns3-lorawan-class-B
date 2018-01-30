
#include "ns3/names.h"
#include "lorawan-gateway-helper.h"

namespace ns3 {

LoRaWANGatewayHelper::LoRaWANGatewayHelper()
{
	m_factory.SetTypeId ("ns3::LoRaWANGatewayApplication");
}

void
LoRaWANGatewayHelper::SetAttribute (std::string name, const AttributeValue &value)
{
	m_factory.Set (name, value);
}

ApplicationContainer 
LoRaWANGatewayHelper::Install (Ptr<Node> node) const
{
	return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LoRaWANGatewayHelper::Install (std::string nodeName) const
{
	Ptr<Node> node = Names::Find<Node> (nodeName);
	return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LoRaWANGatewayHelper::Install (NodeContainer c) const
{
	ApplicationContainer apps;
	for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
	{
		apps.Add (InstallPriv (*i));
	}

	return apps;
}

Ptr<Application>
LoRaWANGatewayHelper::InstallPriv (Ptr<Node> node) const
{
	Ptr<Application> app = m_factory.Create<Application> ();
	node->AddApplication (app);

	return app;
}

} // namespace ns3