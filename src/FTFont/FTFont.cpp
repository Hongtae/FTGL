/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include "FTInternals.h"

#include "FTFontImpl.h"

#include "FTBitmapFontImpl.h"
#include "FTExtrudeFontImpl.h"
#include "FTOutlineFontImpl.h"
#include "FTPixmapFontImpl.h"
#include "FTPolygonFontImpl.h"
#include "FTTextureFontImpl.h"

#include "FTGlyphContainer.h"
#include "FTFace.h"


//
//  FTFont
//


FTFont::FTFont(char const *fontFilePath)
{
    impl = new FTFontImpl(this, fontFilePath);
}


FTFont::FTFont(const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
{
    impl = new FTFontImpl(this, pBufferBytes, bufferSizeInBytes);
}


FTFont::FTFont(FTFontImpl *pImpl)
{
    impl = pImpl;
}


FTFont::~FTFont()
{
    delete impl;
}


bool FTFont::Attach(const char* fontFilePath)
{
    return impl->Attach(fontFilePath);
}


bool FTFont::Attach(const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
{
    return impl->Attach(pBufferBytes, bufferSizeInBytes);
}


bool FTFont::FaceSize(const unsigned int size, const unsigned int res)
{
    return impl->FaceSize(size, res);
}


unsigned int FTFont::FaceSize() const
{
    return impl->FaceSize();
}


void FTFont::Depth(float depth)
{
    return impl->Depth(depth);
}


void FTFont::Outset(float outset)
{
    return impl->Outset(outset);
}


void FTFont::Outset(float front, float back)
{
    return impl->Outset(front, back);
}


bool FTFont::CharMap(FT_Encoding encoding)
{
    return impl->CharMap(encoding);
}


unsigned int FTFont::CharMapCount() const
{
    return impl->CharMapCount();
}


FT_Encoding* FTFont::CharMapList()
{
    return impl->CharMapList();
}


void FTFont::UseDisplayList(bool useList)
{
    return impl->UseDisplayList(useList);
}


float FTFont::Ascender() const
{
    return impl->Ascender();
}


float FTFont::Descender() const
{
    return impl->Descender();
}


float FTFont::LineHeight() const
{
    return impl->LineHeight();
}


void FTFont::Render(const char * string, int renderMode)
{
    return impl->Render(string, renderMode);
}


void FTFont::Render(const wchar_t* string, int renderMode)
{
    return impl->Render(string, renderMode);
}


float FTFont::Advance(const wchar_t* string)
{
    return impl->Advance(string);
}


float FTFont::Advance(const char* string)
{
    return impl->Advance(string);
}


FTBBox FTFont::BBox(const char *string)
{
    return impl->BBox(string, 0, -1);
}


FTBBox FTFont::BBox(const wchar_t *string)
{
    return impl->BBox(string, 0, -1);
}


FTBBox FTFont::BBox(const char *string, const int start, const int end)
{
    return impl->BBox(string, start, end);
}


FTBBox FTFont::BBox(const wchar_t *string, const int start, const int end)
{
    return impl->BBox(string, start, end);
}


FT_Error FTFont::Error() const
{
    return impl->err;
}


//
//  FTFontImpl
//


FTFontImpl::FTFontImpl(FTFont *ftFont, char const *fontFilePath) :
    face(fontFilePath),
    useDisplayLists(true),
    intf(ftFont),
    glyphList(0)
{
    err = face.Error();
    if(err == 0)
    {
        glyphList = new FTGlyphContainer(&face);
    }
}


FTFontImpl::FTFontImpl(FTFont *ftFont, const unsigned char *pBufferBytes,
                       size_t bufferSizeInBytes) :
    face(pBufferBytes, bufferSizeInBytes),
    useDisplayLists(true),
    intf(ftFont),
    glyphList(0)
{
    err = face.Error();
    if(err == 0)
    {
        glyphList = new FTGlyphContainer(&face);
    }
}


FTFontImpl::~FTFontImpl()
{
    if(glyphList)
    {
        delete glyphList;
    }
}


/* FIXME: DoRender should disappear, see commit [853]. */
void FTFontImpl::DoRender(const unsigned int chr, const unsigned int nextChr,
                          FTPoint &origin, int renderMode)
{
    if(CheckGlyph(chr))
    {
        FTPoint kernAdvance = glyphList->Render(chr, nextChr, origin, renderMode);
        origin += kernAdvance;
    }
}


template <typename T>
inline void FTFontImpl::RenderI(const T* string, int renderMode)
{
    const T* c = string;
    pen = FTPoint(0., 0.);

    while(*c)
    {
        DoRender(*c, *(c + 1), pen, renderMode);
        ++c;
    }
}


void FTFontImpl::Render(const char * string, int renderMode)
{
    RenderI((const unsigned char *)string, renderMode);
}


void FTFontImpl::Render(const wchar_t* string, int renderMode)
{
    RenderI(string, renderMode);
}


bool FTFontImpl::Attach(const char* fontFilePath)
{
    if(!face.Attach(fontFilePath))
    {
        err = face.Error();
        return false;
    }

    err = 0;
    return true;
}


bool FTFontImpl::Attach(const unsigned char *pBufferBytes,
                        size_t bufferSizeInBytes)
{
    if(!face.Attach(pBufferBytes, bufferSizeInBytes))
    {
        err = face.Error();
        return false;
    }

    err = 0;
    return true;
}


bool FTFontImpl::FaceSize(const unsigned int size, const unsigned int res)
{
    if(glyphList != NULL)
    {
        delete glyphList;
        glyphList = NULL;
    }

    charSize = face.Size(size, res);
    err = face.Error();

    if(err != 0)
    {
        return false;
    }

    glyphList = new FTGlyphContainer(&face);
    return true;
}


unsigned int FTFontImpl::FaceSize() const
{
    return charSize.CharSize();
}


void FTFontImpl::Depth(float depth)
{
    ;
}


void FTFontImpl::Outset(float outset)
{
    ;
}


void FTFontImpl::Outset(float front, float back)
{
    ;
}


bool FTFontImpl::CharMap(FT_Encoding encoding)
{
    bool result = glyphList->CharMap(encoding);
    err = glyphList->Error();
    return result;
}


unsigned int FTFontImpl::CharMapCount() const
{
    return face.CharMapCount();
}


FT_Encoding* FTFontImpl::CharMapList()
{
    return face.CharMapList();
}


void FTFontImpl::UseDisplayList(bool useList)
{
    useDisplayLists = useList;
}


float FTFontImpl::Ascender() const
{
    return charSize.Ascender();
}


float FTFontImpl::Descender() const
{
    return charSize.Descender();
}


float FTFontImpl::LineHeight() const
{
    return charSize.Height();
}


template <typename T>
inline FTBBox FTFontImpl::BBoxI(const T* string, const int start, const int end)
{
    FTBBox totalBBox;

    /* Only compute the bounds if string is non-empty. */
    if(string && ('\0' != string[start]))
    {
        float advance = 0;

        if(CheckGlyph(string[start]))
        {
            totalBBox = glyphList->BBox(string[start]);
            advance = glyphList->Advance(string[start], string[start + 1]);
        }

        /* Expand totalBox by each glyph in String (for idx) */
        for(int i = start + 1; (end < 0 && string[i])
                                 || (end >= 0 && i < end); i++)
        {
            if(CheckGlyph(string[i]))
            {
                FTBBox tempBBox = glyphList->BBox(string[i]);
                tempBBox += FTPoint(advance, 0.0f, 0.0f);

                totalBBox |= tempBBox;
                advance += glyphList->Advance(string[i], string[i + 1]);
            }
        }
    }

    return totalBBox;
}


FTBBox FTFontImpl::BBox(const char *string, const int start, const int end)
{
    /* The chars need to be unsigned because they are cast to int later */
    return BBoxI((const unsigned char *)string, start, end);
}


FTBBox FTFontImpl::BBox(const wchar_t *string, const int start, const int end)
{
    return BBoxI(string, start, end);
}


template <typename T>
inline float FTFontImpl::AdvanceI(const T* string)
{
    const T* c = string;
    float width = 0.0f;

    while(*c)
    {
        if(CheckGlyph(*c))
        {
            width += glyphList->Advance(*c, *(c + 1));
        }
        ++c;
    }

    return width;
}


float FTFontImpl::Advance(const char* string)
{
    /* The chars need to be unsigned because they are cast to int later */
    return AdvanceI((const unsigned char *)string);
}


float FTFontImpl::Advance(const wchar_t* string)
{
    return AdvanceI(string);
}


bool FTFontImpl::CheckGlyph(const unsigned int characterCode)
{
    if(glyphList->Glyph(characterCode))
    {
        return true;
    }

    /*
     * FIXME: load options are not the same for all subclasses:
     *  FTBitmapGlyph: FT_LOAD_DEFAULT
     *  FTExtrudeGlyph: FT_LOAD_NO_HINTING
     *  FTOutlineGlyph: FT_LOAD_NO_HINTING
     *  FTPixmapGlyph: FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP
     *  FTPolygonGlyph: FT_LOAD_NO_HINTING
     *  FTTextureGlyph: FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP
     */
    unsigned int glyphIndex = glyphList->FontIndex(characterCode);
    FT_GlyphSlot ftSlot = face.Glyph(glyphIndex, FT_LOAD_NO_HINTING);
    if(!ftSlot)
    {
        err = face.Error();
        return false;
    }

    FTGlyph* tempGlyph = intf->MakeGlyph(ftSlot);
    if(!tempGlyph)
    {
        if(0 == err)
        {
            err = 0x13;
        }

        return false;
    }

    glyphList->Add(tempGlyph, characterCode);

    return true;
}

