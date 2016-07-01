//SYSTEMS:

I/O-
    SDL2(https://www.libsdl.org/) is used for getting graphics/sounds/input contexts.
    OpenGL(https://www.opengl.org) is used for 3D rendering.

Scripting-
    Embedded stateless Ruby interpreter.

Textures-
    Stored as 32-bit PNG files with an alpha channel.
    Loaded with the LodePNG(http://lodev.org/lodepng/) library.

Strings-
    Stored on disk as UTF-8 text files.
    Interpreted through Ruby.
    Passed to ICU(http://icu-project.org) and converted to UTF-16 internally.
    Dev environment locale is UTF-8 so string literals also go through ICU.

Fonts-
    Stored on disk as AngelCode BMFont(http://www.angelcode.com/products/bmfont).
    Interpreted through Ruby.
    Rendered as signed distance fields using Hiero(https://github.com/libgdx/libgdx/wiki/Hiero).
    Glyph information stored internally as map between ICU UChar32 and a glyph struct.

Models-
    Stored on disk as quad-based Wavefront OBJ files exported from Blender.
    Loaded and parsed with ASSIMP(http://www.assimp.org/).
    Originals stored as Blender files in seperate folder.

//TODO:
add real lighting
    status: read all of Carmack's .plan files
come up with a level format
    status: began implementing octree-based geometry indexing, slow going
decide on a proper string format
    status: added ICU to the project, but haven't really moved over to it yet
generalize key input for later rebinding
    status: not started
alt-tab support
    status: investigated and found nothing, ALT-TAB keystrokes are being
            eaten while I have mouse focus
collision
    status: rudimentary and bad collision is in, but it's buggy and doesn't use
            octrees
    gravity: ray cast down to see if the player is supported, move the player
             to that level of support



//LIBRARY AND LICENSING INFORMATION:

SDL2.0 used under the zlib license.
    https://www.libsdl.org/license.php

GLEW used under: http://glew.sourceforge.net/glew.txt
    The OpenGL Extension Wrangler Library
    Copyright (C) 2008-2016, Nigel Stewart <nigels[]users sourceforge net>
    Copyright (C) 2002-2008, Milan Ikits <milan ikits[]ieee org>
    Copyright (C) 2002-2008, Marcelo E. Magallon <mmagallo[]debian org>
    Copyright (C) 2002, Lev Povalahev
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * The name of the author may be used to endorse or promote products
      derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
    THE POSSIBILITY OF SUCH DAMAGE.

GLM used under: http://glm.g-truc.net/copying.txt
    ==========================================================================
    OpenGL Mathematics (GLM)
    --------------------------------------------------------------------------
    GLM is licensed under The Happy Bunny License and MIT License

    ==========================================================================
    The Happy Bunny License (Modified MIT License)
    --------------------------------------------------------------------------
    Copyright (c) 2005 - 2014 G-Truc Creation

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    Restrictions:
     By making use of the Software for military purposes, you choose to make a
     Bunny unhappy.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

ASSIMP used under: http://www.assimp.org/main_license.html
    Copyright (c) 2006-2015 assimp team
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    Neither the name of the assimp team nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

Ruby Interpreter used under: https://www.ruby-lang.org/en/about/license.txt
