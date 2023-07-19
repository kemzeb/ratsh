/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <vector>

namespace RatShell {

class FileDescriptionCollector {
public:
    FileDescriptionCollector() = default;
    ~FileDescriptionCollector();

    void add(int fd);
    void collect();
    /// NOTE: This should only be used in special situations.
    void clear();

private:
    std::vector<int> m_fds;
};

class SavedFileDescriptions {
public:
    SavedFileDescriptions() = default;
    ~SavedFileDescriptions();

    void add(int fd);
    void restore();

private:
    struct SavedFileDescription {
        int original { -1 };
        int saved { -1 };
    };
    std::vector<SavedFileDescription> m_saves;
    FileDescriptionCollector m_fds;
};

}