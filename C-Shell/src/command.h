#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdbool.h>
#include <fcntl.h>

#include <readline/readline.h>
#include <readline/history.h>

#define CHUNK 1024
#define ULTRA_CHUNK 4096
#define MEGA_CHUNK 10240
#define NUM_COMMANDS 9

#define RE '>'
#define RESTR ">"
#define RE1 "1>"
#define ER "2>"
#define APP ">>"
#define APP1 "1>>"
#define APPER "2>>"

#define ECHO "echo"
#define EXIT "exit"
#define TYPE "type"
#define PWD "pwd"
#define CD "cd"
#define COMPLETE "complete"
#define JOBS "jobs"
#define HISTORY "history"
#define DECLARE "declare"

#define STRCMP(str, str2) (strcmp((str), (str2)) == 0)

#define OpenAndWriteToFile(filename, output)  \
{                                  \
    FILE* file;                              \
    file = fopen(filename, "w");                \
    if (!(strcmp(output, "") == 0)) {               \
        fprintf(file, "%s\n", output);               \
    }                                               \
    fclose(file);                                 \
};

#define OpenAndAppendToFile(filename, output)  \
{                                  \
    FILE* file;                              \
    file = fopen(filename, "a");                \
    if (!(strcmp(output, "") == 0)) {               \
        fprintf(file, "%s\n", output);               \
    }                                               \
    fclose(file);                                 \
};

#define BuildArgv(cmd, argv) \
{  \
    int stopindex;   \
    for (int i = 0; i < cmd->num_args; i++) {  \
    if (strcmp(cmd->command_args[i], ">") == 0 || strcmp(cmd->command_args[i], "1>") == 0 || strcmp(cmd->command_args[i], "2>") == 0 || strcmp(cmd->command_args[i], ">>") == 0 || strcmp(cmd->command_args[i], "1>>") == 0 || strcmp(cmd->command_args[i], "2>>") == 0) { \
            stopindex = i;  \
            break;    \
        }   \
        argv[i] = cmd->command_args[i];  \
    }    \
    argv[stopindex] = NULL;    \
}

#define push_job(job) {                                              \
    jobs* job_push = malloc(sizeof(*job_push));                       \
    job_push->current_job = strdup((job)->current_job);                \
    job_push->current_pid = (job)->current_pid;                         \
    job_push->job_status = (job)->job_status;                            \
    job_push->job_num = (job)->job_num;                                   \
    jobs_container->job_container[jobs_container->job_index++] = job_push; \
}

#define check_job_status(job, exited) {                                                         \
    int status;                                                                                  \
    pid_t result = waitpid(job->current_pid, &status, WNOHANG);                                   \
    if (result == job->current_pid)                                                                \
       *exited = true;                                                                              \
}  

#define reap_job(job) {                    \
    (job)->current_job[strlen((job)->current_job) - 2] = '\0';                             \
    printf("[%d]%s  %-24s %s\n",  (job)->job_num, marker, "Done", (job)->current_job);\
    (job)->job_status = Done;     \
    jobs_container->current_job_num--; \
}

#define push_history(command) {       \
   if (command_histories == NULL) { \
        command_histories = malloc(sizeof(command_history));         \
        command_histories->prev_commands = malloc(CHUNK * sizeof(char*));       \
        command_histories->index = 0;                  \
   }                    \
   command_histories->prev_commands[command_histories->index++] = strdup(command);   \
}

#define read_command_history(file) {\
    FILE *fp = fopen((file), "r");                 \
    if (fp == NULL) {                             \
        perror("fopen failed");                    \
    }                                                \
                                                      \
    char line[CHUNK];                                  \
    while (fgets(line, sizeof(line), fp) != NULL) {     \
        line[strcspn(line, "\n")] = '\0';                 \
        push_history(line);                             \
    }                                                     \
                                                             \
    fclose(fp);                                              \
}

#define write_command_history(file) {      \
    FILE* fp = fopen(file, "w");            \
    if (fp == NULL) {                        \
        perror("fopen failed");                \
    }                                          \
    for (int i = 0; i < command_histories->index; i++)       \
        fprintf(fp, "%s\n", command_histories->prev_commands[i]);      \
    fclose(fp);                                                          \
}

#define append_command_history(file) {      \
    static int last_index = 0;              \
    FILE* fp = fopen(file, "a");            \
    if (fp == NULL) {                        \
        perror("fopen failed");                \
    }                                          \
    for (int i = last_index; i < command_histories->index; i++)       \
        fprintf(fp, "%s\n", command_histories->prev_commands[i]);      \
    fclose(fp);                                                          \
    last_index = command_histories->index;                                \
}

typedef struct {
    const char* cmd_name;
    const char* completer_script;
    const char* output;
    char* chopped_output[ULTRA_CHUNK];
    size_t chopped_index;
    char** argvs;
    char* cached_line;
} register_command;

typedef struct {
    register_command* registers[CHUNK];
    size_t index;
} Registers;

typedef struct {
    char* commands[MEGA_CHUNK];
    size_t commands_size;
} completion_candidates;

typedef struct {
    char* current_job;
    size_t current_pid;
    int job_num;

    enum {
      Running,
      Done  
    } job_status;

} jobs;

typedef struct {
    jobs* job_container[CHUNK];
    int job_index;
    int current_job_num;
} job_containers;

typedef struct {
    const char** prev_commands;
    size_t index;
} command_history;

typedef struct {
    char* name;
    char* value;
} variable;

typedef struct {
    variable** declared_variables;
    size_t index;
} variable_container;

struct Command {
  char* raw_command;
  char* command_args[CHUNK];
  int num_args;
  const char* commands[NUM_COMMANDS];
  jobs* active_job;  
};

// persistent globals, lifetime is equal to program lifetime
bool running = true;
Registers* global_registration;
completion_candidates* global_completions;
job_containers* jobs_container;
command_history* command_histories;
variable_container* variables_container;

