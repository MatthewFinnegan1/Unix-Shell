#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include "process.h"
#include "executor.h"
#include "parser.h"

using namespace std;

void test_pipe_missing_side();
void test_add_token();
void test_parser();
void test_single_command_no_args();
void test_empty_input();
void test_semicolon_with_spaces();
void test_trailing_and_leading_spaces();
void test_three_stage_pipeline();

int main() {
  test_pipe_missing_side();
  test_single_command_no_args();
  test_empty_input();
  test_semicolon_with_spaces();
  test_trailing_and_leading_spaces();
  test_three_stage_pipeline();
  test_add_token();
  test_parser();
}
void test_pipe_missing_side() {
  {
    // Case 1: Pipe with no left-hand command
    std::list<Process*> processes;
    char input[] = "| grep foo";
    parse_input(input, processes);

    // Expect: no valid processes created
    assert(processes.size() == 0);

    std::cout << "pipe with no left-hand command handled!\n";
    for (auto proc : processes) delete proc;
  }

  {
    // Case 2: Pipe with no right-hand command
    std::list<Process*> processes;
    char input[] = "echo foo |";
    parse_input(input, processes);

    // Expect: left side (echo foo) parsed, but no right-hand process
    assert(processes.size() == 1);
    auto it = processes.begin();
    Process* p1 = *it;

    assert(strcmp(p1->cmdTokens[0], "echo") == 0);
    assert(strcmp(p1->cmdTokens[1], "foo") == 0);
    assert(p1->pipe_out == true);   // still marked as piping out
    // but no valid next process to receive it

    std::cout << "pipe with no right-hand command handled (with leftover left side)!\n";
    for (auto proc : processes) delete proc;
  }
}

void test_single_command_no_args() {
  std::list<Process*> processes;
  char input[] = "pwd";
  parse_input(input, processes);

  assert(processes.size() == 1);
  auto it = processes.begin();
  Process* p = *it;

  assert(strcmp(p->cmdTokens[0], "pwd") == 0);
  assert(p->cmdTokens[1] == nullptr);
  assert(p->pipe_in == false);
  assert(p->pipe_out == false);

  std::cout << "single_command_no_args works!\n";
  for (auto proc : processes) delete proc;
}

void test_empty_input() {
  std::list<Process*> processes;
  char input[] = "";  // empty line
  parse_input(input, processes);

  // Depending on your parser, this should yield 0 processes.
  assert(processes.size() == 0);

  std::cout << "empty_input works!\n";
  for (auto proc : processes) delete proc;
}

void test_semicolon_with_spaces() {
  std::list<Process*> processes;
  char input[] = "ls   ;   pwd";
  parse_input(input, processes);

  assert(processes.size() == 2);
  auto it = processes.begin();
  Process* p1 = *it++;
  Process* p2 = *it++;

  assert(strcmp(p1->cmdTokens[0], "ls") == 0);
  assert(p1->pipe_in == false);
  assert(p1->pipe_out == false);

  assert(strcmp(p2->cmdTokens[0], "pwd") == 0);
  assert(p2->pipe_in == false);
  assert(p2->pipe_out == false);

  std::cout << "semicolon_with_spaces works!\n";
  for (auto proc : processes) delete proc;
}

void test_trailing_and_leading_spaces() {
  std::list<Process*> processes;
  char input[] = "   echo   hello   ";
  parse_input(input, processes);

  assert(processes.size() == 1);
  auto it = processes.begin();
  Process* p = *it;

  assert(strcmp(p->cmdTokens[0], "echo") == 0);
  assert(strcmp(p->cmdTokens[1], "hello") == 0);
  assert(p->cmdTokens[2] == nullptr);
  assert(p->pipe_in == false);
  assert(p->pipe_out == false);

  std::cout << "trailing_and_leading_spaces works!\n";
  for (auto proc : processes) delete proc;
}

void test_three_stage_pipeline() {
  std::list<Process*> processes;
  char input[] = "echo foo | grep f | wc -c";
  parse_input(input, processes);

  assert(processes.size() == 3);

  auto it = processes.begin();
  Process* p1 = *it++;
  Process* p2 = *it++;
  Process* p3 = *it++;

  // p1: pipe out only
  assert(strcmp(p1->cmdTokens[0], "echo") == 0);
  assert(strcmp(p1->cmdTokens[1], "foo") == 0);
  assert(p1->pipe_in == false);
  assert(p1->pipe_out == true);

  // p2: both pipe in and out
  assert(strcmp(p2->cmdTokens[0], "grep") == 0);
  assert(strcmp(p2->cmdTokens[1], "f") == 0);
  assert(p2->pipe_in == true);
  assert(p2->pipe_out == true);

  // p3: pipe in only
  assert(strcmp(p3->cmdTokens[0], "wc") == 0);
  assert(strcmp(p3->cmdTokens[1], "-c") == 0);
  assert(p3->pipe_in == true);
  assert(p3->pipe_out == false);

  std::cout << "three_stage_pipeline works!\n";
  for (auto proc : processes) delete proc;
}

void test_add_token(){
  Process p(false, false);

  char *tok1 = new char[3]; 
  strcpy(tok1, "ls");

  char *tok2 = new char[3];
  strcpy(tok2, "-l");

  p.add_token(tok1);
  p.add_token(tok2);

  assert(strcmp(p.cmdTokens[0], "ls") == 0);
  assert(strcmp(p.cmdTokens[1], "-l") == 0);
  assert(p.cmdTokens[2] == nullptr); // should be NULL terminated

  std::cout << "add_token works!\n";
}

void test_parser(){
  std::list<Process*> processes;

  char input[] = "ls -l ; cat file | grep foo";
  parse_input(input, processes);

  assert(processes.size() == 3);

  auto it = processes.begin();
  Process *p1 = *it++;
  Process *p2 = *it++;
  Process *p3 = *it++;

  assert(strcmp(p1->cmdTokens[0], "ls") == 0);
  assert(strcmp(p1->cmdTokens[1], "-l") == 0);

  assert(strcmp(p2->cmdTokens[0], "cat") == 0);
  assert(strcmp(p2->cmdTokens[1], "file") == 0);
  assert(p2->pipe_out == true);

  assert(strcmp(p3->cmdTokens[0], "grep") == 0);
  assert(strcmp(p3->cmdTokens[1], "foo") == 0);
  assert(p3->pipe_in == true);

  std::cout << "parse_input works!\n";
  std::cout << p1->cmdTokens[0] << " " << p1->cmdTokens[1] << " " << p2->cmdTokens[0] << " " << p2->cmdTokens[1] << " " << p3->cmdTokens[0] << " " << p3->cmdTokens[1] << endl;

  for (auto proc : processes) delete proc;
}