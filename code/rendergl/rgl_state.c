#include "rgl_local.h"

typedef struct
{
    GLuint rboId;
    GLuint fboId;
    GLuint texUnits[MAX_TEXTURE_UNITS];
    GLuint *texPtr;
} dsa_t;

void GL_BindRenderbuffer(GLuint buffer)
{
    
}