// Function Declerations
bool searchPathForFile(const char* filename, char* result, size_t result_size);
bool execute_program(const struct Command* cmd, char* path, register_command* registered);
bool check_for_command(const struct Command* cmd);
bool isTokenEmpty(const char* token);
void EmptyToken(char* token, size_t token_length);
void handleQuotes(struct Command* cmd);
char* findAndReturnFilename(const struct Command* cmd);
void AppendExecutables(completion_candidates* comp_cand);
void RegisterCommand(const char* cmdname, const char* script, Registers* regis, char** argv);
void PullRegister(const char* cmdname, const Registers* regis);
register_command* CheckForRegister(const char* cmdname, const Registers* regis);
void FreeRegisters(const Registers* regis, const register_command* null_regis);
bool ExecuteRegister(struct Command* cmd, const Registers* regis);
bool CommandAlreadyExists(const char* str);
bool CandidateAlreadyExists(const char* str, const register_command* reg);
void SortCandidatesAlphabetically(register_command* regis);
bool HandleCompletion(register_command* regis_command, struct Command* cmd);
bool ComputeLCP(const register_command* regis_command, bool two_tabbed);
bool run_job(const struct Command* cmd, const char* path);
void FreeJobContainer();
const char* setMarker(const jobs* job);
bool execute_multiple_programs(const struct Command* cmd, char** paths, char*** argvs, int ncmds);
bool CheckForMultiPipe(const struct Command* cmd);
bool free_allocations(struct Command* cmd);
int build_mult_pipe_arrays(const struct Command* cmd, char*** out_paths, char**** out_argvs); 
void rebuild_raw_command(struct Command* cmd); 
void ExpandVariables(struct Command* cmd);

void init_command(struct Command* cmd, char* raw_command) {
  if (raw_command == NULL) {
    printf("Error: raw_command is NULL.\n");
    exit(1);
  }
  cmd->raw_command = raw_command;
  handleQuotes(cmd);

  cmd->commands[0] = ECHO;
  cmd->commands[1] = EXIT;
  cmd->commands[2] = TYPE;
  cmd->commands[3] = PWD;
  cmd->commands[4] = CD;
  cmd->commands[5] = COMPLETE;
  cmd->commands[6] = JOBS;
  cmd->commands[7] = HISTORY;
  cmd->commands[8] = DECLARE;

  cmd->active_job = malloc(sizeof(*cmd->active_job));  
  cmd->active_job->current_job = NULL;
  cmd->active_job->current_pid = 0;  
  cmd->active_job->job_num = 0;
}

