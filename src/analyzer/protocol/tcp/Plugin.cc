// See the file  in the main distribution directory for copyright.

#include "zeek/plugin/Plugin.h"
#include "zeek/analyzer/Component.h"
#include "zeek/analyzer/protocol/tcp/TCP.h"
#include "zeek/analyzer/protocol/tcp/TCP_Father.h"

namespace zeek::plugin::detail::Zeek_TCP {

class Plugin : public zeek::plugin::Plugin {
public:
	zeek::plugin::Configuration Configure() override
		{
		AddComponent(new zeek::analyzer::Component("TCPFather", zeek::analyzer::tcp::TCP_FatherAnalyzer::Instantiate));
		AddComponent(new zeek::analyzer::Component("TCP", zeek::analyzer::tcp::TCP_Analyzer::Instantiate));
		AddComponent(new zeek::analyzer::Component("TCPStats", zeek::analyzer::tcp::TCPStats_Analyzer::Instantiate));
		AddComponent(new zeek::analyzer::Component("CONTENTLINE", nullptr));
		AddComponent(new zeek::analyzer::Component("Contents", nullptr));

		zeek::plugin::Configuration config;
		config.name = "Zeek::TCP";
		config.description = "TCP analyzer";
		return config;
		}
} plugin;

} // namespace zeek::plugin::detail::Zeek_TCP
