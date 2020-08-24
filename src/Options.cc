// See the file "COPYING" in the main distribution directory for copyright.

#include "zeek-config.h"

#include "Options.h"
#include "ScriptAnaly.h"

#include <algorithm>

#include <unistd.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "bsd-getopt-long.h"
#include "logging/writers/ascii/Ascii.h"

void zeek::Options::filter_supervisor_options()
	{
	pcap_filter = {};
	signature_files = {};
	pcap_output_file = {};
	}

void zeek::Options::filter_supervised_node_options()
	{
	auto og = *this;
	*this = {};

	debug_log_streams = og.debug_log_streams;
	debug_script_tracing_file = og.debug_script_tracing_file;
	script_code_to_exec = og.script_code_to_exec;
	script_prefixes = og.script_prefixes;

	signature_re_level = og.signature_re_level;
	ignore_checksums = og.ignore_checksums;
	use_watchdog = og.use_watchdog;
	pseudo_realtime = og.pseudo_realtime;
	dns_mode = og.dns_mode;

	bare_mode = og.bare_mode;
	perftools_check_leaks = og.perftools_check_leaks;
	perftools_profile = og.perftools_profile;

	pcap_filter = og.pcap_filter;
	signature_files = og.signature_files;

	// TODO: These are likely to be handled in a node-specific or
	// use-case-specific way.  e.g. interfaces is already handled for the
	// "cluster" use-case, but don't have supervised-pcap-reading
	// functionality yet.
	/* interface = og.interface; */
	/* pcap_file = og.pcap_file; */

	pcap_output_file = og.pcap_output_file;
	random_seed_input_file = og.random_seed_input_file;
	random_seed_output_file = og.random_seed_output_file;
	process_status_file = og.process_status_file;

	plugins_to_load = og.plugins_to_load;
	scripts_to_load = og.scripts_to_load;
	script_options_to_set = og.script_options_to_set;
	}

bool zeek::fake_dns()
	{
	return zeekenv("ZEEK_DNS_FAKE");
	}

extern const char* zeek_version();