void execute_command(struct Command* cmd) {
  if (cmd->command_args[0] == NULL) return;

  push_history(cmd->raw_command);

  char* mult_pipe = strchr(cmd->raw_command, '|');
  if (mult_pipe) 
     if (CheckForMultiPipe(cmd)) return;    

  ExpandVariables(cmd);
  
  if (STRCMP(cmd->command_args[0], EXIT) && cmd->num_args == 1) {
    running = false;
    return;
  }

  if (STRCMP(cmd->command_args[0], ECHO) && cmd->num_args > 1) {
    char* LEN = cmd->raw_command + strlen(cmd->command_args[0]) + 1;

    char* re = strchr(LEN, RE);
    char* re1 = strstr(LEN, RE1);
    char* er = strstr(LEN, ER);
    char* ap = strstr(LEN, APP);
    char* ap1 = strstr(LEN, APP1);
    char* aper = strstr(LEN, APPER);


    if ((re != NULL || re1 != NULL || ap != NULL || ap1 != NULL) && (er == NULL && aper == NULL)) {
        const char* file_name = findAndReturnFilename(cmd);
        if (file_name == NULL) {
            printf("%s\n", LEN);
            return;
        }

        char output[strlen(LEN)];
        int i = 0;
        char target = *re;
        
        for (char* p = LEN; *p != target; p++) {
            if ((re1 != NULL || ap1 != NULL) && *(p + 1) == target) 
                break;
            output[i] = *p;
            i++;
        }
        output[i] = '\0';

        bool append = (ap != NULL || ap1 != NULL);
        if (append) {
            OpenAndAppendToFile(file_name, output);
            return;
        }
        OpenAndWriteToFile(file_name, output);
    }
    else if (er != NULL || aper != NULL) {
        const char* filename;
        for (int i = 1; i < cmd->num_args; i++) {
            if (strcmp(cmd->command_args[i], ER) == 0 || strcmp(cmd->command_args[i], APPER) == 0) {
                filename = cmd->command_args[i + 1];
                break;
            } 
            printf("%s ", cmd->command_args[i]);
        }
        printf("\n");

        bool append = (aper != NULL);
        if (append) {
            OpenAndAppendToFile(filename, "");
            return;
        }
        OpenAndWriteToFile(filename, "");
    }
    else {
        printf("%s\n", LEN);
    }
    return;
}

  if (STRCMP(cmd->command_args[0], TYPE)) {
     if (cmd->commands[0] == NULL) {
          cmd->commands[0] = ECHO;
          cmd->commands[1] = EXIT;
          cmd->commands[2] = TYPE;
          cmd->commands[3] = PWD;
          cmd->commands[4] = CD;
          cmd->commands[5] = COMPLETE;
          cmd->commands[6] = JOBS;
          cmd->commands[7] = HISTORY;
          cmd->commands[8] = DECLARE;
     }
    if (check_for_command(cmd)) return;

    char* path = malloc(CHUNK * sizeof(char*));
    if (searchPathForFile(cmd->command_args[1], path, (CHUNK * sizeof(char*)) )) {
      printf("%s is %s\n", cmd->command_args[1], path);
      free(path);
      return;
    }
    free(path);
    printf("%s: not found\n", cmd->command_args[1]);
    return;
  }

  if (STRCMP(cmd->command_args[0], PWD)) {
    char* cwd = getcwd(NULL, 0);
    if (cwd != NULL) {
        printf("%s\n", cwd);
        free(cwd);
        return;
    }
    else {
        perror("getcwd");
        free(cwd);
        exit(1);
    }
  }

  if (STRCMP(cmd->command_args[0], CD)) {
    if (cmd->command_args[1][0] == '/') {
        if (chdir(cmd->command_args[1]) != 0) {
            printf("cd: %s: No such file or directory\n", cmd->command_args[1]);
            return;
        }
  } else if (cmd->command_args[1][0] == '.' && cmd->command_args[1][1] == '/') {
    if (chdir(cmd->command_args[1] + 2) != 0) {
        printf("cd: %s: No such file or directory\n", cmd->command_args[1]);
        return;
    }
  } else if (cmd->command_args[1][0] == '.' && cmd->command_args[1][1] == '.' && cmd->command_args[1][2] == '/') {
    if (chdir(cmd->command_args[1]) != 0) {
        printf("cd: %s: No such file or directory\n", cmd->command_args[1]);
        return;
    }
  } else if (cmd->command_args[1][0] == '~') {
    const char* home_env = getenv("HOME");
    if (chdir(home_env) != 0) {
        printf("Cannot be taken home.");
        return;
    }
  }
  return;
}
    
  if (STRCMP(cmd->command_args[0], COMPLETE)) {
    static Registers* regis = { 0 };
    if (regis == NULL) regis = malloc(sizeof(Registers)); 

    if (STRCMP(cmd->command_args[1], "-p") && cmd->command_args[2]) {
        PullRegister(cmd->command_args[2], regis);
    }
    else if (STRCMP(cmd->command_args[1], "-C") && cmd->num_args > 2) {
        char* built_argv[NUM_COMMANDS * 4];
        built_argv[0] = strdup(cmd->command_args[cmd->num_args - 1]);
        built_argv[1] = strdup(cmd->command_args[2]);
        built_argv[2] = NULL;

        RegisterCommand(cmd->command_args[cmd->num_args - 1], cmd->command_args[2], regis, built_argv);
    }
    else if (STRCMP(cmd->command_args[1], "-r")) {
        register_command* reggi = CheckForRegister(cmd->command_args[2], regis);
        if (reggi != NULL) {
            bool removed = false;
            for (int i = 0; i < regis->index; i++) {
                if (STRCMP(regis->registers[i]->completer_script, reggi->completer_script)) {
                    removed = true;
                    FreeRegisters(regis, reggi);
                    continue;
                }
                if (removed) {
                    regis->registers[i - 1] = regis->registers[i];
                }
            }
            regis->index--;
            regis->registers[regis->index] = NULL;
        }
    }
    global_registration = regis;
    return;

    }

    if (STRCMP(cmd->command_args[0], JOBS))  {
        if (jobs_container == NULL) return;

        for (int i = 0; i < jobs_container->job_index; i++) {
            if (jobs_container->job_container[i]->job_status != Running) continue;
            bool exited = false;
            check_job_status(jobs_container->job_container[i], &exited);

            const char* marker = setMarker(jobs_container->job_container[i]);

            if (!exited) 
                printf("[%d]%s  %-24s %s\n", jobs_container->job_container[i]->job_num, marker, "Running", jobs_container->job_container[i]->current_job);
            else  
                reap_job(jobs_container->job_container[i]);
        }
        return;
    } 

    if (STRCMP(cmd->command_args[cmd->num_args - 1], "&")) {
        free(cmd->command_args[cmd->num_args - 1]); 
        cmd->command_args[cmd->num_args - 1] = NULL;
        cmd->num_args--;

        char* path = malloc(CHUNK * sizeof(char*));
        if (searchPathForFile(cmd->command_args[0], path, (CHUNK * sizeof(char*))) ) {
            if (!run_job(cmd, path)) {
                free(path);
                printf("Job failed to run\n");
                return;
            }
            printf("[%d] %zu\n", cmd->active_job->job_num, cmd->active_job->current_pid); 
            free(path);
        }
        else {
            printf("Could not find path to file\n");
            free(path);
            return;
        }
        return;
    }

    if (STRCMP(cmd->command_args[0], HISTORY)) {
        if (cmd->num_args == 1) {
            for (int i = 0; i < command_histories->index; i++) 
                printf("%3s %d %s\n", "", i + 1, command_histories->prev_commands[i]);
        } else if (cmd->num_args == 2) {
            int n = atoi(cmd->command_args[1]);
            int starting_index = (command_histories->index - n);
            for (int i = starting_index; i < command_histories->index; i++) 
                printf("%3s %d %s\n", "", i + 1, command_histories->prev_commands[i]);
        } else if (cmd->num_args == 3 && STRCMP(cmd->command_args[1], "-r")) {
            read_command_history(cmd->command_args[2]);
        } else if (cmd->num_args == 3 && STRCMP(cmd->command_args[1], "-w")) {
            write_command_history(cmd->command_args[2]);
        } else if (cmd->num_args == 3 && STRCMP(cmd->command_args[1], "-a")) {
            append_command_history(cmd->command_args[2]);
        }
        return;
    }

    if (STRCMP(cmd->command_args[0], DECLARE)) {
        if (variables_container == NULL) {
            variables_container = malloc(sizeof(variable_container));
            variables_container->declared_variables = malloc(sizeof(variable*) * CHUNK);
            variables_container->index = 0;
        }

        if (cmd->num_args == 2 && strchr(cmd->command_args[1], '=') != NULL) {
            if (atoi(&cmd->command_args[1][0]) != 0 || strchr(cmd->command_args[1], '-') != NULL) {
                printf("declare: `%s': not a valid identifier\n", cmd->command_args[1]);
                return;
            }
            variable* var = malloc(sizeof(variable));
            char* val = strchr(cmd->command_args[1], '=');

            var->name = strdup(cmd->command_args[1]);
            var->name[strcspn(var->name, "=")] = '\0';
            var->value = strdup(val + 1);

            variables_container->declared_variables[variables_container->index++] = var;
        } else if (cmd->num_args == 3 && STRCMP(cmd->command_args[1], "-p")) {
            for (int i = variables_container->index - 1; i >= 0; i--) {
                if (STRCMP(variables_container->declared_variables[i]->name, cmd->command_args[2])) {
                    printf("declare -- %s=\"%s\"\n", cmd->command_args[2], variables_container->declared_variables[i]->value);
                    return;
                }
            }
            printf("declare: %s: not found\n", cmd->command_args[2]);
        }
        return;
    }


    // When the command is unrecognizable must be an executable
    char* path = malloc(CHUNK * sizeof(char*));
    if (searchPathForFile(cmd->command_args[0], path, (CHUNK * (sizeof(char*))) )) {
        if (!execute_program(cmd, path, NULL)) 
            printf("The program: %s failed to execute", cmd->command_args[0]);
        free(path);
        return; 
    }

  free(path);
  printf("%s: command not found\n", cmd->raw_command);


 }

