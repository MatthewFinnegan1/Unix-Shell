#include "process.h"
#include "parser.h"
#include <string.h>
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


/**
 * @brief extracts I/O redirection tokens and sets process bool flags for later execution
 */
void Process::extract_redirection_tokens(){
  int i = 0;
  while (cmdTokens[i] != nullptr) {
        if (strcmp(cmdTokens[i], "<") == 0) {
            // Next token is the filename
            if (cmdTokens[i+1] != nullptr) {
                infile = strdup(cmdTokens[i+1]); 
                
                // Remove both < and filename from list by shifting everything left
                int j = i;
                while (cmdTokens[j+2] != nullptr) {
                    cmdTokens[j] = cmdTokens[j+2];
                    j++;
                }
                cmdTokens[j] = nullptr; 
                cmdTokens[j+1] = nullptr;
                continue; 
            }
        }
        else if ((strcmp(cmdTokens[i], ">") == 0) || (strcmp(cmdTokens[i], ">>") == 0)) {
            if(strcmp(cmdTokens[i], ">>") == 0){
              is_stream_extraction = true;
            }
            if (cmdTokens[i+1] != nullptr) {
                outfile = strdup(cmdTokens[i+1]); 
                
                // Remove both > and filename from list
                int j = i;
                while (cmdTokens[j+2] != nullptr) {
                    cmdTokens[j] = cmdTokens[j+2];
                    j++;
                }
                cmdTokens[j] = nullptr;
                cmdTokens[j+1] = nullptr;
                continue;
            }
        }
        i++;
    }
}