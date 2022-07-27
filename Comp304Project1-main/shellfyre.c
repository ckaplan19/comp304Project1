#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>

//Maltinsoy19
//Ckaplan19

const char * sysname = "shellfyre";

bool flag=true;
bool filemade=false;

char cwdInit[100];

int counter = 0;
char arr[10][100];  //array to hold last visited directories.

enum return_codes {
  SUCCESS = 0,
    EXIT = 1,
    UNKNOWN = 2,
};

struct command_t

{

  char * name;
  bool background;
  bool auto_complete;
  int arg_count;
  char ** args;
  char * redirects[3]; // in/out redirection
  struct command_t * next; // for piping
};

/**
 * Prints a command struct
 * @param struct command_t *
 */
void print_command(struct command_t * command)

{
  int i = 0;
  printf("Command: <%s>\n", command -> name);
  printf("\tIs Background: %s\n", command -> background ? "yes" : "no");
  printf("\tNeeds Auto-complete: %s\n", command -> auto_complete ? "yes" : "no");
  printf("\tRedirects:\n");
  for (i = 0; i < 3; i++)

    printf("\t\t%d: %s\n", i, command -> redirects[i] ? command -> redirects[i] : "N/A");

  printf("\tArguments (%d):\n", command -> arg_count);

  for (i = 0; i < command -> arg_count; ++i)

    printf("\t\tArg %d: %s\n", i, command -> args[i]);

  if (command -> next)

  {

    printf("\tPiped to:\n");

    print_command(command -> next);

  }

}

/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t * command)

{

  if (command -> arg_count)

  {

    for (int i = 0; i < command -> arg_count; ++i)
      free(command -> args[i]);
    free(command -> args);
  }

  for (int i = 0; i < 3; ++i)

    if (command -> redirects[i])

      free(command -> redirects[i]);

  if (command -> next)

  {

    free_command(command -> next);

    command -> next = NULL;

  }

  free(command -> name);

  free(command);

  return 0;

}

/**

 * Show the command prompt

 * @return [description]

 */

int show_prompt()

{

  char cwd[1024], hostname[1024];

  gethostname(hostname, sizeof(hostname));

  getcwd(cwd, sizeof(cwd));

  printf("%s@%s:%s %s$ ", getenv("USER"), hostname, cwd, sysname);

  return 0;

}
/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */

int parse_command(char * buf, struct command_t * command) {

  const char * splitters = " \t"; // split at whitespace
  int index, len;
  len = strlen(buf);
  while (len > 0 && strchr(splitters, buf[0]) != NULL) // trim left whitespace
  {

    buf++;

    len--;

  }
  while (len > 0 && strchr(splitters, buf[len - 1]) != NULL)
    buf[--len] = 0; // trim right whitespace
  if (len > 0 && buf[len - 1] == '?') // auto-complete
    command -> auto_complete = true;
  if (len > 0 && buf[len - 1] == '&') // background
    command -> background = true;
  char * pch = strtok(buf, splitters);
  command -> name = (char * ) malloc(strlen(pch) + 1);
  if (pch == NULL)

    command -> name[0] = 0;
  else
    strcpy(command -> name, pch);
  command -> args = (char ** ) malloc(sizeof(char * ));
  int redirect_index;
  int arg_index = 0;
  char temp_buf[1024], * arg;
  while (1)

  {

    // tokenize input on splitters
    pch = strtok(NULL, splitters);
    if (!pch)
      break;
    arg = temp_buf;
    strcpy(arg, pch);
    len = strlen(arg);
    if (len == 0)
      continue; // empty arg, go for next
    while (len > 0 && strchr(splitters, arg[0]) != NULL) // trim left whitespace
    {

      arg++;

      len--;

    }

    while (len > 0 && strchr(splitters, arg[len - 1]) != NULL)

      arg[--len] = 0; // trim right whitespace

    if (len == 0)

      continue; // empty arg, go for next
    // piping to another command
    if (strcmp(arg, "|") == 0) {
      struct command_t * c = malloc(sizeof(struct command_t));
      int l = strlen(pch);
      pch[l] = splitters[0]; // restore strtok termination
      index = 1;
      while (pch[index] == ' ' || pch[index] == '\t')
        index++; // skip whitespaces
      parse_command(pch + index, c);
      pch[l] = 0; // put back strtok termination
      command -> next = c;
      continue;
    }

    // background process

    if (strcmp(arg, "&") == 0)

      continue; // handled before
    // handle input redirection
    redirect_index = -1;
    if (arg[0] == '<')
      redirect_index = 0;
    if (arg[0] == '>') {
      if (len > 1 && arg[1] == '>') {
        redirect_index = 2;
        arg++;
        len--;
      } else

        redirect_index = 1;

    }

    if (redirect_index != -1)

    {

      command -> redirects[redirect_index] = malloc(len);

      strcpy(command -> redirects[redirect_index], arg + 1);

      continue;

    }

    // normal arguments

    if (len > 2 && ((arg[0] == '"' && arg[len - 1] == '"') || (arg[0] == '\'' && arg[len - 1] == '\''))) // quote wrapped arg

    {

      arg[--len] = 0;

      arg++;

    }

    command -> args = (char ** ) realloc(command -> args, sizeof(char * ) * (arg_index + 1));

    command -> args[arg_index] = (char * ) malloc(len + 1);

    strcpy(command -> args[arg_index++], arg);

  }

  command -> arg_count = arg_index;

  return 0;

}

