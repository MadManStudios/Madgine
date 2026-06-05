#pragma once

template <typename T>
void logContainer(const T &c)
{
    Engine::Platform::Log::LogDummy ss { Engine ::Platform::Log::MessageType::INFO_TYPE };
    Engine::StringUtil::StreamJoiner join { ss, ", " };
    for (const auto &i : c) {
        join.next() << i;
    }
}