bool check_for_command(const struct Command* cmd) {
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (STRCMP(cmd->commands[i], cmd->command_args[1])) {
            printf("%s is a shell builtin\n", cmd->command_args[1]);
            return true;
        }
    } 
    return false;
}

void RegisterCommand(const char* cmdname, const char* script, Registers* regis, char** argv) {
    if (CheckForRegister(cmdname, regis) != NULL) return;

    register_command* reg = malloc(sizeof(register_command));
    reg->cmd_name = strdup(cmdname);
    reg->completer_script = strdup(script);
    reg->chopped_output[0] = NULL;
    reg->chopped_index = 0;

    reg->argvs = malloc(sizeof(char*) * (NUM_COMMANDS * 4));
    size_t i = 0;
    while (argv[i] != NULL) {
        reg->argvs[i] = argv[i];
        i++;
    }
    reg->argvs[4] = NULL;

    regis->registers[regis->index++] = reg;
    regis->registers[regis->index] = NULL;
}

void PullRegister(const char* cmdname, const Registers* regis) {
    if (regis) {
        const register_command* reg_command = CheckForRegister(cmdname, regis);
        if (reg_command != NULL) {
            printf("complete -C '%s' %s\n", reg_command->argvs[1], reg_command->cmd_name);
            return;
        }
        else {
            printf("complete: %s: no completion specification\n", cmdname);
        }
    }
    else {
        printf("Register has not been intialized.\n");
        exit(1);
    }
}



register_command* CheckForRegister(const char* cmdname, const Registers* regis) {
    if (regis) {
        for (int i = 0; i < regis->index; i++) {
            if (STRCMP(regis->registers[i]->cmd_name, cmdname))
                return regis->registers[i];
        }
        return NULL;
    }
    else if (STRCMP(cmdname, "complete")) {
        printf("[Register has not been intialized.]\n");
        exit(1);
    }
    return NULL;
}


bool ExecuteRegister(struct Command* cmd, const Registers* regis) {
    register_command* regis_command = (register_command*)CheckForRegister(cmd->command_args[0], regis);
    if (regis_command == NULL) {
        printf("\x07");
        return false;
    } 

    bool cache_valid = (regis_command->cached_line != NULL &&
                            STRCMP(regis_command->cached_line, cmd->raw_command));

    if (!cache_valid) return HandleCompletion(regis_command, cmd);
    else if (cache_valid && regis_command->chopped_index > 1)  return ComputeLCP(regis_command, true);
    else if (cache_valid && regis_command->chopped_index == 1) {
        size_t already_typed = strlen(regis_command->argvs[2]);
        if (cmd->num_args == 1) 
            printf("%s ", regis_command->chopped_output[0]);        
        else 
            printf("%s ", regis_command->chopped_output[0] + strlen(regis_command->argvs[2]));
        return true;
    }
    return true;
}   

bool HandleCompletion(register_command* regis_command, struct Command* cmd) {
    for (size_t i = 0; i < regis_command->chopped_index; i++) 
            free(regis_command->chopped_output[i]);
        regis_command->chopped_index = 0;
        regis_command->chopped_output[0] = NULL;

        regis_command->argvs[0] = "";  
        regis_command->argvs[1] =  strdup(regis_command->cmd_name);

        const char* current_word = "";  
        const char* prev_word = cmd->command_args[cmd->num_args - 1];

        if (cmd->raw_command[strlen(cmd->raw_command) - 1] != ' ') {
            current_word = cmd->command_args[cmd->num_args - 1];
            prev_word = (cmd->num_args >= 2) ? cmd->command_args[cmd->num_args - 2] : "";
        }

        regis_command->argvs[2] = strdup(current_word);
        regis_command->argvs[3] = strdup(prev_word);

        if (access(regis_command->completer_script, X_OK) != 0) return false; 
        if (!execute_program(cmd, (char*)regis_command->completer_script, regis_command)) return false;

        free(regis_command->cached_line);
        regis_command->cached_line = strdup(cmd->raw_command);

        if (regis_command->chopped_index == 1) {
            size_t already_typed = strlen(regis_command->argvs[2]);
            if (cmd->num_args == 1) 
                printf("%s ", regis_command->chopped_output[0]);        
            else 
                printf("%s ", regis_command->chopped_output[0] + strlen(regis_command->argvs[2]));
            return true;
        }
        else if (regis_command->chopped_index > 1) {
            return ComputeLCP(regis_command, false);
        }
        rl_ding();
        return true;
}

