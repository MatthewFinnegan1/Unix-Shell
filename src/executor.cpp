#include "executor.h"
#include "process.h"
#include "parser.h"
#include <iostream>
#include <fcntl.h>
using namespace std;

void handle_io_redirection(Process* p) {
    if (p->infile) {
        int fd = open(p->infile, O_RDONLY);
        if (fd < 0) {
            perror("input redirection failed");
            exit(1);
        }
        dup2(fd, 0); 
        close(fd);
    }
    if (p->outfile) {
      int fd;
      if(p->is_stream_extraction == false){
        fd = open(p->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      }else{
        fd = open(p->outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
      }
        if (fd < 0) {
            perror("output redirection failed");
            exit(1);
        }
        dup2(fd, 1); 
        close(fd);
    }
}
/**
 * @brief Helper function to run command when pipe_in = 0, pipe_out = 0
 * @return boolean value indicating syscall success or failure
 */
pid_t pipe00(Process* curr_proc, Process*& prev_proc, int& pid_index){
  pid_t pid = fork();
  if (pid < 0){
    perror("fork");
    return -1;
  }
  /*-------CHILD--------*/
  if (pid == 0){
    handle_io_redirection(curr_proc);
    execvp(curr_proc->cmdTokens[0], curr_proc->cmdTokens);
  }
  /*-------PARENT--------*/

  prev_proc = curr_proc;
  prev_proc->pipe_fd[0] = -1; //no pipes created yet
  prev_proc->pipe_fd[1] = -1;

  return pid;
}

/**
 * @brief Helper function to run command when pipe_in = 0, pipe_out = 1
 * @return boolean value indicating syscall success or failure
 */
pid_t pipe01(Process* curr_proc, Process*& prev_proc, int& pid_index, pid_t pids[]){
  int fd[2];
  if(pipe(fd) < 0){
    perror("pipe");
    return -1;
  }

  pid_t pid = fork();
  if (pid < 0){
    perror("fork");
    return -1;
  }
  /* CHILD */
  if (pid == 0){
    close(fd[0]); //close read
    dup2(fd[1], 1); // dup write to stdout
    close(fd[1]); //close write
    handle_io_redirection(curr_proc);
    execvp(curr_proc->cmdTokens[0], curr_proc->cmdTokens);
  }
  /* PARENT */
  close(fd[1]);

  prev_proc = curr_proc;
  prev_proc->pipe_fd[0] = fd[0]; //gonna want to dup2 this to stdin on pipe10
  return pid;
}

/**
 * @brief Helper function to run command when pipe_in = 1, pipe_out = 0
 * @return boolean value indicating syscall success or failure
 */
pid_t pipe10(Process* curr_proc, Process*& prev_proc, int& pid_index, pid_t pids[]){
  pid_t pid = fork();
  if (pid < 0){
    perror("fork");
    return -1;
  }
  
  /* CHILD */
  if (pid == 0){   
    dup2(prev_proc->pipe_fd[0], 0); // dup read of parent (output of prior proc) to stdin
    close(prev_proc->pipe_fd[0]); //close write
    handle_io_redirection(curr_proc);
    execvp(curr_proc->cmdTokens[0], curr_proc->cmdTokens);
  }
  /* PARENT */
  close(prev_proc->pipe_fd[0]);
  prev_proc->pipe_fd[0] = -1;
  prev_proc = curr_proc;
  return pid;
}

/**
 * @brief Helper function to run command when pipe_in = 1, pipe_out = 1
 * @return boolean value indicating syscall success or failure
 */
pid_t pipe11(Process* curr_proc, Process*& prev_proc, int& pid_index, pid_t pids[]){
  int fd[2];
  if(pipe(fd) < 0){
    perror("pipe");
    return -1;
  }

  pid_t pid = fork();
  if (pid < 0){
    perror("fork");
    return -1;
  }
  /* CHILD*/
  if (pid == 0){   
    dup2(prev_proc->pipe_fd[0], 0); // dup read of parent (output of prior proc) to stdin
    close(prev_proc->pipe_fd[0]); 
    
    close(fd[0]); //close read
    dup2(fd[1], 1); // dup write to stdout
    close(fd[1]); //close write
    
    handle_io_redirection(curr_proc);
    execvp(curr_proc->cmdTokens[0], curr_proc->cmdTokens);
  }
  /* PARENT */
  close(fd[1]);

  prev_proc = curr_proc;
  prev_proc->pipe_fd[0] = fd[0];
  return pid;
}
/**
 * @brief
 * Helper function to print the PS1 pormpt.
 */
void display_prompt() { cout << "Finnegan_Shell$ " << flush; }

/**
 * @brief Cleans up allocated resources to prevent memory leaks
 */

void cleanup(list<Process *> &process_list, char *input_line){
  for (Process *p : process_list){
    for(char* token : p->cmdTokens){
      delete[] token;
    }
    delete p;
  }
  process_list.clear();
  free(input_line);
  input_line = nullptr;
}


/**
 * @brief Main loop for the shell, facilitating user interaction and command
 * execution.
 */
void run(){
  list<Process *> process_list;
  char *input_line;
  bool is_quit = false;

  for(;;){
    display_prompt();
    input_line = read_input();

    if(!input_line){
      break;
    }
    
    parse_input(input_line, process_list);
    is_quit = run_commands(process_list);
    cleanup(process_list, input_line);

    if(is_quit){
      break;
    }
  }
}

/**
 * @brief Execute a list of commands using processes and pipes.
 */
bool run_commands(list<Process *> &command_list){
  bool is_quit = false;
  int pid_index = 0;
  int size = command_list.size();
  pid_t pids[size];
  Process *prev = nullptr;

  for (Process *p : command_list){
    if (isQuit(p)){
      return true;
    }

    pid_t child_pid = -1;

    if (p->pipe_in == 0 && p->pipe_out == 0){
      child_pid = pipe00(p, prev, pid_index);
    }else if(p->pipe_in == 0 && p->pipe_out == 1){
      child_pid = pipe01(p, prev, pid_index, pids);
    }else if(p->pipe_in == 1 && p->pipe_out == 0){
      child_pid = pipe10(p, prev, pid_index, pids);
    }else{
      child_pid = pipe11(p, prev, pid_index, pids);
    }

    if(child_pid > 0){
      if(p->is_background){
        std::cout << "[" << child_pid << "]" << std::endl;
      }else{
        pids[pid_index++] = child_pid;
      }
    }
  }
  /* Reap children */
    for (int i = 0; i < pid_index; ++i) {                  
      waitpid(pids[i], NULL, 0);
    }
    return false;
}