void prompt_backspace()

{

  putchar(8); // go back 1

  putchar(' '); // write empty over

  putchar(8); // go back 1 again

}

/**

 * Prompt a command from the user

 * @param  buf      [description]

 * @param  buf_size [description]

 * @return          [description]

 */

int prompt(struct command_t * command)

{

  int index = 0;

  char c;

  char buf[4096];

  static char oldbuf[4096];

  // tcgetattr gets the parameters of the current terminal

  // STDIN_FILENO will tell tcgetattr that it should write the settings

  // of stdin to oldt

  static struct termios backup_termios, new_termios;

  tcgetattr(STDIN_FILENO, & backup_termios);

  new_termios = backup_termios;

  // ICANON normally takes care that one line at a time will be processed

  // that means it will return if it sees a "\n" or an EOF or an EOL

  new_termios.c_lflag &= ~(ICANON | ECHO); // Also disable automatic echo. We manually echo each char.

  // Those new settings will be set to STDIN

  // TCSANOW tells tcsetattr to change attributes immediately.

  tcsetattr(STDIN_FILENO, TCSANOW, & new_termios);

  // FIXME: backspace is applied before printing chars

  show_prompt();
  int multicode_state = 0;
  buf[0] = 0;
  while (1)

  {

    c = getchar();

    // printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

    if (c == 9) // handle tab
    {
      buf[index++] = '?'; // autocomplete
      break;
    }
    if (c == 127) // handle backspace
    {
      if (index > 0) {
        prompt_backspace();
        index--;
      }
      continue;
    }
    if (c == 27 && multicode_state == 0) // handle multi-code keys
    {

      multicode_state = 1;

      continue;

    }

    if (c == 91 && multicode_state == 1)

    {

      multicode_state = 2;

      continue;

    }

    if (c == 65 && multicode_state == 2) // up arrow

    {

      int i;

      while (index > 0)

      {

        prompt_backspace();

        index--;

      }

      for (i = 0; oldbuf[i]; ++i)

      {

        putchar(oldbuf[i]);

        buf[i] = oldbuf[i];

      }

      index = i;

      continue;

    } else

      multicode_state = 0;

    putchar(c); // echo the character

    buf[index++] = c;

    if (index >= sizeof(buf) - 1)

      break;

    if (c == '\n') // enter key

      break;

    if (c == 4) // Ctrl+D

      return EXIT;

  }

  if (index > 0 && buf[index - 1] == '\n') // trim newline from the end

    index--;

  buf[index++] = 0; // null terminate string

  strcpy(oldbuf, buf);

  parse_command(buf, command);

  // print_command(command); // DEBUG: uncomment for debugging

  // restore the old settings

  tcsetattr(STDIN_FILENO, TCSANOW, & backup_termios);

  return SUCCESS;

}

int process_command(struct command_t * command);

int main()

{

  while (1)

  {

    struct command_t * command = malloc(sizeof(struct command_t));

    memset(command, 0, sizeof(struct command_t)); // set all bytes to 0

    int code;

    code = prompt(command);

    if (code == EXIT)

      break;

    code = process_command(command);

    if (code == EXIT)

      break;

    free_command(command);

  }

  printf("\n");

  return 0;

}



