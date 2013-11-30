#include "Posix.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdarg>

static void __attribute__((format(printf, 1, 2))) emit_warning(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

FileDes::~FileDes() {
	if (m_fd != -1) {
		int e;
		do {
			e = 0;
			if (::close(m_fd) == -1) {
				e = errno;
				switch (e) {
					case EBADF: emit_warning("fd %d was invalid on close", m_fd); break;
					case EIO: emit_warning("fd %d caused an I/O error on close", m_fd); break;
				}
			}
		} while (e == EINTR);
	}
}

FileMapping FileMapping::MapWholeFile(const char * const path) {
	const FileDes fd(::open(path, O_RDONLY));
	if (!fd) { throw PosixError(errno); }
	struct stat info;
	if (::fstat(fd, &info) == -1) { throw PosixError(errno); }
	return FileMapping(fd, info.st_size, 0, PROT_READ, MAP_SHARED);
}

FileMapping::FileMapping(const int fd, const size_t len, const off_t offset, const int prot):
	FileMapping(fd, len, offset, prot, MAP_SHARED) {}

FileMapping::FileMapping(const int fd, const size_t len, const off_t offset):
	FileMapping(fd, len, offset, PROT_READ, MAP_SHARED) {}

FileMapping::FileMapping(const int fd, const size_t len, const off_t offset, const int prot, const int flags):
		m_base(nullptr), m_size(0u) {
	void * const p = ::mmap(nullptr, len, prot, flags, fd, offset);
	if (p == MAP_FAILED) { throw PosixError(errno); }
	m_base = p;
	m_size = len;
}

FileMapping::~FileMapping() {
	if (m_base) {
		if (::munmap(m_base, m_size) == -1) {
			int e = errno;
			emit_warning("unmap failure: %s (%zu bytes at %p)", strerror(e), m_size, m_base);
		}
	}
}