bool ComputeLCP(const register_command* regis_command, bool two_tabbed) {

    char* lcp = strdup(regis_command->chopped_output[0]);
    size_t lcp_len = strlen(lcp);

    for (size_t i = 0; i < regis_command->chopped_index; i++) {
        size_t j = 0;
        while (j < lcp_len && lcp[j] == regis_command->chopped_output[i][j]) 
            j++;
        lcp_len = j;
    }
    lcp[lcp_len] = '\0';

    size_t already_typed = strlen(regis_command->argvs[2]);
    if (lcp_len > already_typed && !two_tabbed) {
        rl_insert_text(lcp + already_typed);
        rl_point = rl_end;
    }
    else if (lcp_len <= already_typed && two_tabbed) {
        printf("\n");
        for (size_t i = 0; i < regis_command->chopped_index; i++) 
            printf("%-15s", regis_command->chopped_output[i]);
        printf("\n");
        rl_forced_update_display();
    }
    else {
        rl_ding();
    }

    free(lcp);
    return true;
}

void FreeRegisters(const Registers* regis, const register_command* null_regis) {
    if (!regis) return;

    if (null_regis != NULL) {
        free((void*)null_regis->cmd_name);
        free((void*)null_regis->completer_script);
        free((void*)null_regis->argvs);
        if (null_regis->output) {
            for (int i = 0; i < null_regis->chopped_index; i++) 
                free((void*)null_regis->chopped_output[i]);
            free((void*)null_regis->output);
        }
        free((void*)null_regis);
        return;
    }

    for (int i = 0; i < regis->index; i++) {
        free((void*)regis->registers[i]->cmd_name);
        free((void*)regis->registers[i]->completer_script);
        free((void*)regis->registers[i]->argvs);
        if (regis->registers[i]->output) {
            for (int i = 0; i < regis->registers[i]->chopped_index; i++) 
                free((void*)regis->registers[i]->chopped_output[i]);
            free((void*)regis->registers[i]->output);
        }
        free(regis->registers[i]);
    }
    free((void*)regis);
}

void FreeJobContainer() {
    if (jobs_container == NULL) return;
    for (int i = 0; i < jobs_container->job_index; i++) {
        free((void*)jobs_container->job_container[i]->current_job);
        free((void*)jobs_container->job_container[i]);
    }
    free(jobs_container);
}


void handleQuotes(struct Command* cmd) {
    char* echoStatement = cmd->raw_command;
    const size_t ECHO_SIZE = strlen(echoStatement) + 1;
    char current_token[ECHO_SIZE];
    char* token_array[ECHO_SIZE + 1];
    int current_token_pos = 0;
    int token_array_pos = 0;

    bool inQuotes = false;
    bool inDoubleQuotes = false;
    bool inBackslash = false;
    EmptyToken(current_token, ECHO_SIZE);
    
    for (char* c = echoStatement; *c != '\0'; c++) {
        if (inBackslash) {
            current_token[current_token_pos] = *c;
            current_token_pos++;
            inBackslash = false;
        }
        else if (*c == '"' && !inQuotes) {
            inDoubleQuotes = !inDoubleQuotes;
        }
        else if (*c == '\\' && (inDoubleQuotes == false && inQuotes == false)) {
            inBackslash = true;
        }
        else if (*c == '\\' && inDoubleQuotes == true) {
            inBackslash = true;
        }
        else if (*c == '\'' && !inDoubleQuotes) {
            inQuotes = !inQuotes;
        }
        else if (*c == ' ' && (inQuotes == true || inDoubleQuotes == true)) {
            current_token[current_token_pos] = *c;
            current_token_pos++;
        }
        else if (*c == ' ' && (inQuotes == false && inDoubleQuotes == false)) {
            if (!isTokenEmpty(current_token)) {
                token_array[token_array_pos] = strdup(current_token); // memory allocation
                token_array_pos++;
                current_token_pos = 0;
                EmptyToken(current_token, ECHO_SIZE);
            }
        }
        else {
            current_token[current_token_pos] = *c;
            current_token_pos++;
        }
    }

    if (!isTokenEmpty(current_token)) {
        token_array[token_array_pos] = strdup(current_token);
        token_array_pos++;
    }

    for (int i = 0; i < token_array_pos; i++) {
        cmd->command_args[i] = token_array[i]; 
    }

    cmd->command_args[token_array_pos] = NULL;
    cmd->num_args = token_array_pos;

    rebuild_raw_command(cmd);
}

void rebuild_raw_command(struct Command* cmd) {
    size_t total_len = 0;
    for (int i = 0; i < cmd->num_args; i++) {
        total_len += strlen(cmd->command_args[i]) + 1;
    }
    char* token_buffer = malloc(total_len + 1);
    size_t used = 0;
    for (int i = 0; i < cmd->num_args; i++) {
        int written = sprintf(token_buffer + used, "%s", cmd->command_args[i]);
        used += written;
        if (i < cmd->num_args - 1) {
            token_buffer[used] = ' ';
            used++;
        }
    }

    token_buffer[used] = '\0';
    cmd->raw_command = token_buffer;
}


void tokenize_args(struct Command* cmd, char* token_buffer) {
  char* p = token_buffer;
  int argindex = 0;

  while (*p) {
    while (*p == ' ') p++;
    if (!*p) break;

    const char* head = p;
    while (*p && *p != ' ')  p++;

    size_t len = p - head;

    cmd->command_args[argindex] = malloc(len + 1);

    if (cmd->command_args[argindex] == NULL) {
      printf("Failed to allocate memory for the command arguments");
      free(token_buffer);
      exit(1);
    }

    strncpy(cmd->command_args[argindex], head, len);
    cmd->command_args[argindex][len] = '\0';
    argindex++;
  }

  free(token_buffer);
  cmd->num_args = argindex;
}

bool searchPathForFile(const char* filename, char* result, size_t result_size) {
  const char* path_env = getenv("PATH");
  if (!path_env) return 0;

  char* path_copy = strdup(path_env); // memory string allocaiton
  if (!path_copy) return 0;

  char *dir = strtok(path_copy, ":");
  while (dir != NULL) {
    char full_path[ULTRA_CHUNK];
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, filename);

    if (access(full_path, X_OK) == 0) {
        strncpy(result, full_path, result_size - 1);
        result[result_size - 1] = '\0';
        free(path_copy);
        return 1;
    }

    dir = strtok(NULL, ":");
  }

  free(path_copy);
  return 0;
}



