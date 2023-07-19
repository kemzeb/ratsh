/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "FileDescription.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

namespace RatShell {

FileDescriptionCollector::~FileDescriptionCollector()
{
    collect();
}

void FileDescriptionCollector::add(int fd)
{
    m_fds.push_back(fd);
}

void FileDescriptionCollector::collect()
{
    for (auto fd : m_fds)
        close(fd);
    m_fds.clear();
}

void FileDescriptionCollector::clear()
{
    m_fds.clear();
}

SavedFileDescriptions::~SavedFileDescriptions()
{
    restore();
}

void SavedFileDescriptions::add(int fd)
{
    auto saved_fd = dup(fd);
    /// FIXME: Should we handle this case?
    if (saved_fd < 0)
        perror("dup");

    // Make sure to close the duplicated fd for the child process before its execution.
    auto flags = fcntl(saved_fd, F_GETFD);
    auto rc = fcntl(saved_fd, F_SETFD, flags | FD_CLOEXEC);
    if (rc < 0)
        perror("fcntl");

    m_saves.push_back({ .original = fd, .saved = saved_fd });
    m_fds.add(saved_fd);
}

void SavedFileDescriptions::restore()
{
    for (auto& save : m_saves) {
        if (dup2(save.saved, save.original) < 0)
            perror("SavedFileDescriptions::restore()");
    }
    m_saves.clear();
    m_fds.collect();
}

}