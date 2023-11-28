#include <stdio.h>
#include <string.h>
#include <fuse.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/xattr.h>

static char global_base_path[1024] = {0};

char* gen_abs_path(const char* path) {
	int size = 10240;
	char *pabs = malloc(size);

	memset(pabs, 0, size);
	snprintf(pabs, size, "%s%s", global_base_path, path);

	return pabs;
}

static int cfs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi)
{
	DIR *dir;
	struct dirent *entry;
	struct stat info;
	char *localpath = gen_abs_path(path);

	dir = opendir(localpath);
	if (!dir) {
		printf("cfs_readdir opendir error. path: %s\n", localpath);
		free(localpath);
		return -1;
	}
	while(NULL != (entry = readdir(dir))) {
		char fullpath[1024] = {0};
		snprintf(fullpath, sizeof(fullpath), "%s/%s", localpath, entry->d_name);

		if(stat(fullpath, &info) != -1) {
			filler(buf, entry->d_name, &info, 0);
		} else {
			printf("cfs_readdir stat error. fullpath: %s\n", fullpath);
		}
	}
	closedir(dir);
	free(localpath);
	return 0;
}

static int cfs_releasedir(const char* path, struct fuse_file_info* fi)
{
	printf("cfs_releasedir path:%s\n", path);
	return 0;
}

static int cfs_getattr(const char* path, struct stat *stbuf)
{
	int ret;
	struct stat fd_stat;
	char *abs_path = gen_abs_path(path);
	
	printf("cfs_getattr. path: %s\n", abs_path);
   	memset(stbuf, 0, sizeof(struct stat));

	ret = stat(abs_path, &fd_stat);

	if(ret == 0) {
		*stbuf = fd_stat;
	} else {
		printf("cfs_getaddr error, ret:%d\n", ret);
		ret = -ENOENT;
	}
	/*
	if(strcmp(path, "/") == 0)
		stbuf->st_mode = 0755 | S_IFDIR;
	else
		stbuf->st_mode = 0644 | S_IFREG;
	return 0;
	*/
	free(abs_path);
	return ret;
}

static void* cfs_init(void)
{
    printf("cfs_init.\n");
    return NULL;
}

static void cfs_destroy(void *input)
{
    printf("cfs_destroy.\n");
}

static int cfs_create(const char *path, mode_t m, struct fuse_file_info *fi)
{
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_create. path: %s, mode: %d\n", path, m);
	if((ret = creat(abs_path, m)) == -1) {
		ret = errno;
		printf("cfs_create. creat error. abspath: %s, ret: %d\n", abs_path, ret);
	} else {
		ret = 0;
	}
	free(abs_path);
	return ret;
}

static int cfs_mknod(const char *path, mode_t m, dev_t dev)
{
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_mknod. path: %s, mode: %d\n", path, m);
	if(-1 == (ret = mknod(abs_path, m, dev))) {
		ret = errno;
		printf("cfs_mknod error. abspath: %s, errno: %d\n", abs_path, ret);
	}
	free(abs_path);
	return ret;
}

static int cfs_removexattr(const char *path, const char *name)
{
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_removexattr. path: %s, name: %s\n", path, name);
	if((ret = removexattr(abs_path, name)) != 0) {
		ret = errno;
		printf("cfs_removexattr. error abspath: %s, errno: %d\n", abs_path, ret);
	}
	free(abs_path);
	return ret;
}

static int cfs_getxattr(const char *path, const char *name, char *value, size_t s)
{
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_getxattr. path: %s, name: %s, value: %s, size: %ld\n", path, name, value, s);
	if((ret = getxattr(abs_path, name, value, s)) == -1) {
		ret = errno;
		printf("cfs_getxattr. error abspath: %s, errno: %d\n", abs_path, ret);
		ret = 0;
	}
	free(abs_path);
	return ret;
}