bool execute_program(const struct Command* cmd, char* path, register_command* registered_command) {
    char* mul_pipe = strchr(cmd->raw_command, '|');
    if (mul_pipe) {
        char*** argvs = NULL;        
        char** paths = NULL;        

        int ncmds = build_mult_pipe_arrays(cmd, &paths, &argvs);
        bool ok = execute_multiple_programs(cmd, paths, argvs, ncmds);

        for (int i = 0; i < ncmds; i++) {
            free(paths[i]);
            free(argvs[i]);
        }

        free(paths);
        free(argvs);

        return ok;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }

    char* re = strchr(cmd->raw_command, RE);
    char* re1 = strstr(cmd->raw_command, RE1);
    char* er = strstr(cmd->raw_command, ER);
    char* ap = strstr(cmd->raw_command, APP);
    char* ap1 = strstr(cmd->raw_command, APP1);
    char* aper = strstr(cmd->raw_command, APPER);
    bool readingIntoPipe = (re != NULL || re1 != NULL);
    bool readingErr = (er != NULL || aper != NULL);
    bool registered = (registered_command != NULL);

    if (readingErr) readingIntoPipe = false;

    if (registered) {
        char point_str[16];
        snprintf(point_str, sizeof(point_str), "%d", (int)strlen(cmd->raw_command));
        setenv("COMP_LINE", cmd->raw_command, 1);
        setenv("COMP_POINT", point_str, 1);
    }


    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 0;
    }

    else if (pid == 0 && (readingIntoPipe || readingErr)) {
        close(pipefd[0]);
        if (readingIntoPipe) dup2(pipefd[1], STDOUT_FILENO);
        else if (readingErr) dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        char* argv[CHUNK];
        BuildArgv(cmd, argv);

        execv(path, &argv[0]);
        perror("execv");
        _exit(1);
    }
    else if (pid == 0 && registered) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execv(path, &registered_command->argvs[0]);
        perror("execv");
        _exit(1);
    }
    else if (pid == 0 && !(readingIntoPipe && readingErr && registered)) {
        close(pipefd[0]);
        close(pipefd[1]);

        execv(path, &cmd->command_args[0]);
        perror("execv");
        _exit(1);
    }
    else {
        waitpid(pid, NULL, 0);
    }

    if (readingIntoPipe || readingErr || registered) {
        close(pipefd[1]);

        // reading into output
        size_t capacity = 4096;
        size_t used = 0;
        char* output = malloc(capacity);
        ssize_t n;
        char chunk[CHUNK];
        while ((n = read(pipefd[0], chunk, sizeof(chunk))) > 0) {
            if (used + n + 1 > capacity) {
                capacity *= 2;
                output = realloc(output, capacity);
            }
            memcpy(output + used, chunk, n);
            used += n;
        }
        output[used] = '\0';

        close(pipefd[0]);
        waitpid(pid, NULL, 0);

        if (registered) {
            char* p = output;
            while (*p != '\0') {
                char* s = p;
                while (*p != ' ' && *p != '\0' && *p != '\n') 
                    p++;
                if (p == NULL) break;

                size_t len = p - s;
                char* str;
                str = malloc(len + 1);
                strncpy(str, s, len);
                str[len] = '\0';

                if (!CandidateAlreadyExists(str, registered_command)) 
                    registered_command->chopped_output[registered_command->chopped_index++] = strdup(str);

                if (!CommandAlreadyExists(str)) {
                    global_completions->commands[global_completions->commands_size++] = strdup(str);
                    if (!global_completions->commands[global_completions->commands_size - 1]) {
                        printf("Memory could not be properly allocated to the completions commands.");
                        exit(1);
                    }
                }

                free(str);
                while (*p == ' ' || *p == '\n') 
                    p++;
                if (*p == '\0' || p == NULL) break;
            }
            global_completions->commands[global_completions->commands_size] = NULL;
            registered_command->chopped_output[registered_command->chopped_index] = NULL;
            SortCandidatesAlphabetically(registered_command);

            if (used > 0 && output[used - 1] == '\n') 
                output[used - 1] = '\0';

            registered_command->output = output;
            return 1;
        }

        // finding the file name
        const char* filename = findAndReturnFilename(cmd);
    
        // removing the newline char at the end
        if (used > 0 && output[used - 1] == '\n') 
            output[used - 1] = ' ';

        bool append = (ap != NULL || ap1 != NULL || aper != NULL);
        if (append) {
            OpenAndAppendToFile(filename, output);
            free(output);
            return 1;
        }
        OpenAndWriteToFile(filename, output);
        free(output);
        return 1;
    }
    return 1;
}

int build_mult_pipe_arrays(const struct Command* cmd, char*** out_paths, char**** out_argvs) {
    int ncmds = 1;
    for (int i = 0; i < cmd->num_args; i++) {
        if (STRCMP(cmd->command_args[i], "|")) ncmds++;
    }

    char*** argvs = malloc(sizeof(char**) * ncmds);
    char** paths = malloc(ncmds * sizeof(char*));

    int c = 0, w = 0;
    argvs[0] = malloc(sizeof(char*) * (cmd->num_args + 1));

    for (int i = 0; i < cmd->num_args; i++) {
        if (STRCMP(cmd->command_args[i], "|")) {
            argvs[c][w] = NULL;
            c++;
            w = 0;
            argvs[c] = malloc(sizeof(char*) * (cmd->num_args + 1));
            continue;
        }
        argvs[c][w++] = cmd->command_args[i];
    }
    argvs[c][w] = NULL;


    for (int i = 0; i < ncmds; i++) {
        paths[i] = NULL;
        if (argvs[i][0] == NULL) continue;

        char* buf = malloc(CHUNK);
        if (!searchPathForFile(argvs[i][0], buf, (sizeof(char) * CHUNK) )) {
            free(buf);
        } else {
            paths[i] = buf;
        }
    }

    *out_paths = paths;
    *out_argvs = argvs;

    return ncmds;
}

