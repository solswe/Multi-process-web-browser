#include "wrapper.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <signal.h>

/* === PROVIDED CODE === */
// Function Definitions
void new_tab_created_cb(GtkButton *button, gpointer data);
int run_control();
int on_blacklist(char *uri);
int bad_format (char *uri);
void uri_entered_cb(GtkWidget* entry, gpointer data);
void init_blacklist (char *fname);

/* === PROVIDED CODE === */
// Global Definitions
#define MAX_TAB 100                 // Maximum number of tabs allowed
#define MAX_BAD 1000                // Maximum number of URL's in blacklist allowed
#define MAX_URL 100                 // Maximum char length of a url allowed
#define TAB_INDEX 0                 // Tab index starts from zero
#define NUM_BLACKLIST 0

/* === STUDENTS IMPLEMENT=== */
// HINT: What globals might you want to declare?
char blacklist_arr[MAX_BAD][MAX_URL];   // Array to track blacklisted-urls
int tab_index = TAB_INDEX;    // Count index of tab
int pid_array[MAX_TAB];   // Array to save the pid
int num_blacklist = NUM_BLACKLIST;     // Count the number of blacklist urls

/* === PROVIDED CODE === */
/*
 * Name:		          new_tab_created_cb
 *
 * Input arguments:	
 *      'button'      - whose click generated this callback
 *			'data'        - auxillary data passed along for handling
 *			                this event.
 *
 * Output arguments:   void
 * 
 * Description:        This is the callback function for the 'create_new_tab'
 *			               event which is generated when the user clicks the '+'
 *			               button in the controller-tab. The controller-tab
 *			               redirects the request to the parent (/router) process
 *			               which then creates a new child process for creating
 *			               and managing this new tab.
 */
// NO-OP for now
void new_tab_created_cb(GtkButton *button, gpointer data)
{}
 
/* === PROVIDED CODE === */
/*
 * Name:                run_control
 * Output arguments:    void
 * Function:            This function will make a CONTROLLER window and be blocked until the program terminates.
 */
int run_control()
{
  // (a) Init a browser_window object
	browser_window * b_window = NULL;

	// (b) Create controller window with callback function
	create_browser(CONTROLLER_TAB,
                0,
                G_CALLBACK(new_tab_created_cb),
		            G_CALLBACK(uri_entered_cb),
                &b_window);

	// (c) enter the GTK infinite loop
	show_browser();
	return 0;
}

/* === STUDENTS IMPLEMENT=== */
/* 
    Function: on_blacklist  --> "Check if the provided URI is in th blacklist"
    Input:    char* uri     --> "URI to check against the blacklist"
    Returns:  True  (1) if uri in blacklist
              False (0) if uri not in blacklist
    Hints:
            (a) File I/O
            (b) Handle case with and without "www." (see writeup for details)
            (c) How should you handle "http://" and "https://" ??
*/ 
int on_blacklist (char *uri) {    
  // loop through the blacklist array
  for(int i = 0; i < num_blacklist; i++){
    // check if the current blacklist url has www.
    char *token = strtok(blacklist_arr[i], ".");
    if (strncmp("www", token, 3) == 0) {
      // remove www. from blacklist url
      char* pass_www = blacklist_arr[i] + 4;
      // remove new line character from the blacklist url
      char* noSpace = strtok(pass_www, "\n");
      // if the uri is in the blacklist
      if(strstr(uri, noSpace)!=NULL){
        return 1;
      }
    }
    // case without www.
    else{      
      char* noSpace = strtok(blacklist_arr[i], "\n");
      // if the uri is in the blacklist
      if(strstr(uri, noSpace)!=NULL){
        return 1;
      }
    }          
  } 
  
  return 0;
}

/* === STUDENTS IMPLEMENT=== */
/* 
    Function: bad_format    --> "Check for a badly formatted url string"
    Input:    char* uri     --> "URI to check if it is bad"
    Returns:  True  (1) if uri is badly formatted 
              False (0) if uri is well formatted
    Hints:
              (a) String checking for http:// or https://
*/
int bad_format (char *uri) {
  // Check if the input uri start with https:// and there is character(s) after that
  if (strncmp("https://", uri, 8) == 0) {
    if (strlen(uri) == 8) {
      return 1;
    }
    return 0;
  } 
  // Check if the input uri start with http:// and there is character(s) after that
  else if (strncmp("http://", uri, 7) == 0) {
    if (strlen(uri) == 7) {
      return 1;
    }
    return 0;
  }
  else {
    return 1;
  }
}