static int cfs_setxattr(const char *path, const char *name, const char *value, size_t s, int flag)
{
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_setxattr. path: %s, name: %s, value: %s, size: %ld, flag: %d\n", path, name, value, s, flag);
	if((ret = setxattr(abs_path, name, value, s, flag)) != 0) {
		printf("cfs_setxattr. error abspath: %s, ret: %d\n", abs_path, ret);
		ret = errno;
	}
	free(abs_path);
	return ret;
}

static int cfs_open(const char* path, struct fuse_file_info *fi)
{
	printf("cfs_open. path: %s\n", path);
	return 0;
}

static int cfs_access(const char *path, int mode)
{
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_access. path: %s, mode: %d\n", path, mode);
	if(-1 == (ret = access(abs_path, mode))){
		ret = errno;
		printf("cfs_access error, path: %s, errno: %d\n", abs_path, ret);
	}
	free(abs_path);
	return ret;
}

static int cfs_unlink(const char *path)
{
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_unlink. path: %s\n", path);
	if(-1 == (ret = unlink(abs_path))) {
		ret = errno;
		printf("cfs_unlink error, path: %s, errno:%d\n", abs_path, ret);
	}
	free(abs_path);
	return ret;
}

static int cfs_read(const char* path, char* buf, size_t s, off_t o, struct fuse_file_info *fi)
{
	int fd;
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_read. path: %s, offset: %ld, size: %ld\n", path, o, s);
	fd = open(abs_path, O_RDONLY);
	if(fd == 0 || (ret = pread(fd, buf, s, o)) == -1 ) {
		ret = errno;
		printf("cfs_read error, path: %s, errno : %d\n", abs_path, ret);
	}
	close(fd);
	free(abs_path);
	return ret;
}

static int cfs_write(const char* path, const char* buf, size_t s, off_t o, struct fuse_file_info *fi)
{
	int fd;
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_write. path: %s, offset: %ld, size: %ld\n", path, o, s);
	fd = open(abs_path, O_SYNC|O_RDWR);
	if(fd == 0 || (ret = pwrite(fd, buf, s, o)) == -1) {
		ret = errno;
		printf("cfs_write error , path: %s, errno: %d\n", abs_path, ret);
	}
	close(fd);
	free(abs_path);
	return ret;
}

static int cfs_flush(const char* path, struct fuse_file_info *fi)
{
	printf("cfs_flush. path: %s\n", path);
	return 0;
}

static int cfs_mkdir(const char *path, mode_t m)
{
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_mkdir. path: %s, mode: %d\n", path, m);
	if((ret = mkdir(abs_path, m)) != 0) {
		ret = errno;
		printf("cfs_mkdir mkdir error , path: %s, errno: %d\n", abs_path, ret);
	}
	free(abs_path);
	return ret;
}

static int cfs_rmdir(const char *path)
{
	int ret;
	char *abs_path = gen_abs_path(path);

	printf("cfs_rmdir. path: %s\n", path);
	if(0 != (ret = rmdir(abs_path))){
		ret = errno;
		printf("cfs_rmdir rm error, path: %s, errno: %d\n", abs_path, ret);
	}
	free(abs_path);
	return ret;
}

static struct fuse_operations cryptfs_ops = {
    .init = cfs_init,
    .destroy = cfs_destroy,
    .open = cfs_open,
    .mkdir = cfs_mkdir,
    .rmdir = cfs_rmdir,
    .readdir = cfs_readdir,
    .releasedir = cfs_releasedir,
    .getattr = cfs_getattr,
    .create = cfs_create,
    .mknod = cfs_mknod,
    .open = cfs_open,
    .read = cfs_read,
    .write = cfs_write,
    .flush = cfs_flush,
    .getxattr = cfs_getxattr,
    .setxattr = cfs_setxattr,
    .removexattr = cfs_removexattr,
    .access = cfs_access,
    .unlink = cfs_unlink,
};

int main(int argc, char *argv[])
{
    printf("argc: %d, lastone: %s\n", argc, argv[argc-1]);
    int ret = 0;
    strcpy(global_base_path, "/home/.mirror");
    printf("global_base_path: %s\n", global_base_path);
    ret = fuse_main(argc, argv, &cryptfs_ops);
    return ret;
}
