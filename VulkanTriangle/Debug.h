#pragma once

class Debug
{
public:
#ifdef _DEBUG
    static constexpr bool is_debug_enabled = true;
#else
    static constexpr bool is_debug_enabled = false;
#endif
};
