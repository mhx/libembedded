/*
 * Copyright (c) Marcus Holland-Moritz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#define LIBEMB_STRINGIFY(x) LIBEMB_STRINGIFY_IMPL(x)
#define LIBEMB_STRINGIFY_IMPL(x) #x

#define LIBEMB_CONCAT_IMPL(x, y) x##y
#define LIBEMB_CONCAT(x, y) LIBEMB_CONCAT_IMPL(x, y)

#ifdef __COUNTER__
#define LIBEMB_UNIQUE_NAME(prefix) LIBEMB_CONCAT(prefix, __COUNTER__)
#else
#define LIBEMB_UNIQUE_NAME(prefix) LIBEMB_CONCAT(prefix, __LINE__)
#endif
