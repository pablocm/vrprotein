/*
 * backtrace.h
 * Source:
 * http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
 *
 *  Created on: Mar 25, 2014
 *      Author: pablocm
 */

#ifndef BACKTRACE_H_
#define BACKTRACE_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <iostream>
#include <execinfo.h>
#include <signal.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>
#include <cxxabi.h>

/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext {
	unsigned long uc_flags;
	struct ucontext* uc_link;
	stack_t uc_stack;
	struct sigcontext uc_mcontext;
	sigset_t uc_sigmask;
} sig_ucontext_t;

// Print stack trace with demangled symbols and exit.
inline void crit_err_hdlr(int sig_num, siginfo_t* info, void* ucontext) {
	sig_ucontext_t* uc = static_cast<sig_ucontext_t*>(ucontext);

	void* caller_address;
	/* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
	caller_address = reinterpret_cast<void*>(uc->uc_mcontext.eip); // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
	caller_address = reinterpret_cast<void*>(uc->uc_mcontext.rip); // RIP: x86_64 specific
#else
#error Unsupported architecture.
#endif

	std::cerr << std::endl << "signal " << sig_num << " (" << strsignal(sig_num) << "), address is "
			<< info->si_addr << " from " << caller_address << std::endl;

	void* array[50];
	int size = backtrace(array, 50);

	array[1] = caller_address;

	char** messages = backtrace_symbols(array, size);

	// skip first stack frame (points here)
	for (int i = 1; i < size && messages != NULL; ++i) {
		char *mangled_name = nullptr, *offset_begin = nullptr, *offset_end = nullptr;

		// find parantheses and +address offset surrounding mangled name
		for (char* p = messages[i]; *p; ++p) {
			if (*p == '(') {
				mangled_name = p;
			}
			else if (*p == '+') {
				offset_begin = p;
			}
			else if (*p == ')') {
				offset_end = p;
				break;
			}
		}

		// if the line could be processed, attempt to demangle the symbol
		if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin) {
			*mangled_name++ = '\0';
			*offset_begin++ = '\0';
			*offset_end++ = '\0';

			int status;
			char* real_name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);

			// if demangling is successful, output the demangled function name
			if (status == 0) {
				std::cerr << "[bt]: (" << i << ") " << messages[i] << " : " << real_name << "+"
						<< offset_begin << offset_end << std::endl;

			}
			// otherwise, output the mangled function name
			else {
				std::cerr << "[bt]: (" << i << ") " << messages[i] << " : " << mangled_name << "+"
						<< offset_begin << offset_end << std::endl;
			}
			free(real_name);
		}
		// otherwise, print the whole line
		else {
			std::cerr << "[bt]: (" << i << ") " << messages[i] << std::endl;
		}
	}
	std::cerr << std::endl;

	free(messages);

	exit(EXIT_FAILURE);
}

// print the stack trace
inline void stacktrace() {
	void* array[24];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 24);

	// print out all the frames
	printf("Stack trace:\n");
	backtrace_symbols_fd(array, size, STDOUT_FILENO);
}

#endif /* BACKTRACE_H_ */