void listFiles(const char * dirname, struct command_t * command) { //list files recursively and read if we have the -r and desired file name.
    char cwddir1[100];
    getcwd(cwddir1, sizeof(cwddir1));
  DIR * directory = opendir(dirname);
  if (directory == NULL) {
    return;
  }
  struct dirent * temporary;
  temporary = readdir(directory);
  while (temporary != NULL) {
    char * control;
    control = strstr(temporary -> d_name, command -> args[2]); // Here arc count =4 if we entered 3 command to terminal
    if (control) {
        char str[100];
        strtok(str, "/");
        strcat(str,cwddir1);
        dirname = dirname + 1;
        strcat(str,dirname);
        dirname = dirname - 1;
        strcat(str,"/");
        strcat(str,temporary -> d_name);
        printf("\n");
	char * pointer;
	int ch = '/';
	pointer = strchr( str, ch );
	printf("%s\n", pointer);
    }
    if (temporary -> d_type == DT_DIR && strcmp(temporary -> d_name, ".") != 0 && strcmp(temporary -> d_name, "..") != 0) {
      char path[100] = {
        0
      };
      strcat(path, dirname);
      strcat(path, "/");
      strcat(path, temporary -> d_name);
      listFiles(path, command);
    }
    temporary = readdir(directory);
  }
  closedir(directory);
}

