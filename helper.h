#ifndef HELPER_H
#define HELPER_H

int myfork(void);  // Fork but exits on failure.
void unblock_signals(void);	// unblock all signals for child process
#endif
