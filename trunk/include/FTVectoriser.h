#ifndef     __FTVectoriser__
#define     __FTVectoriser__

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "FTContour.h"
#include "FTPoint.h"
#include "FTVector.h"
#include "FTGL.h"


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
        FTTesselation()
        {
            pointList.reserve( 128);
        }

        /**
         *  Destructor
         */
        ~FTTesselation()
        {
            pointList.clear();
        }

        
        void AddPoint( const FTGL_DOUBLE x, const FTGL_DOUBLE y, const FTGL_DOUBLE z)
        {   
            pointList.push_back( FTPoint( x, y, z));
        }


        size_t size() const { return pointList.size();}

        /**
         * OpenGL primitive type from gluTesselator.
         */
        GLenum meshType;
        
        /**
         * Points generated by gluTesselator.
         */
        typedef FTVector<FTPoint> PointVector;
        PointVector pointList;
        
    private:
        
};


/**
 * FTMesh is a container of FTTesselation's that make up a polygon glyph
 */
class FTGL_EXPORT FTMesh
{
    public:
        /**
         * Default constructor
         */
        FTMesh();

        /**
         *  Destructor
         */
        ~FTMesh();
        
        void AddPoint( const FTGL_DOUBLE x, const FTGL_DOUBLE y, const FTGL_DOUBLE z);
        void Begin( GLenum m);
        void End();
        void Error( GLenum e) { err = e;}
        
        int size() const;
        
        /**
         * Get the GL ERROR returned by the glu tesselator
         */
        GLenum Error() const { return err;}

        /**
         * Holds extra points created by gluTesselator. See ftglCombine.
         */
        typedef FTVector<FTPoint> PointVector;
        PointVector tempPointList;
        
        /**
         * Holds each sub mesh that comprises this glyph.
         */
        typedef FTVector<FTTesselation*> TesselationVector;
        TesselationVector tesselationList;
        
    protected:
    
    private:
        /**
         * The current sub mesh that we are constructing.
         */
        FTTesselation* currentTesselation;
        
        /**
         * GL ERROR returned by the glu tesselator
         */
        GLenum err;

};



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
        FTVectoriser( const FT_Glyph glyph);

        /**
         *  Destructor
         */
        virtual ~FTVectoriser();

        /**
         * Process the freetype outline data into contours of points
         */
        void ProcessContours();

        /**
         * Copy the outline data into a block of <code>FTGL_DOUBLEs</code>
         *
         * @param d a pointer to the memory to copy the data into.
         */
        void GetOutline( FTGL_DOUBLE* d);

        /**
         * Build an FTMesh from the vector outline data. 
         *
         * @param zNormal   The direction of the z axis of the normal
         *                  for this mesh
         */
        void MakeMesh( FTGL_DOUBLE zNormal = 1.0);
        
        /**
         * Copy the tesselation data into a block of <code>FTGL_DOUBLEs</code>
         *
         * @param d a pointer to the memory to copy the data into.
         */
        void GetMesh( FTGL_DOUBLE* d);
        
        /** Get the number of points in the tesselation
         *
         * @return the number of points.
         */
        size_t MeshPoints() const { return mesh->size();}
        
        /**
         * Get the total count of points in this outline
         *
         * @return the number of points
         */
        int points();

        /**
         * Get the count of contours in this outline
         *
         * @return the number of contours
         */
        size_t contours() const { return ftContourCount;}

        /**
         * Get the number of points in a specific contour in this outline
         *
         * @param c     The contour index
         * @return      the number of points in contour[c]
         */
        size_t contourSize( int c) const { return contourList[c]->size();}

        /**
         * Get the flag for the tesselation rule for this outline
         *
         * @return The contour flag
         */
        int ContourFlag() const { return contourFlag;}
        
    private:
        /**
         * The list of contours in the glyph
         */
        FTContour** contourList;

        /**
         * A Mesh for tesselations
         */
        FTMesh* mesh;
        
        /**
         *
         */
        short ftContourCount;

        /**
         * A flag indicating the tesselation rule for the glyph
         */
        int contourFlag;

        /**
         * A Freetype outline
         */
        FT_Outline ftOutline;
        
        /**
         */
         // Magic numbers -- #define MAX_DEG 4
        float bValues[4][4][2];  //3D array storing values of de Casteljau algorithm.
        float ctrlPtArray[4][2]; // Magic numbers
        
        /**
         */
        const float kBSTEPSIZE;

};


#endif  //  __FTVectoriser__
