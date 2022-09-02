/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brhajji- <brhajji-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/02 09:49:48 by brhajji-          #+#    #+#             */
/*   Updated: 2022/09/02 13:13:11 by brhajji-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>                                                                                                                                                      
#include <sys/wait.h>

typedef struct s_utils
{
    int     fd[2];
    int     previous_fd;
	int 	nb;
    struct  s_pipex *pipex;
	struct s_utils *next;
} t_utils;

typedef struct s_pipex
{
    char *cmd;
    char **arg;
	int	has_pipe;
    struct s_pipex *next;
} t_pipex;

char **get_arg(int i, char **av, int ac)
{
    char **rendu;
    int x = 0;
    int j = -1;
    while (i + x < ac && av[i + x][0] != '|' && av[i+x][0] != ';')
        x++;
    rendu = malloc(sizeof(char *) * (x + 1));
    while (++j < x)
        rendu[j] = av[i + j];
    rendu[j] = NULL;
    return rendu;
}

void init(int ac, char **av, t_utils *utils)
{
    t_pipex *pipex;
    int i = 0;
	int x = 0;
    pipex = malloc(sizeof(t_pipex));
	utils->fd[0] = -1;
	utils->pipex = pipex;
    while (++i < ac)
    {
        pipex->cmd = av[i];
        pipex->arg = get_arg(i, av, ac);
		while (i < ac && av[i][0] != '|' && av[i][0] != ';')
			i++;
        if (i < ac && av[i][0] == '|')
			pipex->has_pipe = 1;
		else
			pipex->has_pipe = 0;
		if (i == ac - 1)
            pipex->next = NULL;
		else
		{
			pipex->next = malloc(sizeof(t_pipex));
			pipex = pipex->next;
		}
		x++;
    }
	utils->nb = x;
}

void microshell(t_utils *utils, char **envp)
{
	int i = -1;
	pid_t pid;
	int status;

	while (++i < utils->nb)
	{
		if (utils->pipex->has_pipe == 1)
		{
			utils->previous_fd = utils->fd[0];
				if (pipe(utils->fd))
					exit(0);
		}
		pid = fork();
		if (pid < 0)
			exit(1);
		else
		{
			if (utils->previous_fd != -1)
				dup2(utils->previous_fd, 0);
			if (utils->pipex->has_pipe == 1)
				dup2(utils->fd[1], 1);
			if (execve(utils->pipex->cmd, utils->pipex->arg, envp) < 0)
				exit(3);
		}
		close(utils->fd[1]);
	}
	while (--i >= 0)
		wait(&status);	
}

int main (int ac, char **av, char **envp)
{
    t_utils *utils;

	utils = malloc(sizeof(t_utils));
    init(ac, av, utils);
	microshell(utils, envp);
}