/*
 * cmdshell.c
 *
 *  Created on: 6 april 2020
 *      Author: dell

 SHELL_CMDS_BEGIN
	SHELL_CMD("no0","help0", "error0",0)
	SHELL_CMD("nir","nir - new sim instance, init, reset", "err nir",0)
	SHELL_CMD("new","new - new sim instance", "err new",0)
SHELL_CMDS_END

 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#include "cmdshell.h"

char invitation[]= ">";

/*
 *
 */
uint32_t cmd_quit_cb( int args, char** params )
{
	printf( "\n\nQuit\n" );
	return 0x01;
}
/*
 *
 */
uint32_t cmd_help_cb( int args, char** params )
{
	int idx = 0;

	printf( "\n\nHelp command was entered\n" );
	printf( "----------------------------------------------------------\n" );

	while( shell_commands[ idx ].cmd )
	{
		if( shell_commands[ idx ].help_str != 0x00 )
		{
			printf( "\n%s", shell_commands[ idx ].help_str );
		}
		idx++;
	}
	return 0x00;
}

uint32_t shell_init( void )
{

	return 0x01;
}
/*
 *
 */
char* read_cmdline( void )
{
	int len=0,c;
  char* cmd = malloc(sizeof(char)*MAX_BUF_LEN);

  while( len<( sizeof(char)*( MAX_BUF_LEN-1 )))
  {
  	c = getchar();

  	if(c == '\n')
  	{
  		cmd[len++] = '\0';
      return cmd;
    }
    else
      cmd[len++] = c;
  }
  return NULL;
}

//int parse_cmd_line(char* cmdline, char** cmds)
//{
//        int num_cmds = 0;
//        char* token = strtok(cmdline, ";");
//        while(token!=NULL)
//        {
//                cmds[num_cmds++] = token;
//                token = strtok(NULL, ";");
//        }
//        return num_cmds;
//}

/*
 *
 */
int parse_cmd(char* cmd, char** cmd_tokens)
{
	int tok = 0;
  char* token = strtok(cmd, CMD_DELIMS);

  while(token!=NULL)
  {
  	cmd_tokens[tok++] = token;
    token = strtok(NULL, CMD_DELIMS);
  }
  return tok;
}
/*
 *
 */
uint32_t run_cmd( int tokens, char** cmd_tokens )
{
	char* cmd_tok;
	int idx = 0;

	if( tokens < 1 )
	{
		// nothing was entered
		printf( "\nerr:no cmd typed");

		return 0x00;
	}

	cmd_tok = cmd_tokens[0];

	while( shell_commands[ idx ].cmd )
	{
		if( strcmp( shell_commands[ idx ].cmd, cmd_tok)==0 )
		{
			if( shell_commands[ idx ].cmd_cb )
				return shell_commands[ idx ].cmd_cb( tokens-1, &cmd_tokens[ 1 ]);

			return 0x00;
		}
		idx++;
	}

	// no cmd found
	printf( "\nerr:not found cmd in \"%s\"", cmd_tokens[ 0 ]);

	return 0x00;
}
/*
 *
 */
void shell_run( uint32_t continues_run )
{
	char *rstr;
	char* cmd_tokens[100];

	do
	{
		printf( "\n\n%s", invitation );

		rstr = read_cmdline();

		if( rstr )
		{
			uint32_t tokens =  parse_cmd( rstr, cmd_tokens );

//			printf( "Read string:\n%s\n Number tokens:%d\n ", rstr, tokens );
//			int var;
//			for (var = 0; var < tokens; ++var)
//			{
//				printf("\ntoken[%d]=%s\n", var, cmd_tokens[var]);
//			}

			if( run_cmd( tokens, cmd_tokens ))
				return;

			free( rstr );
		}
	}while( continues_run );
}
