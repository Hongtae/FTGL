/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 *               2008 Éric Beets <ericbeets@free.fr>
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
 *
 * Alternatively, you can redistribute and/or modify this software under
 * the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License,
 * or (at your option) any later version.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
 */

#include "config.h"

#include "FTPolyGlyph.h"
#include "FTVectoriser.h"


FTPolyGlyph::FTPolyGlyph(FT_GlyphSlot glyph, float outset, bool useDisplayList)
:   FTGlyph( glyph),
    glList(0)
{
    if( ft_glyph_format_outline != glyph->format)
    {
        err = 0x14; // Invalid_Outline
        return;
    }

    FTVectoriser vectoriser(glyph, outset * 64.0f);

    if(( vectoriser.ContourCount() < 1) || ( vectoriser.PointCount() < 3))
    {
        return;
    }
    
    unsigned int horizontalTextureScale = glyph->face->size->metrics.x_ppem * 64;
    unsigned int verticalTextureScale = glyph->face->size->metrics.y_ppem * 64;        
        
    vectoriser.MakeMesh(1.0, 1);
    
    if( useDisplayList)
    {
        glList = glGenLists( 1);
        glNewList( glList, GL_COMPILE);
    }

    const FTMesh* mesh = vectoriser.GetMesh();
    for( unsigned int index = 0; index < mesh->TesselationCount(); ++index)
    {
        const FTTesselation* subMesh = mesh->Tesselation( index);
        unsigned int polygonType = subMesh->PolygonType();

        glBegin( polygonType);
            for( unsigned int pointIndex = 0; pointIndex < subMesh->PointCount(); ++pointIndex)
            {
                FTPoint point = subMesh->Point(pointIndex);
                
                glTexCoord2f( point.X() / horizontalTextureScale,
                              point.Y() / verticalTextureScale);
                
                glVertex3f( point.X() / 64.0f,
                            point.Y() / 64.0f,
                            0.0f);
            }
        glEnd();
    }

    if(useDisplayList)
    {
        glEndList();
    }
}


FTPolyGlyph::~FTPolyGlyph()
{
    glDeleteLists( glList, 1);
}


const FTPoint& FTPolyGlyph::Render(const FTPoint& pen)
{
    if(glList)
    {
        glTranslatef(pen.X(), pen.Y(), 0.0f);
        glCallList(glList);
        glTranslatef(-pen.X(), -pen.Y(), 0.0f);
    }
    
    return advance;
}