void zeek::usage(const char* prog, int code)
	{
	fprintf(stderr, "zeek version %s\n", zeek_version());

	fprintf(stderr, "usage: %s [options] [file ...]\n", prog);
	fprintf(stderr, "usage: %s --test [doctest-options] -- [options] [file ...]\n", prog);
	fprintf(stderr, "    <file>                         | Zeek script file, or read stdin\n");
	fprintf(stderr, "    -a|--parse-only                | exit immediately after parsing scripts\n");
	fprintf(stderr, "    -b|--bare-mode                 | don't load scripts from the base/ directory\n");
	fprintf(stderr, "    -d|--debug-script              | activate Zeek script debugging\n");
	fprintf(stderr, "    -e|--exec <zeek code>          | augment loaded scripts by given code\n");
	fprintf(stderr, "    -f|--filter <filter>           | tcpdump filter\n");
	fprintf(stderr, "    -h|--help                      | command line help\n");
	fprintf(stderr, "    -i|--iface <interface>         | read from given interface (only one allowed)\n");
	fprintf(stderr, "    -p|--prefix <prefix>           | add given prefix to Zeek script file resolution\n");
	fprintf(stderr, "    -r|--readfile <readfile>       | read from given tcpdump file (only one allowed, pass '-' as the filename to read from stdin)\n");
	fprintf(stderr, "    -s|--rulefile <rulefile>       | read rules from given file\n");
	fprintf(stderr, "    -t|--tracefile <tracefile>     | activate execution tracing\n");
	fprintf(stderr, "    -u|--usage-issues              | find variable usage issues and exit; use -uu for deeper/more expensive analysis\n");
	fprintf(stderr, "    -v|--version                   | print version and exit\n");
	fprintf(stderr, "    -w|--writefile <writefile>     | write to given tcpdump file\n");
#ifdef DEBUG
	fprintf(stderr, "    -B|--debug <dbgstreams>        | Enable debugging output for selected streams ('-B help' for help)\n");
#endif
	fprintf(stderr, "    -C|--no-checksums              | ignore checksums\n");
	fprintf(stderr, "    -F|--force-dns                 | force DNS\n");
	fprintf(stderr, "    -G|--load-seeds <file>         | load seeds from given file\n");
	fprintf(stderr, "    -H|--save-seeds <file>         | save seeds to given file\n");
	fprintf(stderr, "    -I|--print-id <ID name>        | print out given ID\n");
	fprintf(stderr, "    -N|--print-plugins             | print available plugins and exit (-NN for verbose)\n");
	fprintf(stderr, "    -O|--optimize[=<option>]       | enable script optimization (use -O help for options)\n");
	fprintf(stderr, "    -o|--optimize-only=<func>      | enable script optimization only for the given function\n");
	fprintf(stderr, "    -P|--prime-dns                 | prime DNS\n");
	fprintf(stderr, "    -Q|--time                      | print execution time summary to stderr\n");
	fprintf(stderr, "    -S|--debug-rules               | enable rule debugging\n");
	fprintf(stderr, "    -T|--re-level <level>          | set 'RE_level' for rules\n");
	fprintf(stderr, "    -U|--status-file <file>        | Record process status in file\n");
	fprintf(stderr, "    -W|--watchdog                  | activate watchdog timer\n");
	fprintf(stderr, "    -X|--zeekygen <cfgfile>        | generate documentation based on config file\n");

#ifdef USE_PERFTOOLS_DEBUG
	fprintf(stderr, "    -m|--mem-leaks                 | show leaks  [perftools]\n");
	fprintf(stderr, "    -M|--mem-profile               | record heap [perftools]\n");
#endif
	fprintf(stderr, "    --pseudo-realtime[=<speedup>]  | enable pseudo-realtime for performance evaluation (default 1)\n");
	fprintf(stderr, "    -j|--jobs                      | enable supervisor mode\n");

#ifdef USE_IDMEF
	fprintf(stderr, "    -n|--idmef-dtd <idmef-msg.dtd> | specify path to IDMEF DTD file\n");
#endif

	fprintf(stderr, "    --test                         | run unit tests ('--test -h' for help, only when compiling with ENABLE_ZEEK_UNIT_TESTS)\n");
	fprintf(stderr, "    $ZEEKPATH                      | file search path (%s)\n", bro_path().c_str());
	fprintf(stderr, "    $ZEEK_PLUGIN_PATH              | plugin search path (%s)\n", bro_plugin_path());
	fprintf(stderr, "    $ZEEK_PLUGIN_ACTIVATE          | plugins to always activate (%s)\n", bro_plugin_activate());
	fprintf(stderr, "    $ZEEK_PREFIXES                 | prefix list (%s)\n", bro_prefixes().c_str());
	fprintf(stderr, "    $ZEEK_DNS_FAKE                 | disable DNS lookups (%s)\n", zeek::fake_dns() ? "on" : "off");
	fprintf(stderr, "    $ZEEK_SEED_FILE                | file to load seeds from (not set)\n");
	fprintf(stderr, "    $ZEEK_LOG_SUFFIX               | ASCII log file extension (.%s)\n", logging::writer::Ascii::LogExt().c_str());
	fprintf(stderr, "    $ZEEK_PROFILER_FILE            | Output file for script execution statistics (not set)\n");
	fprintf(stderr, "    $ZEEK_DISABLE_ZEEKYGEN         | Disable Zeekygen documentation support (%s)\n", zeekenv("ZEEK_DISABLE_ZEEKYGEN") ? "set" : "not set");
	fprintf(stderr, "    $ZEEK_DNS_RESOLVER             | IPv4/IPv6 address of DNS resolver to use (%s)\n", zeekenv("ZEEK_DNS_RESOLVER") ? zeekenv("ZEEK_DNS_RESOLVER") : "not set, will use first IPv4 address from /etc/resolv.conf");
	fprintf(stderr, "    $ZEEK_DEBUG_LOG_STDERR         | Use stderr for debug logs generated via the -B flag");

	fprintf(stderr, "\n");

	exit(code);
	}

