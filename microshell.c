#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define STDIN__ 0
#define STDOUT__ 1
#define STDERR__ 2
#define errput(S) write(STDERR__, (S), strlen__(S))
#define ERR_CD_ARG "error: cd: bad arguments"
#define ERR_CD_FAIL "error: cd: cannot change directory to "
#define ERR_FATAL "error: fatal"
#define ERR_EXE_FAIL "error: cannot execute "
#define ENDL "\n"

int strlen__(const char *s);
void error__(const char *msg, const char *targ);

int exec(int ac, char **av, char **env, int flag);
int cd(int ac, char **av);
int run(int ac, char **av, char **env, int flag);
int parent(pid_t pid, int flag, int *fd);
void child(int ac, char **av, char **env, int flag, int *fd);

int main(int ac, char **av, char **env) {
	int l = 0, r = -1, flag = 0, rtn = 0;
	--ac, ++av;
	if (ac) {
		while (++r <= ac) {
			if ((flag = (r == ac) * 1)
			|| (flag = !strcmp(av[r], "|") * 2)
			|| (flag = !strcmp(av[r], ";") * 3)) {
				if (l < r)
					rtn = exec(r - l, av + l, env, flag);
				l = r + 1;
			}
		}
	}
	exit(rtn);
}

int strlen__(const char *s) {
	int i = -1;
	while (s[++i]);
	return i;
}
void error__(const char *msg, const char *targ) {
	unsigned long p1 = (unsigned long)msg;
	unsigned long p2 = (unsigned long)ERR_FATAL;
	errput(msg);
	if (targ)
		errput(targ);
	errput(ENDL);
	if (p1 == p2)
		exit(-1);
}

int exec(int ac, char **av, char **env, int flag) {
	if (!strcmp(*av, "cd"))
		return cd(ac, av);
	return run(ac, av, env, flag);
}
int cd(int ac, char **av) {
	if (ac != 2)
		return error__(ERR_CD_ARG, NULL), -1;
	if (chdir(av[1]))
		return error__(ERR_CD_FAIL, av[1]), -1;
	return 0;
}
int run(int ac, char **av, char **env, int flag) {
	int fd[2];
	if (flag == 2 && pipe(fd))
		error__(ERR_FATAL, NULL);
	pid_t pid = fork();
	if (!pid)
		child(ac, av, env, flag, fd);
	return parent(pid, flag, fd);
}
int parent(pid_t pid, int flag, int *fd) {
	int rtn;
	waitpid(pid, &rtn, 0);
	if (flag == 2 && (dup2(fd[0], STDIN__) || close(fd[0]) || close(fd[1])))
		error__(ERR_FATAL, NULL);
	return WIFEXITED(rtn) && WEXITSTATUS(rtn);
}
void child(int ac, char **av, char **env, int flag, int *fd) {
	av[ac] = NULL;
	if (flag == 2 && (dup2(fd[1], STDOUT__) || close(fd[0]) || close(fd[1])))
		error__(ERR_FATAL, NULL);
	execve(*av, av, env);
	error__(ERR_EXE_FAIL, *av), exit(-1);
}

