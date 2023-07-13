/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "FileDescription.h"
#include <cstdio>
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

SavedFileDescriptions::~SavedFileDescriptions()
{
    restore();
}

void SavedFileDescriptions::add(SavedFileDescriptions::SavedFileDescription description)
{
    m_saves.push_back(description);
    m_fds.add(description.saved);
}

void SavedFileDescriptions::restore()
{
    for (auto& save : m_saves) {
        if (dup2(save.saved, save.original) < 0)
            perror("internal: SavedFileDescriptions failed to restore");
    }
    m_saves.clear();
}

}