#include "parser.h"
#include "process.h"

using namespace std;

/**
 * @brief Creates a string token based off of two pointers to characters of a string
 * @return A boolean value indicating if the substring (token) is only a space
 */
bool createToken(char *&token, char *&start, char *delimiter){
  size_t len = delimiter - start;
  token = nullptr;
  if (len > 0){
    token = new char[len + 1];
    memcpy(token, start, len);
    token[len] = '\0';
    start = delimiter + 1;
  }else{
    /*----ignore space padding----*/
    if (*delimiter == ' '){
      start = delimiter + 1;
      return false;
    }
    /*---Fall through if [;] or [|], this way we won't ignore them if padded with spaces-----*/
    start = delimiter + 1;
  }
  return true;
}

/**
 * @brief Reads input from the standard input (stdin) in chunks and dynamically
 *        allocates memory to store the entire input.
 * @return A pointer to the dynamically allocated memory containing the input
 * string. 
 */
char *read_input(){
  char *input = NULL;
  char tempbuf[MAX_LINE];
  size_t inputlen = 0, templen = 0;

  do{
    char *a = fgets(tempbuf, MAX_LINE, stdin);
    if (a == nullptr){
      return input;
    }
    templen = strlen(tempbuf);
    input = (char *)realloc(input, inputlen + templen + 1);
    strcpy(input + inputlen, tempbuf);
    inputlen += templen;
  } while (templen == MAX_LINE - 1 && tempbuf[MAX_LINE - 2] != '\n');

  return input;
}


/**
 * @brief
 * removes the new line char of the end in cmd.
 */
void sanitize(char *cmd){
  size_t n = strlen(cmd);
  while (n && (cmd[n-1] == '\n' || cmd[n-1] == '\r')) cmd[--n] = '\0';
}

/**
 * @brief Parses the given command string and populates a list of Process objects.
 */
void parse_input(char *cmd, list<Process *> &process_list){
  Process *currProcess = new Process(0, 0);
  const char *delimiters = "|; ";
  char *pCurrentDelimiter;
  char *start = cmd;

  while (pCurrentDelimiter = strpbrk(start, delimiters)){
    /*-----------------------------CREATE TOKEN-------------------------------*/
    char *token;
    if (!createToken(token, start, pCurrentDelimiter)){
      continue;
    }
    /*-------------------------POPULATE PROCESS LIST---------------------------*/
    /*------------DELIMITER IS [SPACE]--------------------*/
    if (*pCurrentDelimiter == ' '){
      currProcess->add_token(token);
    }

    /*------------DELIMITER IS [SEMI COLON]--------------*/
    if (*pCurrentDelimiter == ';'){
      
      if (currProcess->tok_index == 0 && token == nullptr) {
        fprintf(stderr, "syntax error: empty command between ';'\n");
        return;
      }

      if (token){
        currProcess->add_token(token);
      }

      process_list.push_back(currProcess);
      currProcess = new Process(0, 0);
      
    }

    /*---------------DELIMITER IS [PIPE]------------------*/
    if (*pCurrentDelimiter == '|'){
      if (currProcess->tok_index == 0 && token == nullptr) {
        fprintf(stderr, "syntax error: pipe has no left command\n");
        return;
      }

      if (token){
        currProcess->add_token(token);
      }

      currProcess->pipe_out = 1;
      process_list.push_back(currProcess);
      currProcess = new Process(1, 0);
    }
  }
  /*----------------POST LOOP----------------*/
  if (*start != '\n'){
    /*------ Create token -----*/
    size_t len = strlen(start);
    if(len > 0){
      size_t len = strlen(start);
      char *token = new char[len + 1];
      memcpy(token, start, len);
      token[len] = '\0';

      currProcess->add_token(token);
      process_list.push_back(currProcess);
    }else{
      //Check for trailing pipe
      if (currProcess->pipe_in == 1 && currProcess->tok_index == 0) {
        fprintf(stderr, "syntax error: pipe has no right command\n");
        return;
      }
      if (currProcess->tok_index > 0) {
        process_list.push_back(currProcess);
      }
    }

  }else{
    //check for trailing pipe
    if (currProcess->pipe_in == 1 && currProcess->tok_index == 0) {
      fprintf(stderr, "syntax error: pipe has no right command\n");
      return;
    }
    if (currProcess->tok_index > 0) {
      process_list.push_back(currProcess);
    }
  }
}

/**
 * @brief Check if the given command represents a quit request.
 * @return:
 *   - true if the command is a quit request (the first token is "quit").
 *   - false otherwise.
 */
bool isQuit(Process *p){
  if (strcmp(p->cmdTokens[0], "quit") == 0){
    return true;
  }
  else{
    return false;
  }
}