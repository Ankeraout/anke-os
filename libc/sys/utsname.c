#include <sys/utsname.h>
#include <string.h>

int uname(struct utsname *p_structure) {
    if(p_structure == NULL) {
        return -1;
    }

    strcpy(p_structure->sysname, "AnkeOS");
    strcpy(p_structure->nodename, "");
    strcpy(p_structure->release, "0.1.0");
    strcpy(p_structure->version, "");
    strcpy(p_structure->machine, "x86_64");

    return 0;
}