/* === STUDENTS IMPLEMENT=== */
/*
 * Name:                uri_entered_cb
 *
 * Input arguments:     
 *                      'entry'-address bar where the url was entered
 *			   int pid_array[MAX_TAB];
 * Function:             When the user hits the enter after entering the url
 *			                 in the address bar, 'activate' event is generated
 *			                 for the Widget Entry, for which 'uri_entered_cb'
 *			                 callback is called. Controller-tab captures this event
 *			                 and sends the browsing request to the router(/parent)
 *			                 process.
 * Hints:
 *                       (a) What happens if data is empty? No Url passed in? Handle that
 *                       (b) Get the URL from the GtkWidget (hint: look at wrapper.h)
 *                       (c) Print the URL you got, this is the intermediate submission
 *                       (d) Check for a bad url format THEN check if it is in the blacklist
 *                       (e) Check for number of tabs! Look at constraints section in lab
 *                       (f) Open the URL, this will need some 'forking' some 'execing' etc. 
 */
void uri_entered_cb(GtkWidget* entry, gpointer data) {
  if(data == NULL){     // Check if the data is empty
    return;
  }
  
  gchar *input_url;
  input_url = get_entered_uri(entry);       // Get the URL from GtkWidget

  printf("URL entered is %s\n", input_url);     // Print the URL on terminal
  
  if (bad_format(input_url) == 1) {     // Check if the url has a bad format
    alert("Bad format!");
  } else if (on_blacklist(input_url) == 1) {    // Check if the url is on the blacklist
    alert("Blacklist!");
  } else if (tab_index == MAX_TAB) {   // Check if number of tabs reached maximum
    alert("Max tab!");
  } else {        // If the url has a good format, open it
    pid_t childpid;
    childpid = fork();    // Create a fork
    
    if (childpid == -1){    // Fork failed and no child created
      perror("fork() failed");
      exit(1);
    } else if (childpid == 0) {    // Fork success and execute child
      int index_length = snprintf(NULL, 0, "%d", tab_index);
      char *index_string = malloc(index_length + 1);
      snprintf(index_string, index_length + 1, "%d", tab_index);    // Change int tab_index to string

      pid_array[tab_index] = getpid();    // Store pid number into the pid_array
      
      // Executing input url page, if it's failed print an error message
      if (execl("./render", "render", index_string, input_url, NULL) == -1){
        printf("execl() failed");
        exit(1);
      }

      free(index_string);

    } 
    else {      // Fork success and execute parent
      tab_index++;    // Keep track the number of tabs
    }
  }
  return;
}

/* === STUDENTS IMPLEMENT=== */
/* 
    Function: init_blacklist --> "Open a passed in filepath to a blacklist of url's, read and parse into an array"
    Input:    char* fname    --> "file path to the blacklist file"
    Returns:  void
    Hints:
            (a) File I/O (fopen, fgets, etc.)
            (b) If we want this list of url's to be accessible elsewhere, where do we put the array?
*/
void init_blacklist (char *fname) {
  FILE *blacklist;
  blacklist = fopen(fname, "r");    // Open and read from the file 
   
  while(fgets(blacklist_arr[num_blacklist], MAX_URL, blacklist)) {    // Put blacklisted-urls into blacklist_arr
    if (!EOF) {
      blacklist_arr[num_blacklist][strlen(blacklist_arr[num_blacklist]) - 1] = '\0';
    }
    num_blacklist++;
  }
  fclose(blacklist);      // Close the file
  return;
}

/* === STUDENTS IMPLEMENT=== */
/* 
    Function: main 
    Hints:
            (a) Check for a blacklist file as argument, fail if not there [PROVIDED]
            (b) Initialize the blacklist of url's
            (c) Create a controller process then run the controller
                (i)  What should controller do if it is exited? Look at writeup (KILL! WAIT!)
            (d) Parent should not exit until the controller process is done 
*/
int main(int argc, char **argv){
  // [PROVIDED CODE] Check arguments for blacklist, error and warn if no blacklist
  if (argc != 2) {
    fprintf (stderr, "browser <blacklist_file>\n");
    exit (0);
  } else {
    init_blacklist(argv[1]);    // Initialize the blacklist from input
  }

  pid_t childpid;
  childpid = fork();    // Create a fork

	if (childpid == -1){    // Fork failed and no child created
		perror("fork() failed");
		return 1;
	} else if (childpid == 0){    // Fork success and execute child
		run_control();      // Execute controller
    for (int i = 0; i < tab_index; i++) {    
        kill(pid_array[i], SIGKILL);    // Kill owned child forks
        wait(NULL);     
    }
	} else {    // Fork success and execute parent
    wait(NULL);     // Wait for the controller
		printf("Killed\n");
	}
  
  return 0;
}