void listFilesOpen(const char * dirname, struct command_t * command) { //list files recursively and open them all. This is the same method we used in -r implemenation with xdg added to open files
    char cwddir1[100];
    getcwd(cwddir1, sizeof(cwddir1));
  DIR * directory = opendir(dirname);
  if (directory == NULL) {
    return;
  }
  struct dirent * temporary;
  temporary = readdir(directory);
  while (temporary != NULL) {
    char * control;
    control = strstr(temporary -> d_name, command -> args[2]); // Here arc count =4 if we entered 3 command to terminal
    if (control) {
        char str[100];
        strtok(str, "/");
        strcat(str,cwddir1);
        dirname = dirname + 1;
        strcat(str,dirname);
        dirname = dirname - 1;
        strcat(str,"/");
        strcat(str,temporary -> d_name);
        printf("\n");
	char * pointer;
	int ch = '/';
	pointer = strchr( str, ch );
	printf("%s\n", pointer);
	char * openCommand = malloc(sizeof(char) * (strlen(temporary->d_name) + strlen(sysname) + strlen("xdg-open ") + 1));
	strcpy(openCommand, "xdg-open ");
	strcat(openCommand, temporary->d_name);
	system(openCommand);
    }
    if (temporary -> d_type == DT_DIR && strcmp(temporary -> d_name, ".") != 0 && strcmp(temporary -> d_name, "..") != 0) {
      char path[100] = {
        0
      };
      strcat(path, dirname);
      strcat(path, "/");
      strcat(path, temporary -> d_name);
      listFilesOpen(path, command);
    }
    temporary = readdir(directory);
  }
  closedir(directory);
}


    int process_command(struct command_t * command)  {
        if(flag==true){
      
       getcwd(cwdInit, sizeof(cwdInit));
       flag=false;
        }

  int Dir;

  if (strcmp(command -> name, "") == 0)

    return SUCCESS;

    if (strcmp(command -> name, "exit") == 0){
       
          return EXIT;
      }
        

  if (strcmp(command -> name, "cd") == 0) {   //I write to array each time cd is called for cdh command and keep track of 10 recent directories with modulo 10.
    if (command -> arg_count > 0)
    {
      Dir = chdir(command -> args[0]);
      if (Dir == -1){
        printf("ERROR");
      }
      char cwd[200];
      if (getcwd(cwd, sizeof(cwd)) != NULL) {
        //printf("Current working dir: %s\n", cwd);   // current directoryyi bulmak icin kısa fonksiyon bunlar 2 boyutlu arrayda tutulacak.
      }
      int indexNumber= counter%10;
        char counterformtxt[1];
      strcpy(arr[indexNumber], cwd);
      counter++;
    }

  }

  // TODO: Implement your custom commands here
         
    if(strcmp(command->name, "weather") == 0) {   //gets weather based on location
    
    pid_t pidWeather;
    
    pidWeather = fork();
     
    if (pidWeather < 0) {
    fprintf(stderr, "Weather fork Failed");
    return 1;
    }else if(pidWeather > 0){
    wait(NULL);
    }else{
    char loc[15];
    char curl[] = "curl";
    char weather[20];
    printf("Please specify the city you want to get information about the weather.\n");
    printf("Type 'O' to be informed based on your current location.\n");
    scanf("%s", loc);
    if(strcmp(loc, "O") == 0){
    strcat(weather, "wttr.in/");
    }else{
    strcat(weather, "wttr.in/");
    strcat(weather, loc);
    }
    char *command[2] = {curl, weather};
    execlp(command[0], command[0], command[1], NULL);
    }
    }
    
    if(strcmp(command->name, "personality") == 0) {

    printf("This is a personality test that determines if you are artistic or analytic.\n");

    int pipeP1[2];
    int pipeP2[2];
    
    pid_t pidPersonality;
    
    if (pipe(pipeP1) == -1)
    { // fail case
    fprintf(stderr, "Personality Pipe 1 Failed");
    return 1;
    }
    if (pipe(pipeP2) == -1)
    { // fail case
    fprintf(stderr, "Personality Pipe 2 Failed");
    return 1;
    }
    
    pidPersonality = fork();
     
    if (pidPersonality < 0) {
    fprintf(stderr, "Personality fork Failed");
    return 1;
    }



    if(pidPersonality==0){
    char q1[] = ("On a scale from 1 to 10, how much do you like working on history?");
    char q2[] = ("On a scale from 1 to 10, how much do you like working on mathematics?");
    char q3[] = ("On a scale from 1 to 10, how much do you like working on literature?");
    char q4[] = ("On a scale from 1 to 10, how much do you like working on physcis?");
    char q5[] = ("On a scale from 1 to 10, how much do you like working on music?");
    char q6[] = ("On a scale from 1 to 10, how much do you like working on chemistry?");
    char q7[] = ("On a scale from 1 to 10, how much do you like working on philosophy?");
    char q8[] = ("On a scale from 1 to 10, how much do you like working on biology?");
    char q9[] = ("On a scale from 1 to 10, how much do you like working on visual arts?");
    char q10[] = ("On a scale from 1 to 10, how much do you like working on astronomy?");
    int answer = 0;
    int *score[2] = {0, 0};

    printf("%s\n",q1);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[0] = answer + score[0];
    printf("%s\n",q2);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[1] = answer + score[1];
    printf("%s\n",q3);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[0] = answer + score[0];
    printf("%s\n",q4);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[1] = answer + score[1];
    printf("%s\n",q5);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[0] = answer + score[0];
    printf("%s\n",q6);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[1] = answer + score[1];
    printf("%s\n",q7);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[0] = answer + score[0];
    printf("%s\n",q8);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[1] = answer + score[1];
    printf("%s\n",q9);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[0] = answer + score[0];
    printf("%s\n",q10);
    scanf("%d", &answer);
    if((answer < 1) || (answer > 10)){
    printf("Your answer should be between 1-10.\n");
    scanf("%d", &answer);
    }
    score[1] = answer + score[1];
        
    write(pipeP1[1], &score[0], sizeof(int));
    write(pipeP2[1], &score[1], sizeof(int));
    
    exit(0);
    }else if(pidPersonality > 0){
    
    wait(NULL);
    
    close(pipeP1[1]);
    close(pipeP2[1]);
    
    int artistic;
    int analytic;
    
    read(pipeP1[0], &artistic, sizeof(int));
    read(pipeP2[0], &analytic, sizeof(int));
  
    if(artistic == analytic){
    printf("Congratulations, you are an all rounder! You are equally interested in both analytic and artistic subjects.\n");
    }else if(artistic > analytic){
    printf("Congratulations, you are an artistic person!\n");
    }else {
    printf("Congratulations, you are an analytic person!\n");
    }
    
    close(pipeP1[0]);
    close(pipeP2[0]);
    }
    }
  
  
    if (strcmp(command->name, "joker") == 0) {
        FILE *file = fopen("CR_joke.txt", "w");

        fputs("*/1 * * * * XDG_RUNTIME_DIR=/run/user/$(id -u) notify-send Joke \"$(/usr/bin/curl -s https://icanhazdadjoke.com/)\"\n", file);
	fclose(file);
	char *args[] = {"crontab", "CR_joke.txt", NULL};
	char *absPath = "/usr/bin/crontab";
	pid_t pidJoker = fork();
	if (pidJoker == 0) {
	    execv(absPath, args);
	} else {
	    wait(NULL);
	}
        }
   
  if (strcmp(command -> name, "take") == 0){ // take command implementation
      // increase args size by 2
      command -> args = (char ** ) realloc(
        command -> args, sizeof(char * ) * (command -> arg_count += 2));
      // shift everything forward by 1
      for (int i = command -> arg_count - 2; i > 0; --i){
        command -> args[i] = command -> args[i - 1];
      }
      // set args[0] as a copy of name
      command -> args[0] = strdup(command -> name);
      // set args[arg_count-1] (last) to NULL
      command -> args[command -> arg_count - 1] = NULL;
      char input[500];
        // C programming      char input[30] ;
      strcpy(input, command->args[1]);  // args 1 is ex. A/B/C
     // printf("command wanted to be printed isssss %s \n ",command->args[2]);
     // printf("command wanted to be printed is inputttttt %s \n ",input);
      char * pointer = strtok(input, "/");
      int directoryCounter;
      while(pointer!=NULL){
          DIR * directory;
          directoryCounter=0;
          struct dirent * dir;
          directory = opendir(".");
          if (directory) {
            while ((dir = readdir(directory)) != NULL) {
              char * control;
              control = strstr(dir -> d_name, pointer); // Here arc count =3
              if (control) {
              //  printf(" DIRECTORY EXISTS%s\n", dir -> d_name);   we do nothing if directory exists.
              }
              else{
                  if(directoryCounter==0){
               //   printf("directory does not exist but it will be made here \n");
                  mkdir(pointer, S_IRWXU);
                  chdir(pointer);
                  }
                  directoryCounter++;
                  }
            }
            closedir(directory);
          }
          pointer = strtok(NULL, "/");
      }
  }
  if (strcmp(command -> name, "cdh") == 0){   //Here we use the already created directories array by the cd command.
      int pipe1[2]; //my  pipe for  chidl and parent processes.
      if (pipe(pipe1) == -1)
     { // fail case
        fprintf(stderr, "Pipe Failed");
        return 1;
     }
 for(int i=0; i<10; i++) {

         if(i<counter)
     printf("the value of %d     %c  : %s\n",i,97+i, arr[i]);

 }
 printf("Select directory by letter or number \n");
      pid_t pidCdh = fork();

      if(pidCdh==0){
          int numSelected;
  scanf("%d", &numSelected);   //We needed to use child process to use scanf in this program we did this several times.
                
          write(pipe1[1], &numSelected, sizeof(int));
          close(pipe1[1]);
          exit(0);

      }else{

          wait(NULL);
          int numToRead;
          read(pipe1[0], &numToRead, sizeof(int));
          close(pipe1[0]);
          Dir = chdir(arr[numToRead]);

      }
      return SUCCESS;
  }

  if (strcmp(command -> name, "filesearch") == 0) {   //WE used the arg manipulation that is given to us.

    // increase args size by 2
    command -> args = (char ** ) realloc(
      command -> args, sizeof(char * ) * (command -> arg_count += 2));
    // shift everything forward by 1

    for (int i = command -> arg_count - 2; i > 0; --i)

      command -> args[i] = command -> args[i - 1];

    // set args[0] as a copy of name
    command -> args[0] = strdup(command -> name);
    // set args[arg_count-1] (last) to NULL
    command -> args[command -> arg_count - 1] = NULL;

    if (command -> arg_count == 4 || command -> arg_count == 5) {
        if(strcmp(command -> args[1], "-r") == 0){
      listFiles(".", command);
        }
        if(strcmp(command -> args[1], "-o") == 0){
      listFilesOpen(".", command);
      
        }
        if(strcmp(command -> args[1], "-o") == 0 && strcmp(command -> args[2], "-r") == 0){
        
      listFilesOpen(".", command);
      listFiles(".", command);
        }
        if(strcmp(command -> args[1], "-r") == 0 && strcmp(command -> args[2], "-o") == 0){
       
      listFilesOpen(".", command);
      listFiles(".", command);
        }
        
        
        


    }

    if (command -> arg_count == 3) { // 2 parametre girince command arg sayısı=3

      DIR * directory;
      struct dirent * dir;
      directory = opendir(".");

      if (directory) {

        while ((dir = readdir(directory)) != NULL) {

          char * control;



          control = strstr(dir -> d_name, command -> args[1]); // Here arc count =3

          if (control) {
              char* pathAbsolute = realpath(dir -> d_name, NULL);     //We get absolute path with realpath function for filesearch.
           // printf("%s\n", dir -> d_name);
              printf("%s\n", pathAbsolute);

          }
        }

        closedir(directory);

      }

    }
    return (0);
  }

  pid_t pid = fork();

  if (pid == 0) // child

  {

    // increase args size by 2

    command -> args = (char ** ) realloc(

      command -> args, sizeof(char * ) * (command -> arg_count += 2));
    // shift everything forward by 1
    for (int i = command -> arg_count - 2; i > 0; --i)

      command -> args[i] = command -> args[i - 1];
    // set args[0] as a copy of name
    command -> args[0] = strdup(command -> name);
    // set args[arg_count-1] (last) to NULL
    command -> args[command -> arg_count - 1] = NULL;
    /// TODO: do your own exec with path resolving using execv()
    char src[50], path[50];

    strcpy(path, "/bin/");

    strcpy(src, command -> args[0]);

    strcat(path, src);

    execv(path, command -> args);

    exit(0);
  } else

  {

    /// TODO: Wait for child to finish if command is not running in background

    if (!command -> background)

      wait(NULL);

    return SUCCESS;

  }

  printf("-%s: %s: command not found\n", sysname, command -> name);

  return UNKNOWN;

}

