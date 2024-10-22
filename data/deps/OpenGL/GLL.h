/* data = Auguset 27 2024 07:40 am*/

#ifndef GLL_H
#define GLL_H

#ifdef CLOVER_OPENGL_IMPL
#include "KHR/khrplatform.h"
#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"
#include "glext.h"
#include "wglext.h"

// #############################################################################
//                           OpenGL Function Pointers
// #############################################################################
static PFNGLCREATEPROGRAMPROC glCreateProgram_ptr;
static PFNGLDELETETEXTURESPROC glDeleteTextures_ptr;
static PFNGLGENTEXTURESPROC glGenTextures_ptr;
static PFNGLBINDTEXTUREPROC glBindTexture_ptr;
static PFNGLDRAWBUFFERPROC glDrawBuffer_ptr;
static PFNGLDRAWARRAYSPROC glDrawArrays_ptr;
static PFNGLCREATESHADERPROC glCreateShader_ptr;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ptr;
static PFNGLUNIFORM1FPROC glUniform1f_ptr;
static PFNGLUNIFORM2FVPROC glUniform2fv_ptr;
static PFNGLUNIFORM3FVPROC glUniform3fv_ptr;
static PFNGLUNIFORM1IPROC glUniform1i_ptr;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_ptr;
static PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor_ptr;
static PFNGLACTIVETEXTUREPROC glActiveTexture_ptr;
static PFNGLBUFFERSUBDATAPROC glBufferSubData_ptr;
static PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced_ptr;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer_ptr;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus_ptr;
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers_ptr;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D_ptr;
static PFNGLDRAWBUFFERSPROC glDrawBuffers_ptr;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers_ptr;
static PFNGLBLENDFUNCIPROC glBlendFunci_ptr;
static PFNGLBLENDEQUATIONPROC glBlendEquation_ptr;
static PFNGLCLEARBUFFERFVPROC glClearBufferfv_ptr;
static PFNGLSHADERSOURCEPROC glShaderSource_ptr;
static PFNGLCOMPILESHADERPROC glCompileShader_ptr;
static PFNGLGETSHADERIVPROC glGetShaderiv_ptr;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ptr;
static PFNGLATTACHSHADERPROC glAttachShader_ptr;
static PFNGLLINKPROGRAMPROC glLinkProgram_ptr;
static PFNGLVALIDATEPROGRAMPROC glValidateProgram_ptr;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_ptr;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ptr;
static PFNGLGENBUFFERSPROC glGenBuffers_ptr;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_ptr;
static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation_ptr;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_ptr;
static PFNGLBINDBUFFERPROC glBindBuffer_ptr;
static PFNGLBINDBUFFERBASEPROC glBindBufferBase_ptr;
static PFNGLBUFFERDATAPROC glBufferData_ptr;
static PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv_ptr;
static PFNGLUSEPROGRAMPROC glUseProgram_ptr;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays_ptr;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers_ptr;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_ptr;
static PFNGLDETACHSHADERPROC glDetachShader_ptr;
static PFNGLDELETESHADERPROC glDeleteShader_ptr;
static PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced_ptr;
static PFNGLGENERATEMIPMAPPROC glGenerateMipmap_ptr;
static PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_ptr;
static PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeperate_ptr;


