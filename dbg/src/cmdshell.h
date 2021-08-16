#ifndef CMDSHELL_H_
#define CMDSHELL_H_

/*! \file      cmdshell.h
    \version   0.0
    \date      6 апр. 2020 г. 21:36:34
    \brief     
    \details   
    \bug
    \copyright 
    \author

Example
		SHELL_CMDS_BEGIN
			SHELL_CMD("no0","help0", "error0",0)
			SHELL_CMD("nir","nir - new sim instance, init, reset", "err nir",0)
			SHELL_CMD("new","new - new sim instance", "err new",0)
		SHELL_CMDS_END
*/



/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */

#define MAX_BUF_LEN 1024
#define CMD_DELIMS 	" \t\n"

/*
	Use these colors to print colored text on the console
 */
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define SHELL_CMDS_BEGIN cmd_shell_cell_t shell_commands[] = \
		{

#define SHELL_CMDS_END 	{ \
					"q", \
					"q - Quit", \
					"error1", \
					cmd_quit_cb \
				}, \
				{ \
					"h", \
					"h - Printing help for commands list", \
					"errors", \
					cmd_help_cb \
				}, \
				{ \
					0x00, \
					0x00, \
					0x00, \
					0x00 \
				}, \
		};

#define SHELL_CMD(cmd_str,cmd_help,cmd_err,cmd_fn_cb) { \
					cmd_str, \
					cmd_help, \
					cmd_err, \
					cmd_fn_cb \
				},


/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
// call back on cmd was entered
typedef uint32_t (*cb_cmd_shell_t)( int args,  char** params );

typedef struct cmd_shell_cell_t
{
	char* cmd;
	char* help_str;
	char* error_str;
	cb_cmd_shell_t cmd_cb;
} cmd_shell_cell_t;
/* ------------------------------------------------------------------------------------------------
 *                                    Functions declarations
 * ------------------------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C"
{
#endif

extern uint32_t shell_init( void );
extern void shell_run( uint32_t continues_run );

extern uint32_t cmd_quit_cb( int args, char** params );
extern uint32_t cmd_help_cb( int args,  char** params );


extern cmd_shell_cell_t shell_commands[];

#ifdef __cplusplus
}
#endif


#endif /* CMDSHELL_H_ */