static void set_analysis_option(const char* opt)
	{
	if ( ! opt || streq(opt, "all") )
		{
		analysis_options.activate = true;
		analysis_options.compile = true;
		analysis_options.inliner = true;
		analysis_options.optimize = true;
		return;
		}

	if ( streq(opt, "help") )
		{
		fprintf(stderr, "--optimize options:\n");
		fprintf(stderr, "    all	turns on compile, inline, xform-opt\n");
		fprintf(stderr, "    compile	compile scripts to ZAM code; implies xform\n");
		fprintf(stderr, "    delete	delete saved ZAM code\n");
		fprintf(stderr, "    dump-code	dump ZAM code to stdout\n");
		fprintf(stderr, "    dump-max-rds	dump maximal reaching-defs to stdout\n");
		fprintf(stderr, "    dump-min-rds	dump minimal reaching-defs to stdout\n");
		fprintf(stderr, "    dump-uds	dump use-defs to stdout\n");
		fprintf(stderr, "    dump-xform	dump transformed scripts to stdout\n");
		fprintf(stderr, "    help	print this list\n");
		fprintf(stderr, "    inline	inline function calls\n");
		fprintf(stderr, "    no-load	do not load saved ZAM code\n");
		fprintf(stderr, "    no-save	do not save ZAM code\n");
		fprintf(stderr, "    no-ZAM-opt	turn off low-level ZAM optimization\n");
		fprintf(stderr, "    overwrite	overwrite saved ZAM code\n");
		fprintf(stderr, "    profile	generate to stdout a ZAM execution profile\n");
		fprintf(stderr, "    recursive	report on recursive functions and exit\n");
		fprintf(stderr, "    xform	tranform scripts to \"reduced\" form\n");
		fprintf(stderr, "    xform-opt	optimize \"reduced\" form scripts; implies xform\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "    -O		a bare --optimize/-O sets compile, inline and xform-opt\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "--optimize-only=func	apply options to only the given function\n");
		exit(0);
		}

	if ( streq(opt, "compile") )
		analysis_options.compile = true;
	else if ( streq(opt, "delete") )
		analysis_options.delete_save_files = true;
	else if ( streq(opt, "dump-code") )
		analysis_options.dump_code = true;
	else if ( streq(opt, "dump-max-rds") )
		analysis_options.max_rd_trace = true;
	else if ( streq(opt, "dump-min-rds") )
		analysis_options.min_rd_trace = true;
	else if ( streq(opt, "dump-uds") )
		analysis_options.ud_dump = true;
	else if ( streq(opt, "dump-xform") )
		analysis_options.dump_xform = true;
	else if ( streq(opt, "inline") )
		analysis_options.inliner = true;
	else if ( streq(opt, "no-load") )
		analysis_options.no_load = true;
	else if ( streq(opt, "no-save") )
		analysis_options.no_save = true;
	else if ( streq(opt, "no-ZAM-opt") )
		analysis_options.no_ZAM_opt = true;
	else if ( streq(opt, "overwrite") )
		analysis_options.overwrite_save_files = true;
	else if ( streq(opt, "profile") )
		analysis_options.report_profile = true;
	else if ( streq(opt, "recursive") )
		analysis_options.inliner =
			analysis_options.report_recursive = true;
	else if ( streq(opt, "xform") )
		analysis_options.activate = true;
	else if ( streq(opt, "xform-opt") )
		analysis_options.optimize = true;

	else
		{
		fprintf(stderr,"zeek: unrecognized --optimize option: %s\n", opt);
		exit(1);
		}
	}