bool execute_multiple_programs(const struct Command* cmd, char** paths, char*** argvs, int ncmds) {
    int n_pipes = ncmds - 1;
    int pipefd[n_pipes > 0 ? n_pipes : 1][2];

    for (int i = 0; i < n_pipes; i++) {
        if (pipe(pipefd[i]) == -1) {
            perror("pipe");
            for (int k = 0; k < i; k++) { close(pipefd[k][0]); close(pipefd[k][1]); }
            return false;
        }
    }

    pid_t pids[ncmds];
    int started = 0;

    for (int i = 0; i < ncmds; i++)  {   
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            break;
        }

        if (pid == 0) {
            if (i > 0) dup2(pipefd[i - 1][0], STDIN_FILENO);
            if (i < ncmds - 1) dup2(pipefd[i][1], STDOUT_FILENO);

            for (int k = 0; k < n_pipes; k++) {
                close(pipefd[k][0]);
                close(pipefd[k][1]);
            }

            if (paths[i] != NULL) {
                execv(paths[i], &argvs[i][0]);
                perror("execv");
                _exit(1);
            } else {
                struct Command fake_cmd = {0};
                int argc = 0;
                while (argvs[i][argc] != NULL) argc++;

                for (int j = 0; j < argc; j++) fake_cmd.command_args[j] = argvs[i][j];
                fake_cmd.command_args[argc] = NULL;
                fake_cmd.num_args = argc;

                size_t raw_cap = CHUNK;
                char* raw_buf = malloc(raw_cap);
                size_t used = 0;
                for (int j = 0; j < argc; j++) {
                    int written = snprintf(raw_buf + used, raw_cap - used, "%s", argvs[i][j]);
                    used += written;
                    if (j < argc - 1) raw_buf[used++] = ' ';
                }
                raw_buf[used] = '\0';
                fake_cmd.raw_command = raw_buf;

                execute_command(&fake_cmd);
                fflush(stdout);
                _exit(0);            
            }
        }
        pids[started++] = pid;
    }

    for (int k = 0; k < n_pipes; k++) {
        close(pipefd[k][0]);
        close(pipefd[k][1]);
    }
    for (int i = 0; i < started; i++) 
        waitpid(pids[i], NULL, 0);

    return started == ncmds;
}

bool run_job(const struct Command* cmd, const char* path) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 0;
    }

    if (pid == 0) {
        close(pipefd[0]);
        close(pipefd[1]);

        execv(path, &cmd->command_args[0]);
        perror("execv");
        _exit(1);
    }

      if (jobs_container == NULL) {
        jobs_container = malloc(sizeof(*jobs_container));
        jobs_container->job_index = 0;
    }

    cmd->active_job->current_job = strdup(cmd->raw_command);
    cmd->active_job->current_pid = pid;
    cmd->active_job->job_status = Running;

    int num_jobs = 0;
    for (int i = 0; i < jobs_container->job_index; i++) 
        if (jobs_container->job_container[i]->job_status == Running) num_jobs++; 
     
    if (num_jobs < 1) 
        jobs_container->current_job_num = 1;

    cmd->active_job->job_num = jobs_container->current_job_num++;
    push_job(cmd->active_job);
     
    return true;
}

bool CheckForMultiPipe(const struct Command* cmd) {
    bool built_in = false;
    for (int i = 0; i < cmd->num_args; i++) {
        for (int j = 0; j < NUM_COMMANDS; j++) {
            if (STRCMP(cmd->command_args[i], cmd->commands[j])) {
                built_in = true;
                break;
            }
        }
        if (built_in) break;
    }

    if (built_in) {
        char*** argvs = NULL;
        char** paths = NULL;        

        int ncmds = build_mult_pipe_arrays(cmd, &paths, &argvs);
        bool ok = execute_multiple_programs(cmd, paths, argvs, ncmds);

        for (int i = 0; i < ncmds; i++) {
            free(paths[i]);
            free(argvs[i]);
        }

        free(paths);
        free(argvs);
        return ok;
    }
    else 
        return false;
}

const char* setMarker(const jobs* job) {
    int start_index = jobs_container->job_index - 1;
    const char* marker;
    for (int i = start_index; i >= 0; i--) {
        if (jobs_container->job_container[i]->job_status != Running) continue;
        if (jobs_container->job_container[i]->current_pid == job->current_pid && jobs_container->job_container[i]->job_status == Running) {
            marker = "+";
            return marker;
        }
        start_index = i;
        break;
    }
    for (int i = start_index - 1; i >= 0; i--) {
        if (jobs_container->job_container[i]->job_status != Running) continue;
        if (jobs_container->job_container[i]->current_pid == job->current_pid && jobs_container->job_container[i]->job_status == Running) {
            marker = "-";
            return marker;
        }
        break;
    }

    marker = " ";
    return marker;
}

bool isTokenEmpty(const char* token) {
    if (token[0] == '\0') 
        return true;
    return false;
}

void EmptyToken(char* token, size_t token_length) {
    for (int i = 0; i < token_length; i++) 
        token[i] = '\0';
}

char* findAndReturnFilename(const struct Command* cmd) {
    char* file_name = NULL;
    for (int i = 0; i < cmd->num_args; i++) {
        if (STRCMP(cmd->command_args[i], RESTR) || STRCMP(cmd->command_args[i], RE1) || STRCMP(cmd->command_args[i], APP) || STRCMP(cmd->command_args[i], APP1) || STRCMP(cmd->command_args[i], "2>") || STRCMP(cmd->command_args[i], APPER)) {
            if (cmd->command_args[i + 1] != NULL) {
                    file_name = cmd->command_args[i + 1];
                    break;
                }
        }
    }
    return file_name;
}

