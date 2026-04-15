#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include<unistd.h>
#include <limits.h>
#include <signal.h>
#include <malloc.h>
#include<string.h>

#define handle_error(msg) do {perror(msg); exit(EXIT_FAILURE);} while(0)

static char *buffer;

/* 
Note: calling printf() from a signal handler is not safe
(and should not be done in production programs), since
printf() is not async-signal-safe; see signal-safety(7).
Nevertheless, we use printf() here as a simple way of
showing that the handler was called. 
*/
static void handler(int sig, siginfo_t *si, void *unused){
  printf("Got SIGSEGV at address: 0x%lx\n", (long) si->si_addr);
  exit(EXIT_FAILURE);
}
  
int main(int argc, char *argv[]){
  char *p, *buffer; // * means 'a pointer to'
	char c; //c is a char
  int pagesize;
	int i = 0, size;
  
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = handler;
  
  if(sigaction(SIGSEGV, &sa, NULL) == -1)
    handle_error("sigaction");

  // Initializing Pagesize, most systems its 4096 bytes
  pagesize = sysconf(_SC_PAGE_SIZE);
  
  if(pagesize == -1)
    handle_error("sysconf");
 
  //Allocate a buffer; it will have the default protection of PROT_READ|PROT_WRITE. 
  size = pagesize * 10;
  p = memalign(pagesize, size); //Allocating buffer of size of ten pages
  
  if (p == NULL)
    handle_error("memalign");

  memset(p, 0x00, size); // Zeros
  memset(p, 0x41, size); // 'A'
  
  for(i = 0; i < 10; i++){
    /*
    Printing all pages first bytes from first page. 
    The usage of %d format specifier causes compilation warnings.
    */	
	  printf("Address of %d Page: %lx\n", i + 1 , p + (i * pagesize));	
  }

  // TODO: Start writing code here and can define variables for functions above
  //#7
  // The first name stored as a string in the 9th and 10th page of the buffer.
  char firstname[] = "marissa";
  int firstname_len = strlen(firstname); // Calculate the length of the first name string

  char *page9 = p + (8 * pagesize); // Address of 9th page
  char *page10 = p + (9 * pagesize); // Address of 10th page

  memcpy(page9, firstname, firstname_len); // Copying the first name to the 9th page
  memcpy(page10, firstname, firstname_len); // Copying the first name to the 10th page

  printf("Content of 9th page: %.*s\n", firstname_len, page9); // Printing the content of the 9th page, using %.*s to specify the length of the string
  printf("Content of 10th page: %.*s\n", firstname_len, page10); // Printing the content of the 10th page, using %.*s to specify the length of the string

  printf("\n");

  //#8
  char lastname[] = "rupert";
  int lastname_len = strlen(lastname); // Calculate the length of the last name string

  char *page7 = p + (6 * pagesize); // Address of 7th page
  char *page8 = p + (7 * pagesize); // Address of 8th page

  // Change protection of pages 7 and 8 to READ and WRITE
  if (mprotect(page7, pagesize * 2, PROT_READ | PROT_WRITE) == -1)
    handle_error("mprotect");

    memcpy(page7, lastname, lastname_len); // Copying the last name to the 7th page
    memcpy(page8, lastname, lastname_len); // Copying the last name to the 8th page

  printf("Content of 7th page: %.*s\n", lastname_len, page7); // Printing the content of the 7th page, using %.*s to specify the length of the string
  printf("Content of 8th page: %.*s\n", lastname_len, page8); // Printing the content of the 8th page, using %.*s to specify the length of the string

  printf("\n");

  //#9
  char username[] = "mrr24";
  int username_len = strlen(username); // Calculate the length of the username string

  char *page5 = p + (4 * pagesize); // Address of 5th page
  char *page6 = p + (5 * pagesize); // Address of 6th page

  // Set pages 5 and 6 to WRITE ONLY
  if (mprotect(page5, pagesize * 2, PROT_WRITE) == -1)
    handle_error("mprotect");

  memcpy(page5, username, username_len); // Copying the username to the 5th page
  memcpy(page6, username, username_len); // Copying the username to the 6

  printf("Content of 5th page: %.*s\n", username_len, page5); // Printing the content of the 5th page, using %.*s to specify the length of the string
  printf("Content of 6th page: %.*s\n", username_len, page6); // Printing the content of the 6th page, using %.*s to specify the length of the string 

  //#10
  char *copy_buffer = memalign(pagesize, pagesize * 2); // Allocate a new buffer of twice the size as the original buffer
  if (copy_buffer == NULL)
    handle_error("memalign");

  memcpy(copy_buffer, p + (6 * pagesize), pagesize); // Copy the content of the original buffer to the new buffer

  printf("Content of the new buffer in page 7: %.*s\n", pagesize, copy_buffer); // Printing the content of the new buffer, using %.*s to specify the length of the string

  memcpy(copy_buffer, p + (7 * pagesize), pagesize); // Copy the content of the original buffer to the new buffer

  printf("Content of the new buffer in page 8: %.*s\n", pagesize, copy_buffer); // Printing the content of the new buffer, using %.*s to specify the length of the string

  free(copy_buffer); // Free the allocated memory for the copy buffer

  //#11
  // Overwrite buffer with page 6
  memcpy(copy_buffer, p + (5 * pagesize), pagesize);

  printf("Content of the new buffer in page 6: %.*s\n", pagesize, copy_buffer); // Printing the content of the new buffer, using %.*s to specify the length of the string

  // Overwrite second half with page 9
  memcpy(copy_buffer + pagesize, p + (8 * pagesize), pagesize);
  
  printf("Content of the new buffer in page 9: %.*s\n", pagesize, copy_buffer + pagesize); // Printing the content of the new buffer, using %.*s to specify the length of the string


  exit(EXIT_SUCCESS);
}