# blAST project
#
# Copyright (c) 2026 András Bodor <bodand@proton.me>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# - Neither the name of the copyright holder nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Originally created: 2026-08-30.
#
# src/libprcmgr.c4/CMakeLists.txt --
#

add_c4_library(TARGET prcmgr
               C_LIBS skarnet array
               C4_SOURCES
                  src4/prcmgr.c4
               FLAGS
                  "-I${CMAKE_CURRENT_BINARY_DIR}/../libstd4.c4"
                  "-I${CMAKE_CURRENT_BINARY_DIR}/../libarray.c4"
               C_SOURCES
                  src/prcmgr/raw_environ.posix.c
                  src/prcmgr/ska_spawn.c
                  src/prcmgr/wait_pid.c
               )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/prcmgr.c4a"
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS prcmgr
        EXPORT c4std)
