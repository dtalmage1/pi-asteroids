#pragma once
#include <gmock/gmock.h>
#include "game/IInputSource.hpp"

namespace ast {

class MockInputSource : public IInputSource {
public:
    MOCK_METHOD(InputState, query, (), (const, override));
};

} // namespace ast
