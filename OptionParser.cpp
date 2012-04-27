/* This code is released into the public domain,
 * WITHOUT WARRANTY OF ANY KIND.
 */

#include "OptionParser.hpp"
#include <iostream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdarg>

#ifndef NDEBUG
#include <tr1/unordered_set>
#endif

namespace {

#ifndef NDEBUG
static bool has_ambiguous_flags(const OptionParser::FlagSpec *specs, int numspecs) {
	char shortflags[256];
	std::tr1::unordered_set<std::string> longflags;

	std::memset(shortflags, 0, 256);

	for (int i = 0; i < numspecs; ++i) {
		const OptionParser::FlagSpec &spec = specs[i];
		if (spec.short_flags) {
			for (const char *c = spec.short_flags; *c; ++c) {
				int cid = *c;
				if (shortflags[cid]) { return true; }
				++shortflags[cid];
			}
		}
		if (spec.long_flag) {
			std::string s(spec.long_flag);
			if (longflags.count(s)) { return true; }
			longflags.insert(std::move(s));
		}
	}

	return false;
}
#endif

} // annoymous namespace

void OptionParser::print_usage(const OptionParser::FlagSpec *specs, std::ostream &ss, const char *progname, const char *description) {
	assert(specs && progname);
	ss << "usage: " << progname << " [options] inputs\n";
	if (description) {
		ss << "\n" << description;
	}
	if (specs->short_flags || specs->long_flag) {
		ss << "\nOptions:\n";
		for (const OptionParser::FlagSpec *s = specs; s->short_flags || s->long_flag; ++s) {
			if (! s->help)
				continue;

			ss << "    ";
			if (s->short_flags && *s->short_flags) {
				ss << "-" << *s->short_flags << (s->metaname ? s->metaname : "");
				for (const char *c = s->short_flags + 1; *c; ++c) {
					ss << ", -" << *c << (s->metaname ? s->metaname : "");
				}
				if (s->long_flag && *s->long_flag)
					ss << ", ";
			}
			if (s->long_flag && *s->long_flag) {
				ss << "--" << s->long_flag;
				if (s->metaname)
					ss << "=" << s->metaname;
			}
			ss << "\n";
			ss << "          " << s->help << "\n\n";
		}
	}
}

OptionParser::OptionParser(const OptionParser::FlagSpec *specs, int argc, char **argv):
	specs(specs), argc(argc), argv(argv) {
	assert(specs && argc >= 0 && argv);

	numspecs = 0;
	while (specs[numspecs].short_flags || specs[numspecs].long_flag)
		++numspecs;

#ifndef NDEBUG
	assert(! has_ambiguous_flags(specs, numspecs));
#endif

	x = y = 0;
	remain = 0;
	flag_idx = -1;
	arg_value = 0;
}

OptionParser::~OptionParser() {
}

void OptionParser::print_usage(std::ostream &ss, const char *description) {
	OptionParser::print_usage(specs, ss, argv[0], description);
}

int OptionParser::next() {
	while (remain || (x < argc)) {
		char *flagtext;
		char *value = 0;
		int flagidx = -1;
		if (remain) {
			// short flag
			flagtext = remain - 1;
			flagtext[0] = '-';
			value = flagtext + 2;
			if (!*value) { value = 0; }
			flagidx = match_short_flag(flagtext[1]);
			if (!*++remain) { remain = 0; }
		} else {
			flagtext = argv[x++];
			if (flagtext[0] == '-' && flagtext[1] != '\0') {
				if (flagtext[1] == '-') {
					if (flagtext[2] == '\0') { break; } // end of flags
					// long flag
					value = std::strchr(flagtext+2, '=');
					if (value) { *value++ = '\0'; }
					flagidx = match_long_flag(flagtext+2);
				} else {
					// short flags
					remain = flagtext+1;
					continue;
				}
			} else {
				// positional argument
				if (flagtext[0] != '\0') argv[y++] = flagtext;
				continue;
			}
		}

		if (flagidx < 0) {
			std::ostringstream ss;
			ss << "unknown flag '" << flagtext << "'";
			throw UnknownFlag(flagtext, ss.str());
		}
		if (specs[flagidx].metaname) {
			if (!value)
				value = (x < argc ? argv[x++] : 0);
			if (!value) {
				std::ostringstream ss;
				ss << "expected argument for flag '" << flagtext << "'";
				throw ExpectedArg(flagtext, ss.str());
			}
			remain = 0;
		} else
			value = 0;

		arg_value = value;
		flag_idx = flagidx;
		return specs[flagidx].id;
	}

	while (x < argc) argv[y++] = argv[x++];
	x = y;
	while (x < argc) argv[x++] = 0;
	return -1;
}

const char *OptionParser::arg() const {
	return arg_value;
}

int OptionParser::arg_count() const {
	return y;
}

int OptionParser::match_long_flag(const char *flag) {
	for (int i = 0; i < numspecs; ++i) {
		const FlagSpec &s = specs[i];
		if (s.long_flag && !std::strcmp(s.long_flag, flag)) { return i; }
	}
	return -1;
}

int OptionParser::match_short_flag(char f) {
	for (int i = 0; i < numspecs; ++i ) {
		const FlagSpec &s = specs[i];
		if (s.short_flags && std::strchr(s.short_flags, f)) { return i; }
	}
	return -1;
}
