#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/object.h>

class CModuleLib
{
public:
    CModuleLib( void );
    ~CModuleLib();
};

MonoObject* CreateVersionObject();

void InitModuleLib()
{

}