#pragma once
#include "core/file.h"
#include "frontend/session.h"

std::shared_ptr<Session>
TryCreateSession(FB::FilePtr file, const QString &name,
                 std::shared_ptr<Session> parent = nullptr);