zeek::Options zeek::parse_cmdline(int argc, char** argv)
	{
	zeek::Options rval;

	// When running unit tests, the first argument on the command line must be
	// --test, followed by doctest options. Optionally, users can use "--" as
	// separator to pass Zeek options afterwards:
	//
	//     zeek --test [doctest-options] -- [zeek-options]

	// Just locally filtering out the args for Zeek usage from doctest args.
	std::vector<std::string> zeek_args;

	if ( argc > 1 && strcmp(argv[1], "--test") == 0 )
		{
		#ifdef DOCTEST_CONFIG_DISABLE
		fprintf(stderr, "ERROR: C++ unit tests are disabled for this build.\n"
		                "       Please re-compile with ENABLE_ZEEK_UNIT_TESTS "
		                       "to run the C++ unit tests.\n");
		usage(argv[0], 1);
		#endif

		auto is_separator = [](const char* cstr)
			{
			return strcmp(cstr, "--") == 0;
			};
		auto first = argv;
		auto last = argv + argc;
		auto separator = std::find_if(first, last, is_separator);
		zeek_args.emplace_back(argv[0]);

		if ( separator != last )
			{
			auto first_zeek_arg = std::next(separator);

			for ( auto i = first_zeek_arg; i != last; ++i )
				zeek_args.emplace_back(*i);
			}

		rval.run_unit_tests = true;

		for ( auto i = 0; i < std::distance(first, separator); ++i )
			rval.doctest_args.emplace_back(argv[i]);
		}
	else
		{
		for ( auto i = 0; i < argc; ++i )
			zeek_args.emplace_back(argv[i]);
		}

	constexpr struct option long_opts[] = {
		{"parse-only",	no_argument,		0,	'a'},
		{"bare-mode",	no_argument,		0,	'b'},
		{"debug-script",	no_argument,		0,	'd'},
		{"exec",		required_argument,	0,	'e'},
		{"filter",		required_argument,	0,	'f'},
		{"help",		no_argument,		0,	'h'},
		{"iface",		required_argument,	0,	'i'},
		{"zeekygen",		required_argument,		0,	'X'},
		{"prefix",		required_argument,	0,	'p'},
		{"readfile",		required_argument,	0,	'r'},
		{"rulefile",		required_argument,	0,	's'},
		{"tracefile",		required_argument,	0,	't'},
		{"writefile",		required_argument,	0,	'w'},
		{"version",		no_argument,		0,	'v'},
		{"no-checksums",	no_argument,		0,	'C'},
		{"force-dns",		no_argument,		0,	'F'},
		{"load-seeds",		required_argument,	0,	'G'},
		{"save-seeds",		required_argument,	0,	'H'},
		{"print-plugins",	no_argument,		0,	'N'},
		{"prime-dns",		no_argument,		0,	'P'},
		{"time",		no_argument,		0,	'Q'},
		{"debug-rules",		no_argument,		0,	'S'},
		{"re-level",		required_argument,	0,	'T'},
		{"watchdog",		no_argument,		0,	'W'},
		{"print-id",		required_argument,	0,	'I'},
		{"status-file",		required_argument,	0,	'U'},

#ifdef	DEBUG
		{"debug",		required_argument,	0,	'B'},
#endif
#ifdef	USE_IDMEF
		{"idmef-dtd",		required_argument,	0,	'n'},
#endif
#ifdef	USE_PERFTOOLS_DEBUG
		{"mem-leaks",	no_argument,		0,	'm'},
		{"mem-profile",	no_argument,		0,	'M'},
#endif

		{"pseudo-realtime",	optional_argument, 0,	'E'},
		{"jobs",	optional_argument, 0,	'j'},
		{"test",		no_argument,		0,	'#'},

		{"optimize",	optional_argument, 0,	'O'},
		{"optimize-only",	required_argument, 0,	'o'},

		{0,			0,			0,	0},
	};

	char opts[256];
	safe_strncpy(opts, "B:e:f:G:H:I:i:j::n:O:o:p:r:s:T:t:U:w:X:CFNPQSWabdhuv",
	             sizeof(opts));

#ifdef USE_PERFTOOLS_DEBUG
	strncat(opts, "mM", 2);
#endif

	int op;
	int long_optsind;
	opterr = 0;

	// getopt may permute the array, so need yet another array
	auto zargs = std::make_unique<char*[]>(zeek_args.size());

	for ( auto i = 0u; i < zeek_args.size(); ++i )
		zargs[i] = zeek_args[i].data();

	while ( (op = getopt_long(zeek_args.size(), zargs.get(), opts, long_opts, &long_optsind)) != EOF )
		switch ( op ) {
		case 'a':
			rval.parse_only = true;
			break;
		case 'b':
			rval.bare_mode = true;
			break;
		case 'd':
			rval.debug_scripts = true;
			break;
		case 'e':
			rval.script_code_to_exec = optarg;
			break;
		case 'f':
			rval.pcap_filter = optarg;
			break;
		case 'h':
			rval.print_usage = true;
			break;
		case 'i':
			if ( rval.interface )
				{
				fprintf(stderr, "ERROR: Only a single interface option (-i) is allowed.\n");
				exit(1);
				}

			if ( rval.pcap_file )
				{
				fprintf(stderr, "ERROR: Using -i is not allow when reading a pcap file.\n");
				exit(1);
				}

			rval.interface = optarg;
			break;
		case 'j':
			rval.supervisor_mode = true;
			if ( optarg )
				{
				// TODO: for supervised offline pcap reading, the argument is
				// expected to be number of workers like "-j 4" or possibly a
				// list of worker/proxy/logger counts like "-j 4,2,1"
				}
			break;
		case 'p':
			rval.script_prefixes.emplace_back(optarg);
			break;
		case 'r':
			if ( rval.pcap_file )
				{
				fprintf(stderr, "ERROR: Only a single readfile option (-r) is allowed.\n");
				exit(1);
				}

			if ( rval.interface )
				{
				fprintf(stderr, "Using -r is not allowed when reading a live interface.\n");
				exit(1);
				}

			rval.pcap_file = optarg;
			break;
		case 's':
			rval.signature_files.emplace_back(optarg);
			break;
		case 't':
			rval.debug_script_tracing_file = optarg;
			break;
		case 'u':
			++analysis_options.usage_issues;
			break;
		case 'v':
			rval.print_version = true;
			break;
		case 'w':
			rval.pcap_output_file = optarg;
			break;
		case 'B':
			rval.debug_log_streams = optarg;
			break;
		case 'C':
			rval.ignore_checksums = true;
			break;
		case 'E':
			rval.pseudo_realtime = 1.0;
			if ( optarg )
				rval.pseudo_realtime = atof(optarg);
			break;
		case 'F':
			if ( rval.dns_mode != DNS_DEFAULT )
				usage(zargs[0], 1);
			rval.dns_mode = DNS_FORCE;
			break;
		case 'G':
			rval.random_seed_input_file = optarg;
			break;
		case 'H':
			rval.random_seed_output_file = optarg;
			break;
		case 'I':
			rval.identifier_to_print = optarg;
			break;
		case 'N':
			++rval.print_plugins;
			break;
		case 'O':
			set_analysis_option(optarg);
			break;
		case 'o':
			analysis_options.only_func = copy_string(optarg);
			break;
		case 'P':
			if ( rval.dns_mode != DNS_DEFAULT )
				usage(zargs[0], 1);
			rval.dns_mode = DNS_PRIME;
			break;
		case 'Q':
			rval.print_execution_time = true;
			break;
		case 'S':
			rval.print_signature_debug_info = true;
			break;
		case 'T':
			rval.signature_re_level = atoi(optarg);
			break;
		case 'U':
			rval.process_status_file = optarg;
			break;
		case 'W':
			rval.use_watchdog = true;
			break;
		case 'X':
			rval.zeekygen_config_file = optarg;
			break;

#ifdef USE_PERFTOOLS_DEBUG
		case 'm':
			rval.perftools_check_leaks = 1;
			break;
		case 'M':
			rval.perftools_profile = 1;
			break;
#endif

#ifdef USE_IDMEF
		case 'n':
			rval.libidmef_dtd_path = optarg;
			break;
#endif

		case '#':
			fprintf(stderr, "ERROR: --test only allowed as first argument.\n");
			usage(zargs[0], 1);
			break;

		case 0:
			// This happens for long options that don't have
			// a short-option equivalent.
			break;

		case '?':
		default:
			usage(zargs[0], 1);
			break;
		}

	if ( analysis_options.compile || analysis_options.optimize ||
	     analysis_options.inliner || analysis_options.only_func )
		analysis_options.activate = true;

	if ( analysis_options.report_recursive )
		analysis_options.inliner = true;

	if ( analysis_options.usage_issues > 0 )
		{
		analysis_options.activate = true;
		analysis_options.optimize = true;
		}

	if ( analysis_options.no_ZAM_opt )
		analysis_options.compile = true;

	// Process remaining arguments. X=Y arguments indicate script
	// variable/parameter assignments. X::Y arguments indicate plugins to
	// activate/query. The remainder are treated as scripts to load.
	while ( optind < static_cast<int>(zeek_args.size()) )
		{
		if ( strchr(zargs[optind], '=') )
			rval.script_options_to_set.emplace_back(zargs[optind++]);
		else if ( strstr(zargs[optind], "::") )
			rval.plugins_to_load.emplace(zargs[optind++]);
		else
			rval.scripts_to_load.emplace_back(zargs[optind++]);
		}

	auto canonify_script_path = [](std::string* path)
		{
		if ( path->empty() )
			return;

		*path = normalize_path(*path);

		if ( (*path)[0] == '/' || (*path)[0] == '~' )
			// Absolute path
			return;

		if ( (*path)[0] != '.' )
			{
			// Look up file in ZEEKPATH
			auto res = find_script_file(*path, bro_path());

			if ( res.empty() )
				{
				fprintf(stderr, "failed to locate script: %s\n", path->data());
				exit(1);
				}

			*path = res;

			if ( (*path)[0] == '/' || (*path)[0] == '~' )
				// Now an absolute path
				return;
			}

		// Need to translate relative path to absolute.
		char cwd[PATH_MAX];

		if ( ! getcwd(cwd, sizeof(cwd)) )
			{
			fprintf(stderr, "failed to get current directory: %s\n",
			        strerror(errno));
			exit(1);
			}

		*path = std::string(cwd) + "/" + *path;
		};

	if ( rval.supervisor_mode )
		{
		// Translate any relative paths supplied to supervisor into absolute
		// paths for use by supervised nodes since they have the option to
		// operate out of a different working directory.
		for ( auto& s : rval.scripts_to_load )
			canonify_script_path(&s);
		}

	return rval;
	}
