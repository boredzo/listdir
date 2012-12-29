#include <stdbool.h>
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

static int runforpath(const char *path);
static off_t totaldiratpath(const char *path);

int main(int argc, char **argv) {
	int status = EXIT_SUCCESS;
	for (int argi = 1; argi < argc; ++argi) {
		status = runforpath(argv[argi]);
	}
	return status;
}

static int runforpath(const char *path) {
	off_t size = totaldiratpath(path);
	printf("%lld\t%s\n", size, path);
	return size >= 0
		? EXIT_SUCCESS
		: EXIT_FAILURE;
}

static void logpath(unsigned depth, const char *name) {
	for (unsigned d = 0; d < depth; ++d) fputc('\t', stdout);
	printf("%s\n", name);
}
#define logpath(x, y) /**/

static off_t totaldiratpath(const char *path) {
	off_t size = 0;

	DIR *stream = opendir(path);
	if (!stream) {
		perror("Couldn't open directory");
		return -1;
	}

	int olddirfd = open(".", O_RDONLY);
	chdir(path);

	static unsigned depth = 0;
	struct dirent entry;
	struct dirent *result = NULL;
	while ((readdir_r(stream, &entry, &result) == 0) && result) {
		if (strcmp(entry.d_name, ".") == 0) continue;
		if (strcmp(entry.d_name, "..") == 0) continue;
		logpath(depth, entry.d_name);

		switch (entry.d_type) {
			case DT_DIR:
				++depth;
				logpath(depth, "-----");
				size += totaldiratpath(entry.d_name);
				logpath(depth, "-----");
				--depth;
				break;
			default:
				{
					struct stat sb;
					int stat_result = stat(entry.d_name, &sb);
					if (stat_result == 0) {
						size += sb.st_size;
					} else {
						perror("Could not stat");
						char cwd[PATH_MAX];
						fprintf(stderr, "%s/%s\n", getcwd(cwd, PATH_MAX), entry.d_name);
					}
				}
				break;
		}
	}

	int fchdir_result = fchdir(olddirfd);
	if (fchdir_result != 0)
		perror("Could not restore current directory");
	closedir(stream);

	return size;
}
