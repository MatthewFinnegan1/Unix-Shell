#ifndef PROCESS_H
#define PROCESS_H

class Process {
 public:
  Process(bool _pipe_in_flag, bool _pipe_out_flag);

  void add_token(char *tok);
  char *cmdTokens[25];

  bool pipe_in;
  bool pipe_out;

  int pipe_fd[2];
  int tok_index;
};

#endif