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

#include "FTVectoriser.h"
#include "FTGL.h"

#ifndef CALLBACK
#define CALLBACK
#endif

#ifdef __APPLE_CC__    
    typedef GLvoid (*GLUTesselatorFunction)(...);
#elif defined( __mips ) || defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __OpenBSD__ ) || defined( __sun ) || defined (__CYGWIN__)
    typedef GLvoid (*GLUTesselatorFunction)();
#elif defined ( WIN32)
    typedef GLvoid (CALLBACK *GLUTesselatorFunction)( );
#else
    #error "Error - need to define type GLUTesselatorFunction for this platform/compiler"
#endif


void CALLBACK ftglError( GLenum errCode, FTMesh* mesh)
{
    mesh->Error( errCode);
}


void CALLBACK ftglVertex( void* data, FTMesh* mesh)
{
    FTGL_DOUBLE* vertex = static_cast<FTGL_DOUBLE*>(data);
    mesh->AddPoint( vertex[0], vertex[1], vertex[2]);
}


void CALLBACK ftglCombine( FTGL_DOUBLE coords[3], void* vertex_data[4], GLfloat weight[4], void** outData, FTMesh* mesh)
{
    const FTGL_DOUBLE* vertex = static_cast<const FTGL_DOUBLE*>(coords);
    *outData = const_cast<FTGL_DOUBLE*>(mesh->Combine( vertex[0], vertex[1], vertex[2]));
}
        

void CALLBACK ftglBegin( GLenum type, FTMesh* mesh)
{
    mesh->Begin( type);
}


void CALLBACK ftglEnd( FTMesh* mesh)
{
    mesh->End();
}


FTMesh::FTMesh()
:	currentTesselation(0),
    err(0)
{
    tesselationList.reserve( 16);
}


FTMesh::~FTMesh()
{
    for( size_t t = 0; t < tesselationList.size(); ++t)
    {
        delete tesselationList[t];
    }
    
    tesselationList.clear();
}


void FTMesh::AddPoint( const FTGL_DOUBLE x, const FTGL_DOUBLE y, const FTGL_DOUBLE z)
{
    currentTesselation->AddPoint( x, y, z);
}


const FTGL_DOUBLE* FTMesh::Combine( const FTGL_DOUBLE x, const FTGL_DOUBLE y, const FTGL_DOUBLE z)
{
    tempPointList.push_back( FTPoint( x, y,z));
    return static_cast<const FTGL_DOUBLE*>(tempPointList.back());
}


void FTMesh::Begin( GLenum meshType)
{
    currentTesselation = new FTTesselation( meshType);
}


void FTMesh::End()
{
    tesselationList.push_back( currentTesselation);
}


const FTTesselation* const FTMesh::Tesselation( unsigned int index) const
{
    return ( index < tesselationList.size()) ? tesselationList[index] : NULL;
}


FTVectoriser::FTVectoriser(const FT_GlyphSlot glyph, float frontOutset, float backOutset)
:   contourList(0),
    mesh(0),
    ftContourCount(0),
    contourFlag(0)
{
    if( glyph)
    {
        outline = glyph->outline;
        
        ftContourCount = outline.n_contours;
        contourList = 0;
        contourFlag = outline.flags;
        
        ProcessContours(frontOutset, backOutset);
    }
}


FTVectoriser::~FTVectoriser()
{
    for( size_t c = 0; c < ContourCount(); ++c)
    {
        delete contourList[c];
    }

    delete [] contourList;
    delete mesh;
}


void FTVectoriser::ProcessContours(float frontOutset, float backOutset)
{
    short contourLength = 0;
    short startIndex = 0;
    short endIndex = 0;
    
    contourList = new FTContour*[ftContourCount];
    
    for( short contourIndex = 0; contourIndex < ftContourCount; ++contourIndex)
    {
        FT_Vector* pointList = &outline.points[startIndex];
        char* tagList = &outline.tags[startIndex];
        
        endIndex = outline.contours[contourIndex];
        contourLength =  ( endIndex - startIndex) + 1;

        FTContour* contour = new FTContour(pointList, tagList, contourLength, frontOutset, backOutset);
        
        contourList[contourIndex] = contour;
        
        startIndex = endIndex + 1;
    }
}


size_t FTVectoriser::PointCount()
{
    size_t s = 0;
    for( size_t c = 0; c < ContourCount(); ++c)
    {
        s += contourList[c]->PointCount();
    }
    
    return s;
}


const FTContour* const FTVectoriser::Contour( unsigned int index) const
{
    return ( index < ContourCount()) ? contourList[index] : NULL;
}


void FTVectoriser::MakeMesh(FTGL_DOUBLE zNormal, int outsetContour)
{
    if( mesh)
    {
        delete mesh;
    }
        
    mesh = new FTMesh;
    
    GLUtesselator* tobj = gluNewTess();

    gluTessCallback( tobj, GLU_TESS_BEGIN_DATA,     (GLUTesselatorFunction)ftglBegin);
    gluTessCallback( tobj, GLU_TESS_VERTEX_DATA,    (GLUTesselatorFunction)ftglVertex);
    gluTessCallback( tobj, GLU_TESS_COMBINE_DATA,   (GLUTesselatorFunction)ftglCombine);
    gluTessCallback( tobj, GLU_TESS_END_DATA,       (GLUTesselatorFunction)ftglEnd);
    gluTessCallback( tobj, GLU_TESS_ERROR_DATA,     (GLUTesselatorFunction)ftglError);
    
    if( contourFlag & ft_outline_even_odd_fill) // ft_outline_reverse_fill
    {
        gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
    }
    else
    {
        gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
    }
    
    
    gluTessProperty( tobj, GLU_TESS_TOLERANCE, 0);
    gluTessNormal( tobj, 0.0f, 0.0f, zNormal);
    gluTessBeginPolygon( tobj, mesh);

        for( size_t c = 0; c < ContourCount(); ++c)
        {
            const FTContour* contour = contourList[c];

            gluTessBeginContour( tobj);
                for( size_t p = 0; p < contour->PointCount(); ++p)
                {
                    const FTGL_DOUBLE* d;
                    switch(outsetContour)
                    {
                        case 0: d = contour->Point(p); break;
                        case 1 : d = contour->FrontPoint(p); break;
                        case 2 : d = contour->BackPoint(p);  break;
                    }
                    gluTessVertex( tobj, (GLdouble*)d, (GLdouble*)d);
                }

            gluTessEndContour( tobj);
        }
    gluTessEndPolygon( tobj);

    gluDeleteTess( tobj);
}

