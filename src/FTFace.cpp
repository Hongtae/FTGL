#include    "FTFace.h"
#include    "FTLibrary.h"
#include    "FTCharmap.h"


FTFace::FTFace()
:   charMap(0),
    ftFace(0),
    numCharMaps(0),
    numGlyphs(0),
    err(0)
{}


FTFace::~FTFace()
{
    delete charMap;
    Close();
}


bool FTFace::Open( const char* filename)
{
    const FT_Long DEFAULT_FACE_INDEX = 0;
    ftFace = new FT_Face;
    
    // FIXME check library for errors
    err = FT_New_Face( *FTLibrary::Instance().GetLibrary(), filename, DEFAULT_FACE_INDEX, ftFace);

    if( err)
    {
        delete ftFace;
        ftFace = 0;
        return false;
    }
    else
    {
        charMap = new FTCharmap( *ftFace);
        return true;
    }
}


bool FTFace::Open( const unsigned char *pBufferBytes, size_t bufferSizeInBytes )
{
    const FT_Long DEFAULT_FACE_INDEX = 0;
    ftFace = new FT_Face;

    // FIXME check library for errors
    err = FT_New_Memory_Face( *FTLibrary::Instance().GetLibrary(), (FT_Byte *)pBufferBytes, bufferSizeInBytes, DEFAULT_FACE_INDEX, ftFace);

    if( err)
    {
        delete ftFace;
        ftFace = 0;
        return false;
    }
    else
    {
        charMap = new FTCharmap( *ftFace);
        return true;
    }
}


bool FTFace::Attach( const char* filename)
{
    err = FT_Attach_File( *ftFace, filename);
    return !err;
}


void FTFace::Close()
{
    if( ftFace)
    {
        FT_Done_Face( *ftFace);
        delete ftFace;
        ftFace = 0;
    }
}


FTSize& FTFace::Size( const unsigned int size, const unsigned int res)
{
    charSize.CharSize( ftFace, size, res, res);
    err = charSize.Error();

    return charSize;
}


bool FTFace::CharMap( FT_Encoding encoding)
{
    bool result = charMap->CharMap( encoding);
    err = charMap->Error();
    return result;
}


unsigned int FTFace::CharIndex( unsigned int index) const
{
    return charMap->CharIndex( index);
}


FTPoint FTFace::KernAdvance( unsigned int index1, unsigned int index2)
{
    FT_Vector kernAdvance;
    kernAdvance.x = 0;
    kernAdvance.y = 0;
    
    if( FT_HAS_KERNING((*ftFace)) && index1 && index2)
    {
        err = FT_Get_Kerning( *ftFace, index1, index2, ft_kerning_unfitted, &kernAdvance);
        if( !err)
        {   
            kernAdvance.x /= 64;
            kernAdvance.y /= 64;
        }
    }
    
    return FTPoint( kernAdvance);
}


FT_Glyph* FTFace::Glyph( unsigned int index, FT_Int load_flags)
{
    err = FT_Load_Glyph( *ftFace, index, load_flags);   
    err = FT_Get_Glyph( (*ftFace)->glyph, &ftGlyph);
        
    if( !err)
    {
        return &ftGlyph;
    }
    else
    {
        return NULL;
    }
}



