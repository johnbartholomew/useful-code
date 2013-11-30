#ifndef POSIX_HPP
#define POSIX_HPP

#include <sys/types.h>
#include <stdexcept>
#include <cstring>

class PosixError : public std::runtime_error {
	public:
		explicit PosixError(int e): std::runtime_error(std::strerror(e)), m_errno(e) {}
		PosixError(int e, const std::string &message): std::runtime_error(message), m_errno(e) {}
		PosixError(int e, const char *message): std::runtime_error(message), m_errno(e) {}

		int error_code() const { return m_errno; }
	private:
		int m_errno;
};

class FileDes {
	public:
		FileDes(): m_fd(-1) {}
		explicit FileDes(int fd): m_fd(fd) {}
		~FileDes();

		FileDes(FileDes&& from): m_fd(from.m_fd) { from.m_fd = -1; }
		FileDes& operator=(FileDes&& from) {
			FileDes tmp(std::move(from));
			using std::swap;
			swap(this->m_fd, tmp.m_fd);
			return *this;
		}

		FileDes(const FileDes&) = delete;
		FileDes& operator=(const FileDes&) = delete;

		int fd() const { return m_fd; }
		operator int() const { return m_fd; }

		explicit operator bool() const { return (m_fd != -1); }

	private:
		int m_fd;
};

class FileMapping {
	public:
		static FileMapping MapWholeFile(const char * const path);

		FileMapping(): m_base(nullptr), m_size(0u) {}
		~FileMapping();

		explicit FileMapping(const int fd, const size_t len, const off_t offset, const int prot, const int flags);
		explicit FileMapping(const int fd, const size_t len, const off_t offset, const int prot);
		explicit FileMapping(const int fd, const size_t len, const off_t offset = 0);

		FileMapping(FileMapping&& other): m_base(other.m_base), m_size(other.m_size) {
			other.m_base = nullptr;
			other.m_size = 0u;
		}

		FileMapping& operator=(FileMapping&& other) {
			if (this != &other) {
				this->m_base = other.m_base;
				this->m_size = other.m_size;
				other.m_base = nullptr;
				other.m_size = 0u;
			}
			return *this;
		}

		FileMapping(const FileMapping&) = delete;
		FileMapping& operator=(const FileMapping&) = delete;

		void *get() const { return m_base; }
		size_t size() const { return m_size; }

		explicit operator bool() const { return m_base; }

	private:
		void *m_base;
		size_t m_size;
};

#endif