void BuildCompletionsArray(completion_candidates* comp_cand) {

    comp_cand->commands[comp_cand->commands_size++] = strdup(ECHO);
    comp_cand->commands[comp_cand->commands_size++] = strdup(EXIT);
    comp_cand->commands[comp_cand->commands_size++] = strdup(TYPE);
    comp_cand->commands[comp_cand->commands_size++] = strdup(PWD);
    comp_cand->commands[comp_cand->commands_size++] = strdup(CD);
    comp_cand->commands[comp_cand->commands_size++] = strdup(COMPLETE);

    AppendExecutables(comp_cand);
    comp_cand->commands[comp_cand->commands_size] = NULL;
}

completion_candidates* InitCompletions() {
    completion_candidates* comp_cand = malloc(sizeof(completion_candidates));
    comp_cand->commands_size = 0;
    return comp_cand;
}

bool CommandAlreadyExists(const char* str) {
    for (size_t i = 0; i < global_completions->commands_size; i++) {
        if (STRCMP(global_completions->commands[i], str)) 
            return true;
    }
    return false;
}

bool CandidateAlreadyExists(const char* str, const register_command* reg) {
    for (size_t i = 0; i < reg->chopped_index; i++) {
        if (STRCMP(str, reg->chopped_output[i])) {
            return true;
        }
    }
    return false;
}

void FreeCompletions(completion_candidates* comps) {
    for (int i = 0; i < comps->commands_size; i++) 
        free(comps->commands[i]);
    free(comps);
}

void FreeHistories() {
    for (int i = 0; i < command_histories->index; i++) 
        free((void*)command_histories->prev_commands[i]);
    free(command_histories);
}

void AppendExecutables(completion_candidates* comp_cand) {
    const char* path_env = getenv("PATH");
    if (!path_env) printf("Path is NULL!");
    char* path_copy = strdup(path_env);
    if (!path_copy) printf("Path copy is NULL!");

    char* dir_path = strtok(path_copy, ":");
    while (dir_path != NULL) {
        DIR* dir = opendir(dir_path);
        if (dir != NULL) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
                    continue;
                char full_path[2048];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
                struct stat st;
                if (access(full_path, X_OK) == 0 && stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) 
                    comp_cand->commands[comp_cand->commands_size++] = strdup(entry->d_name);
            }
            closedir(dir);
        }
        dir_path = strtok(NULL, ":");
    }
    free(path_copy);
}

void ExpandVariables(struct Command* cmd) {
    for (int i = 0; i < cmd->num_args; i++) {
        char* arg = cmd->command_args[i];
        if (strchr(arg, '$') == NULL) continue;

        size_t cap = strlen(arg) * 2 + 64;
        char* result = malloc(cap);
        size_t rlen = 0;

        char* p = arg;
        while (*p) {
            if (*p == '$' && *(p + 1) == '{') {
                char* end = strchr(p + 2, '}');
                if (end != NULL) {
                    size_t namelen = end - (p + 2);
                    char varname[CHUNK];
                    strncpy(varname, p + 2, namelen);
                    varname[namelen] = '\0';

                    const char* value = NULL;
                    if (variables_container != NULL) {
                        for (int j = 0; j < variables_container->index; j++) {
                            if (STRCMP(variables_container->declared_variables[j]->name, varname)) {
                                value = variables_container->declared_variables[j]->value;
                                break;
                            }
                        }
                    }

                    if (value != NULL) {
                        size_t vlen = strlen(value);
                        if (rlen + vlen + 1 > cap) {
                            cap = (rlen + vlen + 1) * 2;
                            result = realloc(result, cap);
                        }
                        memcpy(result + rlen, value, vlen);
                        rlen += vlen;
                    }

                    p = end + 1;
                    continue;
                }
            }
            else if (*p == '$' && (isalpha((unsigned char)*(p + 1)) || *(p + 1) == '_')) {
                char* name_start = p + 1;
                char* q = name_start;
                while (isalnum((unsigned char)*q) || *q == '_') q++;

                size_t namelen = q - name_start;
                char varname[CHUNK];
                strncpy(varname, name_start, namelen);
                varname[namelen] = '\0';

                const char* value = "";
                if (variables_container != NULL) {
                    for (int j = 0; j < variables_container->index; j++) {
                        if (STRCMP(variables_container->declared_variables[j]->name, varname)) {
                            value = variables_container->declared_variables[j]->value;
                            break;
                        }
                    }
                }

                size_t vlen = strlen(value);
                if (rlen + vlen + 1 > cap) {
                    cap = (rlen + vlen + 1) * 2;
                    result = realloc(result, cap);
                }
                memcpy(result + rlen, value, vlen);
                rlen += vlen;

                p = q;
                continue;
            }

            if (rlen + 2 > cap) {
                cap *= 2;
                result = realloc(result, cap);
            }
            result[rlen++] = *p;
            p++;
        }
        result[rlen] = '\0';

        free(cmd->command_args[i]);
        cmd->command_args[i] = result;
    }

    int write_idx = 0;
    for (int read_idx = 0; read_idx < cmd->num_args; read_idx++) {
        if (cmd->command_args[read_idx][0] == '\0') {
            free(cmd->command_args[read_idx]);
            continue;
        }
        cmd->command_args[write_idx] = cmd->command_args[read_idx];
        write_idx++;
    }
    cmd->num_args = write_idx;
    cmd->command_args[cmd->num_args] = NULL;

    free(cmd->raw_command);
    rebuild_raw_command(cmd);
}


bool free_allocations(struct Command* cmd) {
   if (cmd == NULL) return false;

    for (int i = 0; i < cmd->num_args; i++) {
        if (cmd->command_args[i] != NULL) 
            free(cmd->command_args[i]);
    }

   if (cmd->active_job->current_job != NULL) free((void*)cmd->active_job->current_job);
   free(cmd->active_job);
   free(cmd);

  return true;
}
