#include "preprocessor.hpp"

#include <vector>

C::Preprocessor::TokenBufferPool::Buffer C::Preprocessor::TokenBufferPool::Acquire()
{
    if (available.empty())
        return Buffer(*this, {});

    std::vector<C::PPTokens_Old::Token> buf = std::move(available.back());
    available.pop_back();

    buf.clear();
    return Buffer(*this, std::move(buf));
}

void C::Preprocessor::TokenBufferPool::Release(std::vector<PPTokens_Old::Token>&& buf)
{
    available.push_back(std::move(buf));
}
