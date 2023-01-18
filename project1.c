#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static int search_command(char command[],int word_count,int space_indexes[],char command_history[],int total_commands);
static int strsplit(char* string,int space_indexes[]);
static int search_history(char* command_history,int total_commands);
static int clean_last_char(char* string);
static int store_command(char* command,int total_commands,char*  command_history);
static char* trim_quations(char* string);
static int printfile(char* file_name);




int main(){
  //Collect username with popen function and store it in username.
  char username[256];
  FILE* pipe = popen("whoami","r"); //Result of whoami command is written to pipe file
  fgets(username,256,pipe); //username is readed from the file
  clean_last_char(username);; //username contains a newline char at end so last char is deleted
  strcat(username," >>> "); //username will be printed with the following additions
  pclose(pipe);
  
  
  char command[256];//The command string that will be collected from user
  char command_history[4096];//history array that will store last 15 commands and current command
  int total_commands = 0;//total commands that are executed up until now

  while(1){
    printf("%s",username);//Start with printing username
    fgets(command, 256, stdin);//Collect command from user
    clean_last_char(command);//command contains a newline char at end so last char is deleted
    store_command(command,total_commands,command_history);//command is pushed to command history
    total_commands++;
    if(strcmp(command,"exit") == 0){break;}//if command is exit,exit program
    int space_indexes[256];//A list that contains space character indexes. However it points to the next index of the space.
    int word_count = strsplit(command,space_indexes);//Count of words.Since there isn't check of space placement,this also equals to space char count
 search_command(command,word_count,space_indexes,command_history,total_commands);
  }//this part takes the appropriate action for commands
return 0;
}
//this function takes the appropriate action for commands
int search_command(char command[],int word_count,int space_indexes[],char command_history[],int total_commands){
  pid_t child_pid;
  child_pid = fork();//to exec given commands,child process is created here
  if(child_pid != 0){
    //This is parent process
    wait();//wait is called because parent should not return to printing before child is done printing
    return child_pid;
  }
  else{
    //listdir command is handled with executing ls
    if(strcmp(command,"listdir") == 0 && word_count == 1){
      execlp("ls","ls",(char*)NULL);
      fprintf(stderr,"an error occured");
      abort();
    }
    //mycomputername command is handled with executing hostname
    else if(strcmp(command,"mycomputername") == 0 && word_count == 1){
      execlp("hostname","hostname",(char*)NULL);
      fprintf(stderr,"an error occured");
      abort();
    }
    //whatsmyip command is handled with executing hostname -i
    else if(strcmp(command,"whatsmyip") == 0 && word_count == 1){
      execlp("/bin/hostname","hostname","-i",(char*)NULL);
      fprintf(stderr,"an error occured");
      abort();
    }
    //printfile (f1)  command is handled with executing cat (f1)
    else if(strcmp(command,"printfile") == 0 && word_count == 2){
      char* first_arg = &(command[space_indexes[1]]);
      printfile(first_arg);
      abort();
      /*
      execlp("/bin/cat","cat",first_arg,(char*)NULL);
      fprintf(stderr,"an error occured");
      abort();
      */
    }
    //printfile (f1) > (f2) command is handled with executing cp (f1) (f2)
    else if(strcmp(command,"printfile") == 0 && word_count == 4){
      char* first_arg = &(command[space_indexes[1]]);
      char* third_arg = &(command[space_indexes[3]]);
      execlp("/bin/cp","cp",first_arg,third_arg,(char*)NULL);
      fprintf(stderr,"an error occured");
      abort();
    }
    //For dididothat command, search history is called. This function returns 1 if found else returns 0
    else if(strcmp(command,"dididothat") == 0){
      int found = search_history(command_history,total_commands);
      if(found){
        printf("Yes\n");
      }
      else{
        printf("No\n");
      }
      abort();
    }
    //hellotext command is handled with executing gedit
    else if(strcmp(command,"hellotext") == 0 && word_count == 1){
      execlp("/bin/gedit","gedit",(char*)NULL);
      fprintf(stderr,"an error occured");
      abort();
    }
    else{
      abort();
    }
  }
  return 0;
}

/*Given command is pushed to command history. command history can hold 16 commands. 
One of them is the current command and the others are past commands. 
If past command count passes 15, commands are written over the oldest command.*/
int store_command(char* command,int total_commands,char* command_history){
  int stack_pointer = total_commands % 16;//this calculation returns the oldest command position
  for(int i=0;i<256;i++){
    command_history[256*stack_pointer + i]=command[i];
  }
  return 0;
}

//this function just changes space chars to null chars and word starting positions
int strsplit(char* string,int space_indexes[]){
  int word_count = 0;
  //the first word starts 0th index
  space_indexes[0] = 0;
  word_count++;
  
  for(int i=0;string[i] != '\0';i++){
    if(string[i] == ' '){
      string[i]='\0';
      space_indexes[word_count] = i+1;
      word_count++;
    }
  }
  return word_count;
}
//This functions looks for a exact match of the given command in the command history
int search_history(char* command_history,int total_commands){
  int stack_pointer = (total_commands-1) % 16;//position of current command
  //retrieve current command from command history
  char* current_command = &(command_history[stack_pointer*256]);
  
  //retrieve the part after "dididothat ___"
  char* search_input_nontrimmed;
  search_input_nontrimmed = &(current_command[11]);
  
  printf("NonTrimmed Current : %s\n",search_input_nontrimmed);
  
  //Remove quations 
  char* search_input;
  search_input = trim_quations(search_input_nontrimmed);
  
  printf("Current : %s\n",search_input);
    
  //Compare current command with past commands that have smaller index
  for(int i=0;i<stack_pointer;i++){
    char* past_command = &(command_history[i*256]);
    
    printf("Past : %s\n",past_command);
    
    if(strcmp(past_command,search_input)== 0){
      return 1;
    }
  }
  //Compare current command with past commands that have bigger index
  //This is done seperately because the history may not be filled for the bigger indexes if total commands does not exceed 15
  if(total_commands >= 16){
    for(int i=stack_pointer+1;i<16;i++){
      char* past_command = &(command_history[i*256]);
      if(strcmp(past_command,search_input)== 0){
        return 1;
      }
    }
  }
  return 0;
}

//Removes newline char at the end if it exists
int clean_last_char(char* string){
  int length = strlen(string);
  if (length == 0){
    return 0;
  }
  if(string[length-1] == 10){
    string[length-1]= '\0';
  }
  return 0;
}

//Removes quation chars at the beginnig and end if they exist
char* trim_quations(char* string){
  char* result;
  if(string[0] = '\"'){
    result= &(string[1]);
  }
  int length = strlen(result);
  if(result[length-1] = '\"'){
    result[length-1] = '\0';
  }
  return result;
}
//Opens and prints the given file. Stops at the newline characters and waits for a newline input.
int printfile(char* file_name){
  FILE* pipe = fopen(file_name,"r");
  char c;
  while(!feof(pipe)){
    c = fgetc(pipe);
    if(c != '\n'){
      printf("%c",c);
    }
    else{
      while(1){
        if(getchar() == '\n'){
          break;
        }
      }
    }
  }
}


    
	 	
