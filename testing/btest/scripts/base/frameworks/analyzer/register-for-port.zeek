#
# @TEST-EXEC: zeek -b -r ${TRACES}/ssh/ssh-on-port-80.trace %INPUT dpd_buffer_size=0 base/protocols/conn base/protocols/ssh base/frameworks/dpd
# @TEST-EXEC: cat conn.log | zeek-cut service | grep -q ssh
#
# @TEST-EXEC: zeek -b -r ${TRACES}/ssh/ssh-on-port-80.trace dpd_buffer_size=0 base/protocols/conn base/protocols/ssh base/frameworks/dpd
# @TEST-EXEC: cat conn.log | zeek-cut service | grep -vq ssh

event zeek_init()
	{
	Analyzer::register_for_port(Analyzer::ANALYZER_SSH, 80/tcp);
	}


