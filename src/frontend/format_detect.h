#pragma once
#include "core/file_backend/file.h"
#include "session/session.h"

std::shared_ptr<Session>
TryCreateSession(FB::FilePtr file, const QString &name,
                 std::shared_ptr<Session> parent = nullptr);
