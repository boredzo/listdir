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
static size_t listdiratpath(const char *path);

int main(int argc, char **argv) {
	int status = EXIT_SUCCESS;
	for (int argi = 1; argi < argc; ++argi) {
		status = runforpath(argv[argi]);
	}
	return status;
}

static int runforpath(const char *path) {
	size_t count = listdiratpath(path);
	printf("%zu\t%s\n", count, path);
	return EXIT_SUCCESS;
}

static void printpath(unsigned depth, const char *name) {
	for (unsigned d = 0; d < depth; ++d) fputc('\t', stdout);
	printf("%s\n", name);
}

static size_t listdiratpath(const char *path) {
	size_t count = 0;

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
		printpath(depth, entry.d_name);

		if (entry.d_type == DT_DIR) {
			++depth;
			count += listdiratpath(entry.d_name);
			--depth;
		}
	}

	int fchdir_result = fchdir(olddirfd);
	if (fchdir_result != 0)
		perror("Could not restore current directory");
	closedir(stream);

	return count;
}
