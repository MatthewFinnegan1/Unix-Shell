#include "process.h"
#include "parser.h"
/**
 * @brief Constructor for Process class.
 */
Process::Process(bool _pipe_in_flag, bool _pipe_out_flag)
{
  pipe_in = _pipe_in_flag;
  pipe_out = _pipe_out_flag;
  tok_index = 0;
}

/**
 * @brief add a pointer to a command or flags to cmdTokens
 */
void Process::add_token(char *tok){
  if (!tok){
    return;
  }
  sanitize(tok);
  if (tok[0] == '\0') {          
    delete[] tok;               
    return;
  }
  this->cmdTokens[this->tok_index++] = tok;
  this->cmdTokens[this->tok_index] = nullptr;
}