inline void 
CloverLoadGL()
{
    // Load OpenGL Functions from the Operating System / Graphics Card
    glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glDeleteTextures_ptr = (PFNGLDELETETEXTURESPROC)wglGetProcAddress("glDeleteTextures");
    glGenTextures_ptr = (PFNGLGENTEXTURESPROC)wglGetProcAddress("glGenTextures");
    glBindTexture_ptr = (PFNGLBINDTEXTUREPROC)wglGetProcAddress("glBindTexture");
    glDrawArrays_ptr = (PFNGLDRAWARRAYSPROC)wglGetProcAddress("glDrawArrays");
    glCreateShader_ptr = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
    glGetUniformLocation_ptr = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
    glUniform1f_ptr = (PFNGLUNIFORM1FPROC) wglGetProcAddress("glUniform1f");
    glUniform2fv_ptr = (PFNGLUNIFORM2FVPROC) wglGetProcAddress("glUniform2fv");
    glUniform3fv_ptr = (PFNGLUNIFORM3FVPROC) wglGetProcAddress("glUniform3fv");
    glUniform1i_ptr = (PFNGLUNIFORM1IPROC) wglGetProcAddress("glUniform1i");
    glUniformMatrix4fv_ptr = (PFNGLUNIFORMMATRIX4FVPROC) wglGetProcAddress("glUniformMatrix4fv");
    glVertexAttribDivisor_ptr = (PFNGLVERTEXATTRIBDIVISORPROC) wglGetProcAddress("glVertexAttribDivisor");
    glActiveTexture_ptr = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress("glActiveTexture");
    glBufferSubData_ptr = (PFNGLBUFFERSUBDATAPROC) wglGetProcAddress("glBufferSubData");
    glDrawArraysInstanced_ptr = (PFNGLDRAWARRAYSINSTANCEDPROC) wglGetProcAddress("glDrawArraysInstanced");
    glBindFramebuffer_ptr = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
    glCheckFramebufferStatus_ptr = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
    glGenFramebuffers_ptr = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
    glFramebufferTexture2D_ptr = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");
    glDrawBuffers_ptr = (PFNGLDRAWBUFFERSPROC) wglGetProcAddress("glDrawBuffers");
    glDeleteFramebuffers_ptr = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffers");
    glBlendFunci_ptr = (PFNGLBLENDFUNCIPROC) wglGetProcAddress("glBlendFunci");
    glBlendEquation_ptr = (PFNGLBLENDEQUATIONPROC) wglGetProcAddress("glBlendEquation");
    glClearBufferfv_ptr = (PFNGLCLEARBUFFERFVPROC) wglGetProcAddress("glClearBufferfv");
    glShaderSource_ptr = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
    glCompileShader_ptr = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
    glGetShaderiv_ptr = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog_ptr = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
    glAttachShader_ptr = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
    glLinkProgram_ptr = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
    glValidateProgram_ptr = (PFNGLVALIDATEPROGRAMPROC) wglGetProcAddress("glValidateProgram");
    glGetProgramiv_ptr = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog_ptr = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
    glGenBuffers_ptr = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
    glGenVertexArrays_ptr = (PFNGLGENVERTEXARRAYSPROC) wglGetProcAddress("glGenVertexArrays");
    glGetAttribLocation_ptr = (PFNGLGETATTRIBLOCATIONPROC) wglGetProcAddress("glGetAttribLocation");
    glBindVertexArray_ptr = (PFNGLBINDVERTEXARRAYPROC) wglGetProcAddress("glBindVertexArray");
    glEnableVertexAttribArray_ptr = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer_ptr = (PFNGLVERTEXATTRIBPOINTERPROC) wglGetProcAddress("glVertexAttribPointer");
    glBindBuffer_ptr = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
    glBindBufferBase_ptr = (PFNGLBINDBUFFERBASEPROC) wglGetProcAddress("glBindBufferBase");
    glBufferData_ptr = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
    glGetVertexAttribPointerv_ptr = (PFNGLGETVERTEXATTRIBPOINTERVPROC) wglGetProcAddress("glGetVertexAttribPointerv");
    glUseProgram_ptr = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
    glDeleteVertexArrays_ptr = (PFNGLDELETEVERTEXARRAYSPROC) wglGetProcAddress("glDeleteVertexArrays");
    glDeleteBuffers_ptr = (PFNGLDELETEBUFFERSPROC) wglGetProcAddress("glDeleteBuffers");
    glDeleteProgram_ptr = (PFNGLDELETEPROGRAMPROC) wglGetProcAddress("glDeleteProgram");
    glDetachShader_ptr = (PFNGLDETACHSHADERPROC) wglGetProcAddress("glDetachShader");
    glDeleteShader_ptr = (PFNGLDELETESHADERPROC) wglGetProcAddress("glDeleteShader");
    glDrawElementsInstanced_ptr = (PFNGLDRAWELEMENTSINSTANCEDPROC) wglGetProcAddress("glDrawElementsInstanced");
    glGenerateMipmap_ptr = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
    glDebugMessageCallback_ptr = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");
    glBlendFuncSeperate_ptr = (PFNGLBLENDFUNCSEPARATEPROC)wglGetProcAddress("glBlendFuncSeparate");
}

// #############################################################################
//                           OpenGL Function Wrappers
// #############################################################################
inline GLAPI GLuint APIENTRY glCreateProgram (void)
{
    return glCreateProgram_ptr();
}

inline GLAPI void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures)
{
    glDeleteTextures_ptr(n, textures);
}

inline GLAPI void APIENTRY glGenTextures (GLsizei n, GLuint *textures)
{
    glGenTextures_ptr(n, textures);
}

inline GLAPI void APIENTRY glBindTexture (GLenum target, GLuint texture)
{
    glBindTexture_ptr(target, texture);
}

inline void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    glDrawArrays_ptr(mode, first, count);
}

inline GLuint glCreateShader(GLenum shaderType)
{
    return glCreateShader_ptr(shaderType);
}

inline GLint glGetUniformLocation(GLuint program, const GLchar* name)
{
    return glGetUniformLocation_ptr(program, name);
}

inline void glUniform1f(GLint location, GLfloat v0)
{
    glUniform1f_ptr(location, v0);
}

