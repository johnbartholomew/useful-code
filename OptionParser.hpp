#ifndef OPTIONPARSER_HPP
#define OPTIONPARSER_HPP

/* Option parser. This file (and its associated .cpp file) is intended
 * to be copied into your project directly, and edited to suit your own
 * purposes.
 *
 * This code is released into the public domain,
 * WITHOUT WARRANTY OF ANY KIND.
 */

#include <string>
#include <iosfwd>
#include <exception>

class OptionParser {
	public:
		struct FlagSpec {
			/// Unique identifier for the flag; returned by next().
			/// Don't set this to -1 or you won't be able to tell when the flags end.
			/// Typically a mneumonic character is used (e.g., 'h' for a --help flag).
			int id;

			/// Short flag(s), e.g., "h?" to support both -h and -?.
			/// Set to null if this flag has no short form.
			const char *short_flags;

			/// Long flag. e.g., "help"
			/// Set to null if this flag has no long form.
			const char *long_flag;

			/// A name to use for the argument when the option is printed by print_usage.
			/// The presence (non-nullness) of this field indicates whether an argument
			/// is expected for the flag.
			const char *metaname;

			/// A description/help string for the flag, printed by print_usage.
			const char *help;
		};

		struct BadFlag : public std::exception {
			public:
				BadFlag(const char *flag, const std::string &s): flag(flag), msg(s) {}
				virtual ~BadFlag() noexcept {}
				virtual const char *what() const throw() { return msg.c_str(); }
			private:
				const char *flag;
				std::string msg;
		};

		struct ExpectedArg : public BadFlag {
			ExpectedArg(const char *flag, const std::string &s): BadFlag(flag, s) {}
		};

		struct UnknownFlag : public BadFlag {
			UnknownFlag(const char *flag, const std::string &s): BadFlag(flag, s) {}
		};

		/// Construct the parser. Note that argv is non-const: the array is rearranged during parsing,
		/// so that when all flags have been processed (as indicated by next() returning -1),
		/// the array will contain only positional arguments (followed by some nulls).
		/// When parsing is completed, use arg_count() to find the number of positional arguments.
		OptionParser(const FlagSpec *specs, int argc, char **argv);
		~OptionParser();

		/// Print a usage summary of the program, listing the flags from the 'spec' parameter.
		static void print_usage(const FlagSpec *spec, std::ostream &ss, const char *progname, const char *description = 0);

		/// Print a usage summary of the program, listing the available flags.
		void print_usage(std::ostream &ss, const char *description = 0);

		/// @return The 'id' value of the next flag, or -1 if there are no more flags.
		int next();

		/// @return The argument associated with the previous flag returned by 'next'.
		const char *arg() const;

		/// @return The number of free arguments. Only valid once next() returns -1.
		int arg_count() const;

	private:
		int match_long_flag(const char *flag);
		int match_short_flag(char f);

		const FlagSpec *specs;
		int numspecs;
		int argc;
		char **argv;

		int x, y;
		char *remain;
		int flag_idx;
		char *arg_value;
};

#endif
