#include "FTContour.h"

static const float BEZIER_STEP_SIZE = 0.2f;


FTContour::FTContour( FT_Vector* contour, char* pointTags, unsigned int numberOfPoints)
{
    for( unsigned int pointIndex = 0; pointIndex < numberOfPoints; ++ pointIndex)
    {
        char pointTag = pointTags[pointIndex];
        
        if( pointTag == FT_Curve_Tag_On || numberOfPoints < 2)
        {
            AddPoint( contour[pointIndex].x, contour[pointIndex].y);
            continue;
        }
        
        FTPoint controlPoint( contour[pointIndex]);
        FTPoint previousPoint = ( 0 == pointIndex)
                                ? FTPoint( contour[numberOfPoints - 1])
                                : pointList[pointList.size() - 1];

        FTPoint nextPoint = ( pointIndex == numberOfPoints - 1)
                            ? pointList[0]
                            : FTPoint( contour[pointIndex + 1]);

        if( pointTag == FT_Curve_Tag_Conic)
        {
            char nextPointTag = ( pointIndex == numberOfPoints - 1)
                                ? pointTags[0]
                                : pointTags[pointIndex + 1];
            
            while( nextPointTag == FT_Curve_Tag_Conic)
            {
                nextPoint = FTPoint( static_cast<float>( controlPoint.x + nextPoint.x) * 0.5f,
                                     static_cast<float>( controlPoint.y + nextPoint.y) * 0.5f,
                                     0);

                controlPoints[0][0] = previousPoint.x; controlPoints[0][1] = previousPoint.y;
                controlPoints[1][0] = controlPoint.x;  controlPoints[1][1] = controlPoint.y;
                controlPoints[2][0] = nextPoint.x;     controlPoints[2][1] = nextPoint.y;
                
                evaluateQuadraticCurve();
                ++pointIndex;
                
                previousPoint = nextPoint;
                controlPoint = FTPoint( contour[pointIndex]);
                nextPoint = ( pointIndex == numberOfPoints - 1)
                            ? pointList[0]
                            : FTPoint( contour[pointIndex + 1]);
                nextPointTag = ( pointIndex == numberOfPoints - 1)
                               ? pointTags[0]
                               : pointTags[pointIndex + 1];
            }
            
            controlPoints[0][0] = previousPoint.x; controlPoints[0][1] = previousPoint.y;
            controlPoints[1][0] = controlPoint.x;  controlPoints[1][1] = controlPoint.y;
            controlPoints[2][0] = nextPoint.x;     controlPoints[2][1] = nextPoint.y;
            
            evaluateQuadraticCurve();
            continue;
        }

        if( pointTag == FT_Curve_Tag_Cubic)
        {
            FTPoint controlPoint2 = nextPoint;
            
            FTPoint nextPoint = ( pointIndex == numberOfPoints - 2)
                                ? pointList[0]
                                : FTPoint( contour[pointIndex + 2]);
            
            controlPoints[0][0] = previousPoint.x; controlPoints[0][1] = previousPoint.y;
            controlPoints[1][0] = controlPoint.x;  controlPoints[1][1] = controlPoint.y;
            controlPoints[2][0] = controlPoint2.x; controlPoints[2][1] = controlPoint2.y;
            controlPoints[3][0] = nextPoint.x;     controlPoints[3][1] = nextPoint.y;
        
            evaluateCubicCurve();
            ++pointIndex;
            continue;
        }
    }
}


void FTContour::AddPoint( FTPoint point)
{
    if( pointList.empty() || point != pointList[pointList.size() - 1])
    {
        pointList.push_back( point);
    }
}


void FTContour::AddPoint( float x, float y)
{
    FTPoint point( x, y, 0.0f);
    
    if( pointList.empty() || point != pointList[pointList.size() - 1])
    {
        pointList.push_back( point);
    }
}


void FTContour::evaluateQuadraticCurve()
{
    for( unsigned int i = 0; i <= ( 1.0f / BEZIER_STEP_SIZE); i++)
    {
        float bezierValues[2][2];

        float t = static_cast<float>(i) * BEZIER_STEP_SIZE;

        bezierValues[0][0] = (1.0f - t) * controlPoints[0][0] + t * controlPoints[1][0];
        bezierValues[0][1] = (1.0f - t) * controlPoints[0][1] + t * controlPoints[1][1];
    
        bezierValues[1][0] = (1.0f - t) * controlPoints[1][0] + t * controlPoints[2][0];
        bezierValues[1][1] = (1.0f - t) * controlPoints[1][1] + t * controlPoints[2][1];
        
        bezierValues[0][0] = (1.0f - t) * bezierValues[0][0] + t * bezierValues[1][0];
        bezierValues[0][1] = (1.0f - t) * bezierValues[0][1] + t * bezierValues[1][1];
    
        AddPoint( bezierValues[0][0], bezierValues[0][1]);
    }
}

void FTContour::evaluateCubicCurve()
{
    for( unsigned int i = 0; i <= ( 1.0f / BEZIER_STEP_SIZE); i++)
    {
        float bezierValues[3][2];

        float t = static_cast<float>(i) * BEZIER_STEP_SIZE;

        bezierValues[0][0] = (1.0f - t) * controlPoints[0][0] + t * controlPoints[1][0];
        bezierValues[0][1] = (1.0f - t) * controlPoints[0][1] + t * controlPoints[1][1];
    
        bezierValues[1][0] = (1.0f - t) * controlPoints[1][0] + t * controlPoints[2][0];
        bezierValues[1][1] = (1.0f - t) * controlPoints[1][1] + t * controlPoints[2][1];
        
        bezierValues[2][0] = (1.0f - t) * controlPoints[2][0] + t * controlPoints[3][0];
        bezierValues[2][1] = (1.0f - t) * controlPoints[2][1] + t * controlPoints[3][1];
        
        bezierValues[0][0] = (1.0f - t) * bezierValues[0][0] + t * bezierValues[1][0];
        bezierValues[0][1] = (1.0f - t) * bezierValues[0][1] + t * bezierValues[1][1];
    
        bezierValues[1][0] = (1.0f - t) * bezierValues[1][0] + t * bezierValues[2][0];
        bezierValues[1][1] = (1.0f - t) * bezierValues[1][1] + t * bezierValues[2][1];
        
        bezierValues[0][0] = (1.0f - t) * bezierValues[0][0] + t * bezierValues[1][0];
        bezierValues[0][1] = (1.0f - t) * bezierValues[0][1] + t * bezierValues[1][1];
    
        AddPoint( bezierValues[0][0], bezierValues[0][1]);
    }
}

