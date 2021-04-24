#pragma once

#define ThrowIfFailed(RESULT) if(FAILED((RESULT))) { throw std::exception(); }

#define ReturnFalseIfFailed(RESULT) if(FAILED((RESULT))) { return false; }

