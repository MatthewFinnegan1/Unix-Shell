#ifndef PARSER_H
#define PARSER_H

#include <list>
#define MAX_LINE 81

class Process;

char* read_input();
void parse_input(char* input_line, std::list<Process*>& process_list);
bool isQuit(Process* process);
void sanitize(char* cmd);

#endif