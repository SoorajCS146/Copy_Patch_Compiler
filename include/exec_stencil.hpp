#ifndef EXEC_STENCIL_HPP
#define EXEC_STENCIL_HPP

#include <vector>
#include <cstdint>
#include "ir.hpp"
#include "stencil.hpp"
#include "codebuffer.hpp"

// Runs IR program using stencils and returns exit code.
int generate_and_run_with_stencils(const std::vector<IRInstr>& ir, StencilLibrary& lib);

#endif