inline void glUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
    glUniform2fv_ptr(location, count, value);
}

inline void glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
    glUniform3fv_ptr(location, count, value);
}

inline void glUniform1i(GLint location, GLint v0)
{
    glUniform1i_ptr(location, v0);
}

inline void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    glUniformMatrix4fv_ptr(location, count, transpose, value);
}

inline void glVertexAttribDivisor(GLuint index, GLuint divisor)
{
    glVertexAttribDivisor_ptr(index, divisor);
}

inline void glActiveTexture(GLenum texture)
{
    glActiveTexture_ptr(texture);
}

inline void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    glBufferSubData_ptr(target, offset, size, data);
}

inline void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    glDrawArraysInstanced_ptr(mode, first, count, instanceCount);
}

inline void glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    glBindFramebuffer_ptr(target, framebuffer);
}

inline GLenum glCheckFramebufferStatus(GLenum target)
{
    return glCheckFramebufferStatus_ptr(target);
}

inline void glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    glGenFramebuffers_ptr(n, framebuffers);
}

inline void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    glFramebufferTexture2D_ptr(target, attachment, textarget, texture, level);
}

inline void glDrawBuffers(GLsizei n, const GLenum* bufs)
{
    glDrawBuffers_ptr(n, bufs);
}

inline void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    glDeleteFramebuffers_ptr(n, framebuffers);
}

inline void glBlendFunci(GLuint buf, GLenum src, GLenum dst)
{
    glBlendFunci_ptr(buf, src, dst);
}

inline void glBlendEquation(GLenum mode)
{
    glBlendEquation_ptr(mode);
}

inline void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    glClearBufferfv_ptr(buffer, drawbuffer, value);
}

inline void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* strings, const GLint* lengths)
{
    glShaderSource_ptr(shader, count, strings, lengths);
}

inline void glCompileShader(GLuint shader)
{
    glCompileShader_ptr(shader);
}

inline void glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    glGetShaderiv_ptr(shader, pname, params);
}

inline void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    glGetShaderInfoLog_ptr(shader, bufSize, length, infoLog);
}

inline void glAttachShader(GLuint program, GLuint shader)
{
    glAttachShader_ptr(program, shader);
}

inline void glLinkProgram(GLuint program)
{
    glLinkProgram_ptr(program);
}

inline void glValidateProgram(GLuint program)
{
    glValidateProgram_ptr(program);
}

inline void glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    glGetProgramiv_ptr(program, pname, params);
}

inline void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    glGetProgramInfoLog_ptr(program, bufSize, length, infoLog);
}

inline void glGenBuffers(GLsizei n, GLuint* buffers)
{
    glGenBuffers_ptr(n, buffers);
}

inline void glGenVertexArrays(GLsizei n, GLuint* arrays)
{
    glGenVertexArrays_ptr(n, arrays);
}

inline GLint glGetAttribLocation(GLuint program, const GLchar* name)
{
    return glGetAttribLocation_ptr(program, name);
}

inline void glBindVertexArray(GLuint array)
{
    glBindVertexArray_ptr(array);
}

inline void glEnableVertexAttribArray(GLuint index)
{
    glEnableVertexAttribArray_ptr(index);
}

inline void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    glVertexAttribPointer_ptr(index, size, type, normalized, stride, pointer);
}

inline void glBindBuffer(GLenum target, GLuint buffer)
{
    glBindBuffer_ptr(target, buffer);
}

inline void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    glBindBufferBase_ptr(target, index, buffer);
}

inline void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    glBufferData_ptr(target, size, data, usage);
}

inline void glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
    glGetVertexAttribPointerv_ptr(index, pname, pointer);
}

inline void glUseProgram(GLuint program)
{
    glUseProgram_ptr(program);
}

inline void glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
    glDeleteVertexArrays_ptr(n, arrays);
}

inline void glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    glDeleteBuffers_ptr(n, buffers);
}

inline void glDeleteProgram(GLuint program)
{
    glDeleteProgram_ptr(program);
}

inline void glDetachShader (GLuint program, GLuint shader)
{
    glDetachShader_ptr(program, shader);
}

inline void glDeleteShader(GLuint shader)
{
    glDeleteShader_ptr(shader);
}

inline void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
    glDrawElementsInstanced_ptr(mode, count, type, indices, instancecount);
}

inline void glGenerateMipmap(GLenum target)
{
    glGenerateMipmap_ptr(target);
}

inline void glDebugMessageCallback (GLDEBUGPROC callback, const void *userParam)
{
    glDebugMessageCallback_ptr(callback, userParam);
}
inline void glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    glBlendFuncSeperate_ptr(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}
#endif
#endif
