#include "command.h"

char **shell_completion(const char* text, int start, int end);
char *command_generator(const char* text, int state);
void my_display_matches(char** matches, int num_matches, int max_length);
void RunProgram();
void post_command_reap();

void RunProgram() {
  rl_attempted_completion_function = shell_completion;
  rl_completion_display_matches_hook = my_display_matches;

  global_completions = InitCompletions();
  BuildCompletionsArray(global_completions);

  using_history();
  const char* histfile = getenv("HISTFILE");

  if (histfile != NULL) 
      read_command_history(histfile);

  while (running) {
    setbuf(stdout, NULL);

    char* command = readline("$ ");
    if (command == NULL) {
        printf("Input ended up NULL, terminating the program.\n");
        exit(1);
    }
    command[strcspn(command, "\n")] = '\0';
    add_history(command);

    struct Command* cmd = malloc(sizeof(struct Command));
    init_command(cmd, command);
    execute_command(cmd);
    post_command_reap();

    if (!free_allocations(cmd)) {
        printf("Error freeing allocations.\n");
        exit(1);
    }
  }

  if (histfile != NULL)
      write_command_history(histfile);

  FreeRegisters(global_registration, NULL);
  FreeCompletions(global_completions);
  FreeJobContainer();
  FreeHistories();
}

void post_command_reap() {
    if (jobs_container == NULL) return;

    for (int i = 0; i < jobs_container->job_index; i++) {
      bool exited = false;
      check_job_status(jobs_container->job_container[i], &exited);
      const char* marker = setMarker(jobs_container->job_container[i]);
      if (exited) 
        reap_job(jobs_container->job_container[i]);
    }
}

char **shell_completion(const char* text, int start, int end) { // on tab pressed
  (void)end;

  if (rl_line_buffer != NULL && strchr(rl_line_buffer, ' ') != NULL) {
    char* command = strdup(rl_line_buffer);
    struct Command* cmd = malloc(sizeof(struct Command));
    memset(cmd->command_args, 0, sizeof cmd->command_args);
    init_command(cmd, command);

    register_command* regis_command = CheckForRegister(cmd->command_args[0], global_registration);
    if (regis_command != NULL) {
        ExecuteRegister(cmd, global_registration);  
        free(command);
        free_allocations(cmd);

        rl_attempted_completion_over = 1;
        rl_completion_suppress_append = 1;
        char** empty = malloc(sizeof(char*) * 2);
        empty[0] = strdup("");         
        empty[1] = NULL;
        return empty;
      }

      free(command);
      free_allocations(cmd);
    }

  if (start == 0) {
    return rl_completion_matches(text, command_generator);
  }

  return NULL;
}

char *command_generator(const char* text, int state) {
  static int list_index;
  static size_t len;

  if (state == 0) {
    list_index = 0;
    len = strlen(text);
  }

  while (global_completions->commands[list_index]) {
    const char* name = global_completions->commands[list_index++];
    if (strncmp(name, text, len) == 0) {
      return strdup(name);
    }
}

  return NULL;
}
// Probably remove this cuz we dont really need it anymore
void my_display_matches(char** matches, int num_matches, int max_length) {
  (void)max_length;
  printf("\n");

  for (int i = 1; i <= num_matches; i++) {
    struct stat st;
    char display_name[CHUNK];

    if (stat(matches[i], &st) == 0 && S_ISDIR(st.st_mode)) {
      snprintf(display_name, sizeof(display_name), "%s/", matches[i]);
    } else {
      snprintf(display_name, sizeof(display_name), "%s", matches[i]);
    }

    printf("%-15s", display_name);

  }
  printf("\n");
  rl_forced_update_display();
}
