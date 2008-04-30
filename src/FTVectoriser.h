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

#ifndef     __FTVectoriser__
#define     __FTVectoriser__

#include "FTGL/ftgl.h"

#include "FTContour.h"
#include "FTList.h"
#include "FTVector.h"


#ifndef CALLBACK
#define CALLBACK
#endif


/**
 * FTTesselation captures points that are output by OpenGL's gluTesselator.
 */
class FTGL_EXPORT FTTesselation
{
    public:
        /**
         * Default constructor
         */
        FTTesselation(GLenum m)
        : meshType(m)
        {
            pointList.reserve(128);
        }

        /**
         *  Destructor
         */
        ~FTTesselation()
        {
            pointList.clear();
        }

        /**
         * Add a point to the mesh.
         */
        void AddPoint(const FTGL_DOUBLE x, const FTGL_DOUBLE y,
                      const FTGL_DOUBLE z)
        {
            pointList.push_back(FTPoint(x, y, z));
        }

        /**
         * The number of points in this mesh
         */
        size_t PointCount() const { return pointList.size(); }

        /**
         *
         */
        const FTPoint& Point(unsigned int index) const
        { return pointList[index]; }

        /**
         * Return the OpenGL polygon type.
         */
        GLenum PolygonType() const { return meshType; }

    private:
        /**
         * Points generated by gluTesselator.
         */
        typedef FTVector<FTPoint> PointVector;
        PointVector pointList;

        /**
         * OpenGL primitive type from gluTesselator.
         */
        GLenum meshType;
};


/**
 * FTMesh is a container of FTTesselation's that make up a polygon glyph
 */
class FTGL_EXPORT FTMesh
{
        typedef FTVector<FTTesselation*> TesselationVector;
        typedef FTList<FTPoint> PointList;

    public:
        /**
         * Default constructor
         */
        FTMesh();

        /**
         *  Destructor
         */
        ~FTMesh();

        /**
         * Add a point to the mesh
         */
        void AddPoint(const FTGL_DOUBLE x, const FTGL_DOUBLE y,
                      const FTGL_DOUBLE z);

        /**
         *  Create a combine point for the gluTesselator
         */
        const FTGL_DOUBLE* Combine(const FTGL_DOUBLE x, const FTGL_DOUBLE y,
                                   const FTGL_DOUBLE z);

        /**
         * Begin a new polygon
         */
        void Begin(GLenum meshType);

        /**
         * End a polygon
         */
        void End();

        /**
         * Record a gluTesselation error
         */
        void Error(GLenum e) { err = e; }

        /**
         * The number of tesselations in the mesh
         */
        unsigned int TesselationCount() const { return tesselationList.size(); }

        /**
         * Get a tesselation by index
         */
        const FTTesselation* const Tesselation(unsigned int index) const;

        /**
         * Return the temporary point list. For testing only.
         */
        const PointList& TempPointList() const { return tempPointList; }

        /**
         * Get the GL ERROR returned by the glu tesselator
         */
        GLenum Error() const { return err; }

    private:
        /**
         * The current sub mesh that we are constructing.
         */
        FTTesselation* currentTesselation;

        /**
         * Holds each sub mesh that comprises this glyph.
         */
        TesselationVector tesselationList;

        /**
         * Holds extra points created by gluTesselator. See ftglCombine.
         */
        PointList tempPointList;

        /**
         * GL ERROR returned by the glu tesselator
         */
        GLenum err;

};

const FTGL_DOUBLE FTGL_FRONT_FACING = 1.0;
const FTGL_DOUBLE FTGL_BACK_FACING = -1.0;

/**
 * FTVectoriser class is a helper class that converts font outlines into
 * point data.
 *
 * @see FTExtrdGlyph
 * @see FTOutlineGlyph
 * @see FTPolyGlyph
 * @see FTContour
 * @see FTPoint
 *
 */
class FTGL_EXPORT FTVectoriser
{
    public:
        /**
         * Constructor
         *
         * @param glyph The freetype glyph to be processed
         */
        FTVectoriser(const FT_GlyphSlot glyph);

        /**
         *  Destructor
         */
        virtual ~FTVectoriser();

        /**
         * Build an FTMesh from the vector outline data.
         *
         * @param zNormal   The direction of the z axis of the normal
         *                  for this mesh
         * FIXME: change the following for a constant
         * @param outsetType Specify the outset type contour
         *  0 : Original
         *  1 : Front
         *  2 : Back
         * @param outsetSize Specify the outset size contour
         */
        void MakeMesh(FTGL_DOUBLE zNormal = FTGL_FRONT_FACING, int outsetType = 0, float outsetSize = 0.0f);

        /**
         * Get the current mesh.
         */
        const FTMesh* const GetMesh() const { return mesh; }

        /**
         * Get the total count of points in this outline
         *
         * @return the number of points
         */
        size_t PointCount();

        /**
         * Get the count of contours in this outline
         *
         * @return the number of contours
         */
        size_t ContourCount() const { return ftContourCount; }

        /**
         * Return a contour at index
         *
         * @return the number of contours
         */
         const FTContour* const Contour(unsigned int index) const;

        /**
         * Get the number of points in a specific contour in this outline
         *
         * @param c     The contour index
         * @return      the number of points in contour[c]
         */
        size_t ContourSize(int c) const { return contourList[c]->PointCount(); }

        /**
         * Get the flag for the tesselation rule for this outline
         *
         * @return The contour flag
         */
        int ContourFlag() const { return contourFlag; }

    private:
        /**
         * Process the freetype outline data into contours of points
         *
         * @param front front outset distance
         * @param back back outset distance
         */
        void ProcessContours();

        /**
         * The list of contours in the glyph
         */
        FTContour** contourList;

        /**
         * A Mesh for tesselations
         */
        FTMesh* mesh;

        /**
         * The number of contours reported by Freetype
         */
        short ftContourCount;

        /**
         * A flag indicating the tesselation rule for the glyph
         */
        int contourFlag;

        /**
         * A Freetype outline
         */
        FT_Outline outline;
};


#endif  //  __FTVectoriser__
