/* $Header: https://svn.ita.chalmers.se/repos/security/edu/course/computer_security/trunk/lab/login_linux/login_linux.c 585 2013-01-19 10:31:04Z pk@CHALMERS.SE $ */

/* gcc -std=gnu99 -Wall -g -o mylogin login_linux.c -lcrypt */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <crypt.h>
/* Uncomment next line in step 2 */
#include "pwent.h"

#define TRUE 1
#define FALSE 0
#define LENGTH 16
#define MAX_AGE 10
#define MAX_ATTEMPTS 3
#define SHELL "sh"



void setup_sighandler() {

	/* add signalhandling routines here */
	/* see 'man 2 signal' */
	//signal(SIGINT, SIG_IGN);  /* ctrl+c */
	signal(SIGTSTP, SIG_IGN);  /* ctrl+z */
	signal(SIGQUIT, SIG_IGN);  /* ctrl+\ */
}

void start_shell(int uid) {
	if (setuid(uid) == -1) {
		printf("Failed to set the UID.\n");
		return;
	}
	char *args[] = {SHELL, NULL};
	if (execvp(SHELL, args) == -1) {
		printf("Failed to start shell.\n");
	}
}

int main(int argc, char *argv[]) {

	// struct passwd *passwddata; /* this has to be redefined in step 2 */
	/* see pwent.h */

	mypwent* ent;

	char important1[LENGTH] = "**IMPORTANT 1**";

	char user[LENGTH];

	char important2[LENGTH] = "**IMPORTANT 2**";

	//char   *c_pass; //you might want to use this variable later...
	char prompt[] = "password: ";
	char *user_pass;

	setup_sighandler();

	while (TRUE) {
		/* check what important variable contains - do not remove, part of buffer overflow test */
		printf("Value of variable 'important1' before input of login name: %s\n",
				important1);
		printf("Value of variable 'important2' before input of login name: %s\n",
				important2);

		printf("login: ");
		fflush(NULL); /* Flush all  output buffers */
		__fpurge(stdin); /* Purge any data in stdin buffer */

		// We use fgets due to it being buffer overflow safe if you limit the length.
		if (fgets(user, LENGTH, stdin) == NULL) /* gets() is vulnerable to buffer */
			exit(0); /*  overflow attacks.  */
		
		for(int ptr = 0; ptr < LENGTH; ptr++){  // replace newline with string terminator for fgets
			if (user[ptr] == '\n')
				user[ptr] = '\0';
		}

		/* check to see if important variable is intact after input of login name - do not remove */
		printf("Value of variable 'important 1' after input of login name: %*.*s\n",
				LENGTH - 1, LENGTH - 1, important1);
		printf("Value of variable 'important 2' after input of login name: %*.*s\n",
		 		LENGTH - 1, LENGTH - 1, important2);

		user_pass = getpass(prompt);
		ent = mygetpwnam(user);
		// passwddata = getpwnam(user);

		if (ent == NULL) {
			printf("No user with that name \n");
			continue;
		}
			/* You have to encrypt user_pass for this to work */
			/* Don't forget to include the salt */

		if (ent->pwfailed >= MAX_ATTEMPTS) {
			printf("Too many failed login attempts\n");
			// Resetting counter here for debugging purposes
			ent->pwfailed = 0;  // basic implementation, would have to be more sophistiacted in real life scenarios
			mysetpwent(user, ent);
			break;
		}

		// Currently we're using "pass" as the password
		// salt is XZ
		if (!strcmp(crypt(user_pass, ent->passwd_salt), ent->passwd)) {
			ent->pwfailed = 0;
			printf("You're in !\n");
			int attempts = ++ent->pwage;
			if (attempts>MAX_AGE)
				printf("You need to renew your password >:( \n");

			mysetpwent(user, ent);
			start_shell(ent->uid);
			continue;

			/*  check UID, see setuid(2) */
			/*  start a shell, use execve(2) */

		}
		ent->pwfailed++;
		mysetpwent(user, ent);
		printf("Login Incorrect \n");
	}
	return 0;
}
