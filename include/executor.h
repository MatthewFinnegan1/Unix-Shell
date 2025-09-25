#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <list>
#include <sys/types.h>
#include <unistd.h>
class Process;

void run();
void cleanup(std::list<Process*>& process_list, char* input_line);
bool run_commands(std::list<Process*>& command_list);
void display_prompt();

#endif